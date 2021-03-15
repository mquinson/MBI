import shutil, os, sys, stat, subprocess, re, argparse

def civlrun(cmd, to, filename, binary, id):
    cmd = re.sub("mpirun", "java -jar /builds/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/CIVL/CIVL-1.20_5259/lib/civl-1.20_5259.jar verify", cmd)
    cmd = re.sub('-np ', "-input_mpi_nprocs=", cmd)
    cmd = re.sub('\${EXE}', filename, cmd)
    cmd = re.sub('\$zero_buffer', "", cmd)
    cmd = re.sub('\$infty_buffer', "", cmd)
    cmd = re.sub('$', " > {}_{}.txt 2>&1".format(binary,id), cmd)
    try:
        ret = subprocess.call(cmd, shell=True, timeout=to)
    except subprocess.TimeoutExpired:    
        return 'timeout'
        
    if int(subprocess.check_output("grep 'DEADLOCK' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'deadlock'

   

    elif int(subprocess.check_output("grep 'has a different root' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'compliance'

    
    elif int(subprocess.check_output("grep 'has a different MPI_Op' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'compliance'


    
    elif int(subprocess.check_output("grep 'MPI message leak' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'mpierr'
    elif int(subprocess.check_output("grep 'MPI_ERROR' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'mpierr'

    elif int(subprocess.check_output("grep 'MEMORY_LEAK' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'resleak'
   

    elif int(subprocess.check_output("grep 'The standard properties hold for all executions' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'noerror'


    elif int(subprocess.check_output("grep 'A CIVL internal error has occurred' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'RSF'

    
    elif int(subprocess.check_output("grep 'This feature is not yet implemented' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'CUN'
    elif int(subprocess.check_output("grep 'doesn.t have a definition' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'CUN'
    elif int(subprocess.check_output("grep 'Undeclared identifier' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'CUN'
    
    else:
        return 'other'
