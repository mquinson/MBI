import re
import os
import sys
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "MPI-SV wrapper"

    def ensure_image(self):
        if os.path.exists("/root/mpi-sv/mpisv"):
            print("This is the docker image of MPI-SV. Good.")
        else:
            print("Please run this script in a mpisv/mpi-sv image. Run these commands:")
            print("  docker image pull mpisv/mpi-sv")
            print("  docker run -it --rm --name MIB --volume $(pwd):/MBI mpisv/mpi-sv /MBI/MBI.py -x mpisv")
            sys.exit(1)

    def run(self, execcmd, filename, binary, id, timeout):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "mpisv", execcmd)
        execcmd = re.sub('-np ', "", execcmd)
        execcmd = re.sub('\${EXE}', f'{binary}.bc', execcmd)
        execcmd = re.sub('\$zero_buffer', "", execcmd)
        execcmd = re.sub('\$infty_buffer', "", execcmd)

        run_cmd(
            buildcmd=f"mpisvcc {filename} -o {binary}.bc",
            execcmd=execcmd,
            cachefile=cachefile,
            binary=binary,
            timeout=timeout)
        
        if os.path.exists('klee-last') and not os.path.exists(f"{binary}_{id}-klee-out"):
            os.rename(os.readlink('klee-last'), f"{binary}_{id}-klee-out")
            os.remove('klee-last')

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout'):
            outcome = 'timeout'
        if not os.path.exists(f'{cachefile}.txt') or not os.path.exists(f"{binary}_{id}-klee-out/info"):
            return 'failure'

        with open(f"{binary}_{id}-klee-out/info", 'r') as infofile:
            info = infofile.read()
        with open(f'{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('failed external call', output):
            return 'UNIMPLEMENTED'

        if re.search('found deadlock', output):
            return 'deadlock'

        if re.search('No Violation detected by MPI-SV', info):
            return 'OK'

        return 'other'
