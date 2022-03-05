import re
import os
import tools.smpi 
import subprocess
from MBIutils import *

class Tool(tools.smpi.Tool):
    def identify(self):
        return "SimGrid MPI with Valgrind wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x smpivg")

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        if not os.path.exists('simgrid.supp'):
            if os.path.exists('../../simgrid.supp'):
                print(f"\nCopying simgrid.supp from {os.getcwd()}/../.. to {os.getcwd()}.")
                subprocess.run("cp ../../simgrid.supp .", shell=True, check=True)
            else:
                print(f"\nDownloading simgrid.supp in {os.getcwd()}.")
                subprocess.run("apt-get update", shell=True, check=True)
                subprocess.run("apt-get install -y wget", shell=True, check=True)
                subprocess.run("wget 'https://framagit.org/simgrid/simgrid/-/raw/master/tools/simgrid.supp?inline=false' -O simgrid.supp", shell=True, check=True)
        else:
            print(f"\nsimgrid.supp found in {os.getcwd()}, no need to download it.")

        tools.smpi.Tool.run(self, execcmd, filename, binary, id, timeout, batchinfo, extraargs="-wrapper 'valgrind --leak-check=no --suppressions=simgrid.supp'")
        subprocess.run("rm -f vgcore.*", shell=True, check=True) # Save disk space ASAP

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/smpivg/{cachefile}.timeout'):
            outcome = 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/smpivg/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/smpivg/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('ERROR SUMMARY: [^0]', output):
            return 'failure'

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
        if re.search('Command killed by signal 15, elapsed time: ', output):
            return 'timeout'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ({cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
