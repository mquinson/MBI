import shutil, os, sys, stat, subprocess, re, argparse

def isprun(cmd, to, filename, binary, id):
    
    compile_cmd = "ispcc -o {} {} > {}_{}.txt".format(binary,filename,binary,id)
    try:
        subprocess.run(compile_cmd , shell=True, check=True)
    except subprocess.CalledProcessError:
        return 'CUN'

    cmd = re.sub("mpirun", "isp.exe", cmd)
    cmd = re.sub('-np', '-n', cmd)
    cmd = re.sub('\${EXE}', "./{}".format(binary), cmd)
    cmd = re.sub('\$zero_buffer', "-b", cmd)
    cmd = re.sub('\$infty_buffer', "-g", cmd)
    cmd = re.sub('$', " > {}_{}.txt 2>&1".format(binary,id), cmd)
    print("\nClearing port before RUNNING : {}\n".format(cmd))
    subprocess.run("kill -9 $(lsof -t -i:9999)", shell=True)
    ret = None	
    try:
        ret = subprocess.run(cmd, shell=True, timeout=to)
    except subprocess.TimeoutExpired:
        return 'timeout'


    
    if int(subprocess.check_output("grep 'ISP detected deadlock!!!' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'deadlock'

    elif int(subprocess.check_output("grep 'resource leaks detected' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'resleak'

    elif int(subprocess.check_output("grep 'ISP detected no deadlocks' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'noerror'

    elif int(subprocess.check_output("grep 'Fatal error in PMPI' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'mpierr'
    elif int(subprocess.check_output("grep 'Fatal error in MPI' {}_{}.txt | wc -l".format(binary, id), shell=True)) > 0:
        return 'mpierr'
    
    
    else:
        return 'other'
