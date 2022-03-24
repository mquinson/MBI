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
            print("  docker run -it --rm --name MIB --shm-size=512m --volume $(pwd):/MBI mpisv/mpi-sv  /MBI/scripts/ensure_python3 /MBI/MBI.py -x mpisv")
            sys.exit(1)

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "mpisv", execcmd)
        execcmd = re.sub('-np ', "", execcmd)
        execcmd = re.sub('\${EXE}', f'{binary}.bc', execcmd)
        execcmd = re.sub('\$zero_buffer', "", execcmd)
        execcmd = re.sub('\$infty_buffer', "", execcmd)

        ran = run_cmd(
            buildcmd=f"mpisvcc {filename} -o {binary}.bc",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)
        
        if os.path.exists('klee-last') and not os.path.exists(f"{binary}_{id}-klee-out"):
            os.rename(os.readlink('klee-last'), f"{binary}_{id}-klee-out")
            os.remove('klee-last')
        
        # save disk space ASAP if ran
        if ran:
            subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True) # Remove generated cruft (binary files)
            subprocess.run("find -name '*.bin' -o -name '*.istats' -o -name 'pid' -o -name '*.ll' | xargs rm -f", shell=True, check=True) # Remove cruft generated by Klee
            subprocess.run("rm -rf klee-last core", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/mpisv/{cachefile}.timeout'):
            return 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/mpisv/{cachefile}.txt')):
            return 'failure'
        if not (os.path.exists(f'{cachefile}-klee-out/info') or os.path.exists(f'logs/mpisv/{cachefile}-klee-out/info')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/mpisv/{cachefile}.txt', 'r') as infile:
            output = infile.read()
        with open(f'{cachefile}-klee-out/info' if os.path.exists(f'{cachefile}-klee-out/info') else f'logs/mpisv/{cachefile}-klee-out/info', 'r') as infile:
            info = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output) or re.search('failed external call', output):
            return 'UNIMPLEMENTED'

        if re.search('found deadlock', output):
            return 'deadlock'

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

        if re.search('klee: .*? Assertion `.*? failed.', output):
            return 'failure'

        if re.search('No Violation detected by MPI-SV', info):
            return 'OK'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> (mpisv/{cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
