import shutil, os, sys, stat, subprocess, re, argparse, shlex

def mustrun(cmd, filename, binary, id, distributed=True):
    try:
        subprocess.run("mpicc {} -o {} > {}_{}.txt 2>&1".format(filename,binary,binary,id), shell=True, check=True)
    except subprocess.CalledProcessError:
        return 'CUN'
    if distributed:
        cmd = re.sub("mpirun", "mustrun --must:distributed", cmd)
    else:
        cmd = re.sub("mpirun", "mustrun", cmd)
    cmd = re.sub('\${EXE}', binary, cmd)
    cmd = re.sub('\$zero_buffer', "", cmd)
    cmd = re.sub('\$infty_buffer', "", cmd)	
    print("\nRUNNING : {}\n".format(cmd))
    ret = None
    output = "Executing https://gitlab.com/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/-/tree/master/Benchmarks/microbenchs/{}.c\n\n".format(binary)

    try:
        # We run the subprocess and parse its output line by line, so that we can kill it as soon as it detects a timeout
        process = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        while True:
            line = process.stdout.readline()
            if line == '' and process.poll() is not None:
                break
            if line:
                line = line.decode('UTF-8') # From byte array to string
                output = output + line
                print (line, end='')
                if re.search("ERROR: MUST detected a deadlock", line):
                    process.terminate()
        rc = process.poll()
    except subprocess.TimeoutExpired:   
            return 'timeout'

        
    if not os.path.isfile("./MUST_Output.html"):
        return 'RSF'
    os.rename("./MUST_Output.html", "{}_{}.txt".format(binary,id))        
            
    if int(subprocess.check_output("grep deadlock {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'deadlock'
    
    elif int(subprocess.check_output("grep 'not freed' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'resleak'

    elif int(subprocess.check_output("grep 'conflicting roots' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'compliance'

    elif int(subprocess.check_output("grep 'unknown datatype' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'mpierr'
    elif int(subprocess.check_output("grep 'has to be a non-negative integer' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'mpierr'
    elif int(subprocess.check_output("grep 'must use equal type signatures' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'mpierr'
    
    elif re.search('caught MPI error', output.decode('UTF-8')) != None:
        return 'mpierr'     
    
    elif int(subprocess.check_output("grep 'Error' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'other'

    elif re.search('MUST-ERROR', output.decode('UTF-8')) != None:
        return 'RSF'
    
    else:
        return 'noerror'
