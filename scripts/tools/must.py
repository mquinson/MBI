import re
import os
from MBIutils import *

def must_filter(line, process):
    if re.search("ERROR: MUST detected a deadlock", line):
        pid = process.pid
        pgid = os.getpgid(pid)
        try:
            process.terminate()
            # Send the signal to all the processes in the group. The command and everything it forked
            os.killpg(pgid, signal.SIGTERM)
        except ProcessLookupError:
            pass  # Ok, it's gone now

class V17(AbstractTool):
    def identify(self):
        return "MUST v1.7.2 wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x must")

    def build(self, rootdir, cached=True):
        if cached and os.path.exists("/MBI-builds/MUST17/bin/mustrun"):
            return

        subprocess.run(f"rm -rf /MBI-builds/MUST17", shell=True, check=True) # MUST v1.7 sometimes fails when reinstalling over the same dir

        # Build it
        here = os.getcwd() # Save where we were
        subprocess.run(f"rm -rf /tmp/build-must ; mkdir /tmp/build-must", shell=True, check=True)
        os.chdir("/tmp/build-must")
        subprocess.run(f"wget https://hpc.rwth-aachen.de/must/files/MUST-v1.7.2.tar.gz", shell=True, check=True)
        subprocess.run(f"tar xfz MUST-*.tar.gz", shell=True, check=True)
        os.chdir("/tmp/build-must/MUST-v1.7.2")

        subprocess.run(f"CC=$(which gcc) CXX=$(which gcc++) FC=$(which gfortran) cmake . -DCMAKE_INSTALL_PREFIX=/MBI-builds/MUST17 -DCMAKE_BUILD_TYPE=Release", shell=True, check=True)
        subprocess.run(f"make -j$(nproc) install VERBOSE=1", shell=True, check=True)
        subprocess.run(f"make -j$(nproc) install-prebuilds VERBOSE=1", shell=True, check=True)
        subprocess.run(f"rm -rf /tmp/build-must", shell=True, check=True)

        # Back to our previous directory
        os.chdir(here)

    def setup(self):
        os.environ['PATH'] = os.environ['PATH'] + ":/MBI-builds/MUST17/bin/"

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "mustrun --must:distributed", execcmd)
        execcmd = re.sub('\${EXE}', f'./{binary}', execcmd)
        execcmd = re.sub('\$zero_buffer', "", execcmd)
        execcmd = re.sub('\$infty_buffer', "", execcmd)

        subprocess.run("killall -9 mpirun 2>/dev/null", shell=True)

        ran = self.run_cmd(
            buildcmd=f"mpicc {filename} -o {binary}",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo,
            read_line_lambda=must_filter)

        if os.path.isfile("./MUST_Output.html"):
            os.rename(f"./MUST_Output.html", f"{cachefile}.html")

        if ran: # cleanup if that test was ran
            subprocess.run(f"rm -rf must_temp core {binary}", shell=True, check=True)

    def teardown(self):
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True) # Remove generated (binary files)
        subprocess.run("rm -rf must_temp core", shell=True, check=True)

    def parse(self, cachefile):
        # do not report timeouts ASAP, as MUST still deadlocks when it detects a root mismatch
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/must/{cachefile}.txt')):
            return 'failure'
        if not (os.path.exists(f'{cachefile}.html') or os.path.exists(f'logs/must/{cachefile}.html')):
            return 'failure'

        with open(f'{cachefile}.html' if os.path.exists(f'{cachefile}.html') else f'logs/must/{cachefile}.html', 'r') as infile:
            html = infile.read()

        if re.search('deadlock', html):
            return 'deadlock'

        if re.search('not freed', html):
            return 'resleak'

        if re.search('conflicting roots', html):
            return 'various'

        if re.search('unknown datatype', html) or re.search('has to be a non-negative integer', html) or re.search('must use equal type signatures', html):
            return 'conflicting roots'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/must/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('caught MPI error', output):
            return 'mpierr'

        if re.search('Error', html):
            return 'mpierr'

        if re.search('MUST detected no MPI usage errors nor any suspicious behavior during this application run', html):
            return 'OK'

        if re.search('YOUR APPLICATION TERMINATED WITH THE EXIT STRING: Segmentation fault', output):
            return 'segfault'
        if re.search('caught signal nr 11', output) or re.search('caught signal nr 6', output):
            return 'segfault'

        if re.search('internal ABORT - process ', output):
            return 'failure'

        # No interesting output found, so return the timeout as is if it exists
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/must/{cachefile}.timeout'):
            return 'timeout'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ({self.identify()}/{cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'

class V18(V17):
    def identify(self):
        return "MUST v1.8.1-rc1 wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x must18")

    def build(self, rootdir, cached=True):
        if cached and os.path.exists("/MBI-builds/MUST18/bin/mustrun"):
            return

        subprocess.run(f"rm -rf /MBI-builds/MUST18", shell=True, check=True) # MUST sometimes fails when reinstalling over the same dir

        # Build it
        here = os.getcwd() # Save where we were
        if not os.path.exists((f"{rootdir}/tools/MUST-v1.8.0-rc1.tar.gz")):
            subprocess.run(f"cd {rootdir}/tools; wget https://hpc.rwth-aachen.de/must/files/MUST-v1.8.0-rc1.tar.gz", shell=True, check=True)
        subprocess.run(f"rm -rf /tmp/build-must ; mkdir /tmp/build-must", shell=True, check=True)
        os.chdir("/tmp/build-must")
        subprocess.run(f"tar xfz {rootdir}/tools/MUST-v1.8.0-rc1.tar.gz", shell=True, check=True)

        subprocess.run(f"CC=$(which clang) CXX=$(which clang++) OMPI_CC=$(which clang) OMPI_CXX=$(which clang++) FC=$(which gfortran) cmake MUST-v1.8.0-rc1 -DCMAKE_INSTALL_PREFIX=/MBI-builds/MUST18 -DCMAKE_BUILD_TYPE=Release", shell=True, check=True)
        subprocess.run(f"make -j$(nproc) install VERBOSE=1", shell=True, check=True)
        subprocess.run(f"make -j$(nproc) install-prebuilds VERBOSE=1", shell=True, check=True)

        # Back to our previous directory
        os.chdir(here)

    def setup(self, rootdir):
        os.environ['PATH'] = os.environ['PATH'] + ":/MBI-builds/MUST18/bin/"
