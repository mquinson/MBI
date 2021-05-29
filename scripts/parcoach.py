import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "PARCOACH wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x parcoach")

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        run_cmd(
            buildcmd=f"clang -c -g -emit-llvm {filename} -I/usr/lib/x86_64-linux-gnu/mpich/include/ -o {binary}.bc",
            execcmd=f"opt-9 -load ../../builds/parcoach/src/aSSA/aSSA.so -parcoach -check-mpi {binary}.bc -o /dev/null",
            cachefile=cachefile,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

    def teardown(self): 
        subprocess.run("rm -f *.bc core", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/parcoach/{cachefile}.timeout'):
            outcome = 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/parcoach/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/parcoach/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('0 warning\(s\) issued', output):
            return 'OK'

        if re.search('missing info for external function', output):
            return 'UNIMPLEMENTED'

        return 'deadlock'
