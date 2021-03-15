import shutil, os, sys, stat, subprocess, re, argparse

def simgridrun(cmd, to, filename, binary, id):

    try:
        subprocess.run("smpicc {} -o {}  > {}_{}.txt 2>&1".format(filename,binary,binary,id), shell=True, check=True)
    except subprocess.CalledProcessError:
        return 'CUN'
        
    cmd = re.sub("mpirun", "smpirun -wrapper simgrid-mc -platform ./cluster.xml --cfg=smpi/list-leaks:10", cmd)
    cmd = re.sub('\${EXE}', binary, cmd)
    cmd = re.sub('\$zero_buffer', "--cfg=smpi/buffering:zero", cmd)
    cmd = re.sub('\$infty_buffer', "--cfg=smpi/buffering:infty", cmd)
    cmd = re.sub('$', ' > {}_{}.txt 2>&1'.format(binary,id), cmd)
    try:
        ret = subprocess.call(cmd, shell=True, timeout=to)
    except subprocess.TimeoutExpired:    
        return 'timeout'
        
    if int(subprocess.check_output("grep 'No property violation found' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'noerror'

    elif int(subprocess.check_output("grep 'DEADLOCK DETECTED' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'deadlock'

    elif int(subprocess.check_output("grep 'returned MPI_ERR' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'mpierr'

    elif int(subprocess.check_output("grep 'Not yet implemented' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'CUN'

    elif int(subprocess.check_output("grep 'leak detected' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'resleak' 
    
    else:
        print("Couldn't assign output to specific behaviour (ret=={}) : this will be treated as 'other'".format(ret))
        return 'other'


