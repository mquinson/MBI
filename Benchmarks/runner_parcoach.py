import shutil, os, sys, stat, subprocess, re, argparse

def parcoachrun(cmd, to, filename, binary, id):
    subprocess.call("clang -c -g -emit-llvm {} -I/usr/lib/x86_64-linux-gnu/mpich/include/ -o {}.bc".format(filename,binary), shell=True)
    cmd = "opt-9 -load /builds/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/Parcoach/parcoach/build/src/aSSA/aSSA.so -parcoach -check-mpi < {}.bc > /dev/null 2> {}_{}.txt".format(binary,binary,id)
    print("\nRUNNING : {}\n".format(cmd))
    ret = None	
    try:
        ret = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, timeout=to)
    except subprocess.TimeoutExpired:
        return 'timeout'

    if int(subprocess.check_output("grep '0 warning(s) issued' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'noerror'

    elif int(subprocess.check_output("grep 'missing info for external function' {}_{}.txt | wc -l".format(binary,id), shell=True)) > 0:
        return 'CUN'
    
    else:
        return 'deadlock'
