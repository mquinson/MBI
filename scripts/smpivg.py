import re
import os
import smpi 
import subprocess

class Tool(smpi.Tool):
    def identify(self):
        return "SimGrid MPI with Valgrind wrapper"

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        if not os.path.exists('simgrid.supp'):
            subprocess.run("apt-get update", shell=True, check=True)
            subprocess.run("apt-get install -y wget", shell=True, check=True)
            subprocess.run("wget 'https://framagit.org/simgrid/simgrid/-/raw/master/tools/simgrid.supp?inline=false' -O simgrid.supp", shell=True, check=True)

        smpi.Tool.run(self, execcmd, filename, binary, id, timeout, batchinfo, extraargs="-wrapper 'valgrind --suppressions=simgrid.supp'")
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
