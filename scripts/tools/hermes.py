import re
import os
import sys
import tempfile
import shutil
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "Hermes wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x hermes")

    def build(self, rootdir, cached=True):
        if cached and os.path.exists('/MBI-builds/hermes/bin/ispcc') and os.path.exists('/MBI-builds/hermes/clangTool/clangTool'):
            return
        print(f"Building Hermes. Files exist: {os.path.exists('/MBI-builds/hermes/bin/ispcc')}, {os.path.exists('/MBI-builds/hermes/clangTool/clangTool')}")

        # Get a GIT checkout. Either create it, or refresh it
        subprocess.run(f"rm -rf /MBI-builds/hermes && mkdir -p /MBI-builds && git clone --depth=1 https://github.com/DhritiKhanna/Hermes.git /MBI-builds/hermes", shell=True, check=True)

        # Build it
        here = os.getcwd() # Save where we were
        os.chdir(f"/MBI-builds/hermes")
        subprocess.run("cd clangTool/ && make -j$(nproc) clangTool", shell=True, check=True)
        subprocess.run("autoreconf --install", shell=True, check=True)
        subprocess.run(f"./configure --disable-gui --prefix='/MBI-builds/hermes/' --enable-optional-ample-set-fix --with-mpi-inc-dir=/usr/lib/x86_64-linux-gnu/mpich/include CXXFLAGS='-fPIC' LDFLAGS='-lz3'", shell=True, check=True)
        subprocess.run("make -j$(nproc)", shell=True, check=True)
        subprocess.run("make -j$(nproc) install", shell=True, check=True)

        # Back to our previous directory
        os.chdir(here)

    def setup(self):
        os.environ['PATH'] = f"{os.environ['PATH']}:/MBI-builds/hermes/bin/"
        if os.path.exists("compile_commands.json"):
            return
        with open('compile_commands.json', 'w') as outfile:
            outfile.write("[{")
            outfile.write(f'  "command": "/usr/bin/cxx -c -I/usr/lib/x86_64-linux-gnu/openmpi/include/openmpi -I/MBI-builds/hermes/clangTool/ -I/usr/lib/x86_64-linux-gnu/openmpi/include -I/usr/lib/x86_64-linux-gnu/mpich/include -I/MBI-builds/hermes/clangTool/clang+llvm-3.8.0-x86_64-linux-gnu-debian8/lib/clang/3.8.0/include/ -I/usr/include/linux/ -fpermissive -pthread source.c",\n')
            outfile.write(f'          "directory": "{self.rootdir}/logs/hermes",\n')
            outfile.write(f'          "file": "{self.rootdir}/logs/hermes/source.c"\n')
            outfile.write('}]')

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        os.environ['PATH'] = f"{os.environ['PATH']}:/MBI-builds/hermes/bin/"
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "isp.exe", execcmd)
        execcmd = re.sub('-np', '-n', execcmd)
        execcmd = re.sub('\${EXE}', f"./{binary}", execcmd)
        execcmd = re.sub('\$zero_buffer', "-b", execcmd)
        execcmd = re.sub('\$infty_buffer', "-g", execcmd)

        with tempfile.TemporaryDirectory() as tmpdirname:
            self.run_cmd(
               buildcmd=f"cp {filename} source.c &&"
                       +"/MBI-builds/hermes/clangTool/clangTool source.c &&"
                       +f"ispcxx -I/MBI-builds/hermes/clangTool/ -o {tmpdirname}/{binary} i_source.c /MBI-builds/hermes/clangTool/GenerateAssumes.cpp /MBI-builds/hermes/clangTool/IfInfo.cpp /MBI-builds/hermes/profiler/Client.c",
               execcmd=execcmd,
               cachefile=cachefile,
               filename=filename,
               binary=binary,
               timeout=timeout,
               cwd=tmpdirname,
               batchinfo=batchinfo)

            if os.path.exists(f"{tmpdirname}/report.html"):
               os.rename(f"{tmpdirname}/report.html", f"{binary}_{id}.html")

#        subprocess.run("rm -f core vgcore.*", shell=True, check=True) # Save disk space ASAP
#        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/hermes/{cachefile}.timeout'):
            return 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/hermes/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/hermes/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

        # Hermes-specific
        if re.search('Detected a DEADLOCK in interleaving', output):
            return 'deadlock'

        # Inherited from ISP
        if (re.search('resource leaks detected', output) and
            not re.search('No resource leaks detected', output)):
            return 'resleak'

        if re.search('Rank [0-9]: WARNING: Waited on non-existant request in', output):
            return 'mpierr'
        if re.search('Rank [0-9]: Invalid rank in MPI_.*? at ',output):
            return 'invalid rank'
        # Invalid argument/tag/datatype/etc.
        if re.search('Fatal error in P?MPI_.*?: Invalid', output):
            if re.search('Invalid argument', output):
                return 'invalid argument'
            if re.search('Invalid communicator', output):
                return 'invalid communicator'
            if re.search('Invalid datatype', output):
                return 'invalid datatype'
            if re.search('Invalid MPI_Op', output):
                return 'invalid mpi operator'
            if re.search('Invalid tag', output):
                return 'invalid tag'
            else:
                return 'mpierr'
        if re.search('Fatal error in PMPI', output):
            return 'mpierr'
        if re.search('Fatal error in MPI', output):
            return 'mpierr'

        # https://github.com/DhritiKhanna/Hermes/issues/2
        if re.search("isp.exe: ServerSocket.cpp:220: int ServerSocket::Receive.*?: Assertion `iter != _cli_socks.end", output):
            return 'failure'
        if re.search('Command killed by signal 15, elapsed time: 300', output):
            return 'timeout'

        if re.search('Assertion failed', output):
            return 'failure'

        if re.search('Segmentation fault', output):
            return 'segfault'

        if re.search('1 warning generated', output):
            if not re.search('implicitly declaring', output):
                return 'other'

        if re.search('Command return code: 22', output):
            return 'failure'

        if re.search('Command return code: 0', output):
            return 'OK'

        if re.search('No resource leaks detected', output):
            return 'OK'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> (hermes/{cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
