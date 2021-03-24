import shutil, os, sys, stat, subprocess, re, argparse, shlex

def run_cmd(buildcmd, execcmd, binary, read_line_lambda=None):
    output = "Compiling https://gitlab.com/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/-/tree/master/Benchmarks/microbenchs/{}.c\n\n".format(binary)
    output += "$ {}".format(buildcmd)

    compil = subprocess.run(buildcmd, shell=True, stderr=subprocess.STDOUT)
    if compil.stdout is not None:
        output += compil.stdout
    if compil.returncode != 0:
        output += "Compilation of {}.c raised an error (retcode: %d)".format(binary, compil.returncode)
        for line in (output.split('\n')):
            print ("| {}".format(line), file=sys.stderr)
        return 'CUN', compil.returncode, output

    output += "\n\nExecuting https://gitlab.com/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/-/tree/master/Benchmarks/microbenchs/{}.c\n\n$ {}\n".format(binary,execcmd)
    for line in (output.split('\n')):
        print ("| {}".format(line), file=sys.stderr)

    try:
        # We run the subprocess and parse its output line by line, so that we can kill it as soon as it detects a timeout
        process = subprocess.Popen(shlex.split(execcmd), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        while True:
            line = process.stdout.readline()
            if line:
                line = line.decode('UTF-8') # From byte array to string
                output = output + line
                print ("| {}".format(line), end='', file=sys.stderr)
                if read_line_lambda != None:
                    read_line_lambda(line, process)
            if process.poll() is not None:
                break
        rc = process.poll()
    except subprocess.TimeoutExpired:
        return 'timeout', output

    return None, rc, output


def simgridrun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "smpirun -wrapper simgrid-mc -platform ./cluster.xml --cfg=smpi/list-leaks:10", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "--cfg=smpi/buffering:zero", execcmd)
    execcmd = re.sub('\$infty_buffer', "--cfg=smpi/buffering:infty", execcmd)
    
    res, rc, output = run_cmd(
        buildcmd="smpicc {} -o {}  > {}_{}.txt 2>&1".format(filename,binary,binary,id),
        execcmd=execcmd, 
        binary=binary)

    with open('{}_{}.txt'.format(binary, id), 'w') as outfile:
        outfile.write(output)    
        
    if res != None:
        return res
    if re.search('No property violation found', output):
        return 'noerror'
    if re.search('DEADLOCK DETECTED', output):
        return 'deadlock'
    if re.search('returned MPI_ERR', output):
        return 'mpierr'
    if re.search('Not yet implemented', output):
        return 'CUN'
    if re.search('leak detected', output):
        return 'resleak'

    print("Couldn't assign output to specific behaviour (ret: {}) : this will be treated as 'other'".format(rc))
    return 'other'

def must_filter(line, process):
    if re.search("ERROR: MUST detected a deadlock", line):
        process.terminate()
def mustrun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "mustrun --must:distributed", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "", execcmd)
    execcmd = re.sub('\$infty_buffer', "", execcmd)	

    res, rc, output = run_cmd(
        buildcmd="mpicc {} -o {} > {}_{}.txt 2>&1".format(filename,binary,binary,id),
        execcmd=execcmd, 
        binary=binary,
        read_line_lambda=must_filter)

    with open('{}_{}.txt'.format(binary, id), 'w') as outfile:
        outfile.write(output)    
    
    if not os.path.isfile("./MUST_Output.html"):
        return 'RSF'

    html = ""
    with open('MUST_Output.html') as input:
        for line in (input.readlines()):
            html += line
    os.rename("./MUST_Output.html", "{}_{}.html".format(binary,id))

    if re.search('deadlock', html):
        return 'deadlock'
    
    if re.search('not freed', html):
        return 'resleak'

    if re.search('conflicting roots', html):
        return 'compliance'

    if re.search('unknown datatype', html) or re.search('has to be a non-negative integer', html) or re.search('must use equal type signatures', html):
        return 'mpierr'
    
    if re.search('caught MPI error', html):
        return 'mpierr'     
    
    if re.search('Error', html):
        return 'other'

    if re.search('MUST-ERROR', html):
        return 'RSF'
    
    return 'noerror'

