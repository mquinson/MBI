import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "Intel TAC"

    def ensure_image(self):
        AbstractTool.ensure_image(self, dockerparams="--shm-size=512m ", params="-x itac")

    def setup(self, rootdir):
        subprocess.run("apt update && apt install wget", shell=True, check=True)
        subprocess.run("wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB -O- | apt-key add -", shell=True, check=True)
        subprocess.run("echo 'deb https://apt.repos.intel.com/oneapi all main' > /etc/apt/sources.list.d/oneAPI.list", shell=True, check=True)
        subprocess.run("apt update && apt install -y intel-oneapi-itac intel-oneapi-compiler-dpcpp-cpp-and-cpp-classic intel-oneapi-mpi-devel", shell=True, check=True)
        # Take the environment set by the /opt/intel/oneapi/setvars.sh shell script for us in this python script. Gosh, Intel...
        subprocess.run('bash -c "source /opt/intel/oneapi/setvars.sh && printenv" > environment.txt', shell=True, check=True)
        with open('environment.txt', "r") as input:
            for line in input:
                if re.search('=', line) is not None:
                    m = re.match('([^=]*)=(.*)', line)
                    if m is None:
                        raise Exception(f"Parse error while trying to integrating the Intel environment: {line}")
                    print(f"os.environ[{m.group(1)}]={m.group(2)}")
                    os.environ[m.group(1)] = m.group(2)

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "mpirun -check_mpi -genv VT_CHECK_TRACING on", execcmd)
        execcmd = re.sub('\${EXE}', f'./{binary}', execcmd)
        execcmd = re.sub('\$zero_buffer', "", execcmd)
        execcmd = re.sub('\$infty_buffer', "", execcmd)

        ran = run_cmd(
            buildcmd=f"mpiicc {filename} -O0 -g -o {binary}",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

        if ran: 
            subprocess.run(f"rm -f core vgcore.* {binary}", shell=True, check=True) # Save disk space ASAP

    def teardown(self): 
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True) # Remove generated cruft (binary files)
        subprocess.run("rm -f core", shell=True, check=True) 

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/itac/{cachefile}.timeout'):
            return 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/itac/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/itac/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

        match = re.search('ERROR: (.*?): (fatal )?error', output)
        if match:
#            print ('<Match: %r, groups=%r>' % (match.group(), match.groups()))
            return match.group(1)

        match = re.search('WARNING: (.*?): (fatal )?warning', output)
        if match:
            return match.group(1)
        if re.search('Command return code: 0,', output):
            return 'OK'

        if re.search('Command killed by signal 15, elapsed time: 300', output):
            return 'timeout'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> (itac/{cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
