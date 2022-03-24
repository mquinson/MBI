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
        if cached and os.path.exists(f"{rootdir}/builds/hermes/bin/ispcc") and os.path.exists(f"{rootdir}/tools/hermes/clangTool/clangTool"):
            return
#        subprocess.run("apt-get update && apt-get install -y libtinfo5 libtinfo-dev", shell=True, check=True)

        # Get a GIT checkout. Either create it, or refresh it
        if os.path.exists(f"{rootdir}/tools/hermes/.git"):
            subprocess.run(f"cd {rootdir}/tools/hermes && git pull &&  cd ../..", shell=True, check=True)
        else:
            subprocess.run(f"rm -rf {rootdir}/tools/hermes && git clone --depth=1 https://github.com/DhritiKhanna/Hermes.git {rootdir}/tools/hermes", shell=True, check=True)

        # Build it
        here = os.getcwd() # Save where we were
        os.chdir(f"{rootdir}/tools/hermes")
        subprocess.run("cd clangTool/ && make -j$(nproc) clangTool", shell=True, check=True)
        subprocess.run("autoreconf --install", shell=True, check=True)
        subprocess.run(f"./configure --disable-gui --prefix={rootdir}/builds/hermes --enable-optional-ample-set-fix --with-mpi-inc-dir=/usr/lib/x86_64-linux-gnu/mpich/include CXXFLAGS='-fPIC' LDFLAGS='-lz3'", shell=True, check=True)
        subprocess.run("make -j$(nproc) install", shell=True, check=True)
        # Back to our previous directory
        os.chdir(here)

    def setup(self, rootdir):
        os.environ['PATH'] = f"{os.environ['PATH']}:{rootdir}/builds/hermes/bin/"
        with open('compile_commands.json', 'w') as outfile:
            outfile.write("[{")
            outfile.write(f'  "command": "/usr/bin/cxx -c -I/usr/lib/x86_64-linux-gnu/openmpi/include/openmpi -I{rootdir}/tools/hermes/clangTool/ -I/usr/lib/x86_64-linux-gnu/openmpi/include -I/usr/lib/x86_64-linux-gnu/mpich/include -I{rootdir}/tools/hermes/clangTool/clang+llvm-3.8.0-x86_64-linux-gnu-debian8/lib/clang/3.8.0/include/ -pthread source.c",\n')
            outfile.write(f'          "directory": "{rootdir}/logs/hermes",\n')
            outfile.write(f'          "file": "{rootdir}/logs/hermes/source.c"\n')
            outfile.write('}]')

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "isp.exe", execcmd)
        execcmd = re.sub('-np', '-n', execcmd)
        execcmd = re.sub('\${EXE}', f"./{binary}", execcmd)
        execcmd = re.sub('\$zero_buffer', "-b", execcmd)
        execcmd = re.sub('\$infty_buffer', "-g", execcmd)

        run_cmd(
            buildcmd=f"cp {filename} source.c &&"
                     +"/MBI/tools/hermes/clangTool/clangTool source.c &&"
                     +f"ispcxx -I/MBI/tools/hermes/clangTool/ -o {binary} i_source.c /MBI/tools/hermes/clangTool/GenerateAssumes.cpp /MBI/tools/hermes//clangTool/IfInfo.cpp /MBI/tools/hermes/profiler/Client.c",
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
