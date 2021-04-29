import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def run(self, execcmd, filename, binary, id, timeout):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "isp.exe", execcmd)
        execcmd = re.sub('-np', '-n', execcmd)
        execcmd = re.sub('\${EXE}', f"./{binary}", execcmd)
        execcmd = re.sub('\$zero_buffer', "-b", execcmd)
        execcmd = re.sub('\$infty_buffer', "-g", execcmd)

        print("\nClearing port before executing ISP\n")
        subprocess.run("kill -9 $(lsof -t -i:9999) 2>/dev/null", shell=True)

        run_cmd(
            buildcmd=f"ispcc -o {binary} {filename}",
            execcmd=execcmd,
            cachefile=cachefile,
            binary=binary,
            timeout=timeout)

    def parser(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout'):
            outcome = 'timeout'
        if not os.path.exists(f'{cachefile}.txt'):
            return 'failure'

        with open(f'{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('ISP detected deadlock!!!', output):
            return 'deadlock'
        if re.search('Detected a DEADLOCK in interleaving', output):
            return 'deadlock'

        if re.search('resource leaks detected', output):
            return 'resleak'

        if re.search('ISP detected no deadlocks', output):
            return 'OK'

        if re.search('Fatal error in PMPI', output):
            return 'mpierr'
        if re.search('Fatal error in MPI', output):
            return 'mpierr'

        return 'other'
