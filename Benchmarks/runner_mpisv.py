import shutil, os, sys, stat, subprocess, re, argparse

def mpisvrun(cmd, to, filename, binary, id):
    
    try:     
        compile_cmd = "mpisvcc -o {} {} ".format(binary,filename)
        print("COMPILING : {}\n".format(compile_cmd))
        subprocess.run(compile_cmd, shell=True, check=True)
        subprocess.run("pwd", shell=True)
        subprocess.run("file {}".format(binary), shell=True)
    except subprocess.CalledProcessError:
        return 'CUN'
    
    cmd = re.sub("mpirun", "mpisv", cmd)
    cmd = re.sub('-np ', "", cmd)
    cmd = re.sub('\${EXE}', binary, cmd)
    cmd = re.sub('\$zero_buffer', "", cmd)
    cmd = re.sub('\$infty_buffer', "", cmd)
    cmd = re.sub('$', " > {}_{}.txt 2>&1".format(binary,id), cmd)
    print("\nRUNNING : {} \n \n".format(cmd))
    try:
        ret = subprocess.call(cmd, shell=True, timeout=to)
    except subprocess.TimeoutExpired:    
        return 'timeout'
        
    if int(subprocess.check_output("grep 'No Violation detected by MPI-SV' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'noerror'

    elif int(subprocess.check_output("grep 'found deadlock' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'deadlock'

    elif int(subprocess.check_output("grep 'failed external call' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'CUN'

    else:
        return 'other'
