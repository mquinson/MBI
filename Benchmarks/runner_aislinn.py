import shutil, os, sys, stat, subprocess, re, argparse, time

def aislinnrun(cmd, to, filename, binary, id):
    
    try:
        subprocess.run("aislinn-cc -g {} -o {} > {}_{}.txt 2>&1".format(filename,binary,binary,id), shell=True, check=True)
    except subprocess.CalledProcessError:
        return 'CUN'
    
    cmd = re.sub("mpirun", "aislinn", cmd)
    cmd = re.sub('\${EXE}', binary, cmd)
    cmd = re.sub('\$zero_buffer', "--send-protocol=rendezvous", cmd)
    cmd = re.sub('\$infty_buffer', "--send-protocol=eager", cmd)
    cmd = re.sub('-np ', '-p=', cmd)
    print("\nRUNNING : {}\n".format(cmd))
    ret = None
    output = "Executing https://gitlab.com/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/-/tree/master/Benchmarks/microbenchs/{}.c\n\n".format(binary)
    
    cmd = re.sub('$', ' > {}_{}.txt 2>&1'.format(binary, id), cmd)
    try:
        ret = subprocess.call(cmd, shell=True, timeout=to)
    except subprocess.TimeoutExpired:    
        return 'timeout'
        
    if int(subprocess.check_output("grep 'No errors found' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'noerror'
    
    elif int(subprocess.check_output("grep 'Deadlock' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'deadlock'
    elif int(subprocess.check_output("grep 'Collective operation mismatch' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'deadlock'
    
    elif int(subprocess.check_output("grep 'Invalid rank' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'mpierr'
    elif int(subprocess.check_output("grep 'Invalid datatype' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'mpierr'
    

    elif int(subprocess.check_output("grep 'Collective operation: root mismatch' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'compliance'

    elif int(subprocess.check_output("grep 'Unkown function call' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'RSF'
    
    else:
        return 'other'


