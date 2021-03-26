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

    if res != None:
        return res
    
    if re.search('deadlock', html):
        return 'deadlock'
    
    if re.search('not freed', html):
        return 'resleak'

    if re.search('conflicting roots', html):
        return 'compliance'

    if re.search('unknown datatype', html) or re.search('has to be a non-negative integer', html) or re.search('must use equal type signatures', html):
        return 'mpierr'
    
    if re.search('caught MPI error', output):
        return 'mpierr'     
    
    if re.search('Error', html):
        return 'other'

    if re.search('MUST-ERROR', output):
        return 'RSF'
    
    return 'noerror'


def aislinnrun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "aislinn", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "--send-protocol=rendezvous", execcmd)
    execcmd = re.sub('\$infty_buffer', "--send-protocol=eager", execcmd)
    execcmd = re.sub('-np ', '-p=', execcmd)

    res, rc, output = run_cmd(
        buildcmd="aislinn-cc -g {} -o {} > {}_{}.txt 2>&1".format(filename,binary,binary,id),
        execcmd=execcmd, 
        binary=binary)

    if res != None:
        return res
    
    if re.search('No errors found', output):
        return 'noerror'
    
    if re.search('Deadlock', output):
        return 'deadlock'
    if re.search('Collective operation mismatch', output):
        return 'deadlock'
    
    if re.search('Invalid rank', output):
        return 'mpierr'
    if re.search('Invalid datatype', output):
        return 'mpierr'
    
    if re.search('Collective operation: root mismatch', output):
        return 'compliance'

    if re.search('Unkown function call', output):
        return 'RSF'
    
    return 'other'

    
def civlrun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "java -jar /builds/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/CIVL/CIVL-1.20_5259/lib/civl-1.20_5259.jar verify", execcmd)
    execcmd = re.sub('-np ', "-input_mpi_nprocs=", execcmd)
    execcmd = re.sub('\${EXE}', filename, execcmd)
    execcmd = re.sub('\$zero_buffer', "", execcmd)
    execcmd = re.sub('\$infty_buffer', "", execcmd)
    execcmd = re.sub('$', " > {}_{}.txt 2>&1".format(binary,id), execcmd)

    res, rc, output = run_cmd(
        buildcmd="echo 'Nothing to compile'",
        execcmd=execcmd, 
        binary=binary)

    if res != None:
        return res
    
    if re.search('DEADLOCK', output):
        return 'deadlock'
   
    if re.search('has a different root', output):
        return 'compliance'
    if re.search('has a different MPI_Op', output):
        return 'compliance'

    if re.search('MPI message leak', output):
        return 'mpierr'
    if re.search('MPI_ERROR', output):
        return 'mpierr'

    if re.search('MEMORY_LEAK', output):
        return 'resleak'
   
    if re.search('The standard properties hold for all executions', output):
        return 'noerror'

    if re.search('A CIVL internal error has occurred', output):
        return 'RSF'
    
    if re.search('This feature is not yet implemented', output):
        return 'CUN'
    if re.search('doesn.t have a definition', output):
        return 'CUN'
    if re.search('Undeclared identifier', output):
        return 'CUN'
    
    return 'other'


def isprun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "isp.exe", execcmd)
    execcmd = re.sub('-np', '-n', execcmd)
    execcmd = re.sub('\${EXE}', "./{}".format(binary), execcmd)
    execcmd = re.sub('\$zero_buffer', "-b", execcmd)
    execcmd = re.sub('\$infty_buffer', "-g", execcmd)
    execcmd = re.sub('$', " > {}_{}.txt 2>&1".format(binary,id), execcmd)

    print("\nClearing port before RUNNING : {}\n".format(cmd))
    subprocess.run("kill -9 $(lsof -t -i:9999)", shell=True)

    res, rc, output = run_cmd(
        buildcmd="ispcc -o {} {} > {}_{}.txt".format(binary,filename,binary,id),
        execcmd=execcmd, 
        binary=binary)

    if res != None:
        return res
    
    if re.search('ISP detected deadlock!!!', output):
        return 'deadlock'

    if re.search('resource leaks detected', output):
        return 'resleak'

    if re.search('ISP detected no deadlocks', output):
        return 'noerror'

    if re.search('Fatal error in PMPI', output):
        return 'mpierr'
    if re.search('Fatal error in MPI', output):
        return 'mpierr'
    
    return 'other'


def parcoachrun(execcmd, filename, binary, id):

    execcmd = "opt-9 -load /builds/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/Parcoach/parcoach/build/src/aSSA/aSSA.so -parcoach -check-mpi < {}.bc > /dev/null 2> {}_{}.txt".format(binary,binary,id)

    res, rc, output = run_cmd(
        buildcmd="clang -c -g -emit-llvm {} -I/usr/lib/x86_64-linux-gnu/mpich/include/ -o {}.bc".format(filename,binary),
        execcmd=execcmd, 
        binary=binary)

    if res != None:
        return res
    
    if re.search('0 warning(s) issued', output):
        return 'noerror'

    if re.search('missing info for external function', output):
        return 'CUN'
    
    return 'deadlock'
