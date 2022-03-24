import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "ISP wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x isp")

    def build(self, rootdir, cached=True):
        if cached and os.path.exists(f"{rootdir}/builds/ISP/bin/ispcc"):
            return

        # Build it
        here = os.getcwd() # Save where we were
        os.chdir(f"{rootdir}/tools/isp-0.3.1")
        subprocess.run(f"./configure --prefix={rootdir}/builds/ISP --with-mpi-inc-dir=/usr/lib/x86_64-linux-gnu/mpich/include --enable-optional-ample-set-fix", shell=True, check=True)
        subprocess.run(f'sed -i "s/-source 1.5 -target 1.5 -classpath/-source 1.7 -target 1.7 -classpath/" UI/Makefile*', shell=True, check=True)
        subprocess.run("make -j$(nproc) install", shell=True, check=True)

        # Back to our previous directory
        os.chdir(here)

    def setup(self, rootdir):
        os.environ['PATH'] = f"{os.environ['PATH']}:{rootdir}/builds/ISP/bin/"

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "isp.exe", execcmd)
        execcmd = re.sub('-np', '-n', execcmd)
        execcmd = re.sub('\${EXE}', f"./{binary}", execcmd)
        execcmd = re.sub('\$zero_buffer', "-b", execcmd)
        execcmd = re.sub('\$infty_buffer', "-g", execcmd)


        if run_cmd(buildcmd=f"ispcc -o {binary} {filename}",
                   execcmd=execcmd,
                   cachefile=cachefile,
                   filename=filename,
                   binary=binary,
                   timeout=timeout,
                   batchinfo=batchinfo):
                   
            # The test was actually run
            print("\nClearing port after executing ISP\n")
            subprocess.run("kill -9 $(lsof -t -i:9999) 2>/dev/null", shell=True)

    def teardown(self): # Remove generated cruft (binary files)
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/isp/{cachefile}.timeout'):
            return 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/isp/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/isp/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('ISP detected deadlock!!!', output):
            return 'deadlock'
        if re.search('Detected a DEADLOCK in interleaving', output):
            return 'deadlock'

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

        if re.search('resource leaks detected', output):
            return 'resleak'

        if re.search("Attempting to use an MPI routine after finalizing MPI", output):
            return 'mpierr'

        if re.search('Rank [0-9]: WARNING: Waited on non-existant request in', output):
            return 'mpierr'
        if re.search('Rank [0-9]: Invalid rank in MPI_.*? at ',output):
            return 'mpierr'
        if re.search('Fatal error in PMPI', output):
            return 'mpierr'
        if re.search('Fatal error in MPI', output):
            return 'mpierr'

        if re.search('ISP detected no deadlocks', output):
            return 'OK'

        if re.search('BAD TERMINATION OF ONE OF YOUR APPLICATION PROCESSES', output):
            return 'failure'

        if re.search('Command killed by signal 15, elapsed time: 300', output):
            return 'timeout'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> (isp/{cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
