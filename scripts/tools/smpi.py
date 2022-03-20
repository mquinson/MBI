import re
import os
import tools.simgrid
from MBIutils import *

class Tool(tools.simgrid.Tool):
    def identify(self):
        return "SimGrid MPI"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x smpi")

    def run(self, execcmd, filename, binary, id, timeout, batchinfo, extraargs=""):
        cachefile = f'{binary}_{id}'

        if not os.path.exists("cluster.xml"):
            with open('cluster.xml', 'w') as outfile:
                outfile.write("<?xml version='1.0'?>\n")
                outfile.write("<!DOCTYPE platform SYSTEM \"https://simgrid.org/simgrid.dtd\">\n")
                outfile.write('<platform version="4.1">\n')
                outfile.write(' <cluster id="acme" prefix="node-" radical="0-99" suffix="" speed="1Gf" bw="125MBps" lat="50us"/>\n')
                outfile.write('</platform>\n')

        execcmd = re.sub("mpirun", f"smpirun {extraargs} --cfg=smpi/finalization-barrier:on -platform ./cluster.xml", execcmd)
        execcmd = re.sub('\${EXE}', binary, execcmd)
        execcmd = re.sub('\$zero_buffer', "", execcmd)
        execcmd = re.sub('\$infty_buffer', "", execcmd)

        run_cmd(
            buildcmd=f"smpicc {filename} -trace-call-location -g -Wl,-znorelro -Wl,-znoseparate-code -o {binary}",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True) # Remove generated cruft (binary files)
        subprocess.run("rm -f smpitmp-* core", shell=True, check=True) 

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/smpi/{cachefile}.timeout'):
            return 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/smpi/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/smpi/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('MC is currently not supported here', output):
            return 'failure'

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

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
        if re.search('Command return code: 0,', output):
            return 'OK'
        if re.search('Command killed by signal 15, elapsed time: 300', output):
            return 'timeout'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ({cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
