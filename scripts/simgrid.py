import re
import os
from MBIutils import *

def run(execcmd, filename, binary, id, timeout):
    cachefile = f'{binary}_{id}'

    if not os.path.exists("cluster.xml"):
        with open('cluster.xml', 'w') as outfile:
            outfile.write("<?xml version='1.0'?>\n")
            outfile.write("<!DOCTYPE platform SYSTEM \"https://simgrid.org/simgrid.dtd\">\n")
            outfile.write('<platform version="4.1">\n')
            outfile.write(' <cluster id="acme" prefix="node-" radical="0-99" suffix="" speed="1Gf" bw="125MBps" lat="50us"/>\n')
            outfile.write('</platform>\n')

    execcmd = re.sub("mpirun", "smpirun -wrapper simgrid-mc -platform ./cluster.xml --cfg=smpi/list-leaks:10", execcmd)
    if re.search("rma", binary):  # DPOR reduction in simgrid cannot deal with RMA calls as they contain mutexes
        execcmd = re.sub("smpirun", "smpirun --cfg=model-check/reduction:none", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "--cfg=smpi/buffering:zero", execcmd)
    execcmd = re.sub('\$infty_buffer', "--cfg=smpi/buffering:infty", execcmd)

    run_cmd(
        buildcmd=f"smpicc {filename} -g -Wl,-znorelro -Wl,-znoseparate-code -o {binary}",
        execcmd=execcmd,
        cachefile=cachefile,
        binary=binary,
        timeout=timeout)

def parse(cachefile):
    if os.path.exists(f'{cachefile}.timeout'):
        return 'timeout'
    if not os.path.exists(f'{cachefile}.txt'):
        return 'failure'

    with open(f'{cachefile}.txt', 'r') as infile:
        output = infile.read()

    if re.search('DEADLOCK DETECTED', output):
        return 'deadlock'
    if re.search('returned MPI_ERR', output):
        return 'mpierr'
    if re.search('Not yet implemented', output):
        return 'UNIMPLEMENTED'
    if re.search('CRASH IN THE PROGRAM', output):
        return 'segfault'
    if re.search('Probable memory leaks in your code: SMPI detected', output):
        return 'resleak'
    if re.search('No property violation found', output):
        return 'OK'

    print("Couldn't assign output to specific behaviour; This will be treated as 'other'")
    return 'other'
