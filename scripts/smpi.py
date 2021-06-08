import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "SimGrid MPI with Valgrind wrapper"


    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x smpi")

    def setup(self, rootdir):
        os.environ['PATH'] = os.environ['PATH'] + ":" + rootdir + "/builds/SimGrid/bin"
        os.environ['VERBOSE'] = '1'

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        if not os.path.exists("cluster.xml"):
            with open('cluster.xml', 'w') as outfile:
                outfile.write("<?xml version='1.0'?>\n")
                outfile.write("<!DOCTYPE platform SYSTEM \"https://simgrid.org/simgrid.dtd\">\n")
                outfile.write('<platform version="4.1">\n')
                outfile.write(' <cluster id="acme" prefix="node-" radical="0-99" suffix="" speed="1Gf" bw="125MBps" lat="50us"/>\n')
                outfile.write('</platform>\n')

        execcmd = re.sub("mpirun", "smpirun -wrapper 'valgrind --suppressions=../../tools/simgrid/tools/simgrid.supp' -platform ./cluster.xml", execcmd)
        execcmd = re.sub('\${EXE}', binary, execcmd)
        execcmd = re.sub('\$zero_buffer', "", execcmd)
        execcmd = re.sub('\$infty_buffer', "", execcmd)

        run_cmd(
            buildcmd=f"smpicc {filename} -g -Wl,-znorelro -Wl,-znoseparate-code -o {binary}",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

        subprocess.run("rm -f vgcore.*", shell=True, check=True) # Save disk space ASAP

    def teardown(self): 
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True) # Remove generated cruft (binary files)
        subprocess.run("rm -f smpitmp-* core", shell=True, check=True) 

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/simgrid/{cachefile}.timeout'):
            outcome = 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/simgrid/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/simgrid/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('ERROR SUMMARY: [^0]', output):
            return 'failure'

        if re.search('MC is currently not supported here', output):
            return 'failure'

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

        return 'other'
