import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "PARCOACH wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x parcoach")

    def build(self, rootdir, cached=True):
        if cached and os.path.exists(f"/MBI-builds/parcoach/src/aSSA/aSSA.so"):
            print("No need to rebuild ParCoach.")
            return
        if not os.path.exists("/usr/lib/llvm-9/bin/clang"):
            subprocess.run("ln -s $(which clang) /usr/lib/llvm-9/bin/clang", shell=True, check=True)

        here = os.getcwd() # Save where we were
        # Get a GIT checkout.
        subprocess.run("rm -rf /tmp/parcoach && git clone --depth=1 https://github.com/parcoach/parcoach.git /tmp/parcoach", shell=True, check=True)
        # Go to where we want to install it, and build it out-of-tree (we're in the docker)
        subprocess.run("mkdir -p /MBI-builds/parcoach", shell=True, check=True)
        os.chdir('/MBI-builds/parcoach')
        subprocess.run(f"cmake /tmp/parcoach -DCMAKE_C_COMPILER=clang -DLLVM_DIR={rootdir}/tools/Parcoach/llvm-project/build", shell=True, check=True)
        subprocess.run("make -j$(nproc) VERBOSE=1", shell=True, check=True)
        subprocess.run("rm -rf /tmp/parcoach", shell=True, check=True)

        # Back to our previous directory
        os.chdir(here)

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        self.run_cmd(
            buildcmd=f"clang -c -g -emit-llvm {filename} -I/usr/lib/x86_64-linux-gnu/mpich/include/ -o {binary}.bc",
            execcmd=f"opt-9 -load /MBI-builds/parcoach/src/aSSA/aSSA.so -parcoach -check-mpi {binary}.bc -o /dev/null",
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

        subprocess.run("rm -f *.bc core", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/parcoach/{cachefile}.timeout'):
            return 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/parcoach/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/parcoach/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('0 warning\(s\) issued', output):
            return 'OK'

        if re.search('missing info for external function', output):
            return 'UNIMPLEMENTED'

        return 'deadlock'

    def is_correct_diagnostic(self, test_id, res_category, expected, detail):
        # PARCOACH detect only call ordering errors
        if res_category != 'TRUE_POS':
            return True

        if possible_details[detail] == "DMatch":
            return True

        if possible_details[detail] == "InputHazard":
            return True

        return False
