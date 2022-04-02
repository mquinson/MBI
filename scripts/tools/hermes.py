import re
import os
import sys
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "Hermes wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x hermes")

    def build(self, rootdir, cached=True):
        if cached and os.path.exists('/builds/hermes/bin/ispcc') and os.path.exists('/builds/hermes/clangTool/clangTool'):
            return
        print(f"Building Hermes. Files exist: {os.path.exists('/builds/hermes/bin/ispcc')}, {os.path.exists('/builds/hermes/clangTool/clangTool')}")
#        subprocess.run("apt-get update && apt-get install -y libtinfo5 libtinfo-dev", shell=True, check=True)

        # Get a GIT checkout. Either create it, or refresh it
        subprocess.run(f"rm -rf /builds/hermes && mkdir /builds && git clone --depth=1 https://github.com/DhritiKhanna/Hermes.git /builds/hermes", shell=True, check=True)

        # Build it
        here = os.getcwd() # Save where we were
        os.chdir(f"/builds/hermes")
        subprocess.run("cd clangTool/ && make -j$(nproc) clangTool", shell=True, check=True)
        subprocess.run("autoreconf --install", shell=True, check=True)
        subprocess.run(f"./configure --disable-gui --enable-optional-ample-set-fix --with-mpi-inc-dir=/usr/lib/x86_64-linux-gnu/mpich/include CXXFLAGS='-fPIC' LDFLAGS='-lz3'", shell=True, check=True)
        subprocess.run("make -j$(nproc)", shell=True, check=True)
        # Back to our previous directory
        os.chdir(here)

    def setup(self):
        os.environ['PATH'] = f"{os.environ['PATH']}:/builds/hermes/bin/"
        with open('compile_commands.json', 'w') as outfile:
            outfile.write("[{")
            outfile.write(f'  "command": "/usr/bin/cxx -c -I/usr/lib/x86_64-linux-gnu/openmpi/include/openmpi -I/builds/hermes/clangTool/ -I/usr/lib/x86_64-linux-gnu/openmpi/include -I/usr/lib/x86_64-linux-gnu/mpich/include -I/builds/hermes/clangTool/clang+llvm-3.8.0-x86_64-linux-gnu-debian8/lib/clang/3.8.0/include/ -pthread source.c",\n')
            outfile.write(f'          "directory": "{self.rootdir}/logs/hermes",\n')
            outfile.write(f'          "file": "{self.rootdir}/logs/hermes/source.c"\n')
            outfile.write('}]')

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "isp.exe", execcmd)
        execcmd = re.sub('-np', '-n', execcmd)
        execcmd = re.sub('\${EXE}', f"./{binary}", execcmd)
        execcmd = re.sub('\$zero_buffer', "-b", execcmd)
        execcmd = re.sub('\$infty_buffer', "-g", execcmd)

        self.run_cmd(
            buildcmd=f"cp {filename} source.c &&"
                     +"/builds/hermes/clangTool/clangTool source.c &&"
                     +f"ispcxx -I/builds/hermes/clangTool/ -o {binary} i_source.c /builds/hermes/clangTool/GenerateAssumes.cpp /builds/hermes/clangTool/IfInfo.cpp /MBI/tools/hermes/profiler/Client.c",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

        if os.path.exists("./report.html"):
            os.rename("./report.html", f"{binary}_{id}.html")

        subprocess.run("rm -f vgcore.*", shell=True, check=True) # Save disk space ASAP
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True)

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
        if re.search('Rank [0-9]: WARNING: Waited on non-existant request in', output):
            return 'mpierr'
        if re.search('Rank [0-9]: Invalid rank in MPI_.*? at ',output):
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

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> (hermes/{cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
