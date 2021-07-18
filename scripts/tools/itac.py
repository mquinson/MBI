import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "Intel TAC"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x itac")

    def setup(self, rootdir):
        subprocess.run("apt update && apt install wget", shell=True, check=True)
        subprocess.run("wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB -O- | apt-key add -", shell=True, check=True)
        subprocess.run("echo 'deb https://apt.repos.intel.com/oneapi all main' > /etc/apt/sources.list.d/oneAPI.list", shell=True, check=True)
        subprocess.run("apt update && apt install -y intel-oneapi-itac intel-oneapi-compiler-dpcpp-cpp-and-cpp-classic intel-oneapi-mpi-devel", shell=True, check=True)
        subprocess.run('bash -c "source /opt/intel/oneapi/setvars.sh && printenv"', shell=True, check=True)
        # This is a manual dump of the printenv after installation and 'source /opt/intel/oneapi/setvars.sh'
        os.environ['PATH'] = '/opt/intel/oneapi/mpi/2021.3.0//libfabric/bin:/opt/intel/oneapi/mpi/2021.3.0//bin:/opt/intel/oneapi/itac/2021.3.0/bin:/opt/intel/oneapi/itac/2021.3.0/bin:/opt/intel/oneapi/dev-utilities/2021.3.0/bin:/opt/intel/oneapi/debugger/10.1.1/gdb/intel64/bin:/opt/intel/oneapi/compiler/2021.3.0/linux/lib/oclfpga/llvm/aocl-bin:/opt/intel/oneapi/compiler/2021.3.0/linux/lib/oclfpga/bin:/opt/intel/oneapi/compiler/2021.3.0/linux/bin/intel64:/opt/intel/oneapi/compiler/2021.3.0/linux/bin:/opt/intel/oneapi/compiler/2021.3.0/linux/ioc/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'
        os.environ['TBBROOT'] = '/opt/intel/oneapi/tbb/2021.3.0/env/..'
        os.environ['ONEAPI_ROOT'] = '/opt/intel/oneapi'
        os.environ['SETVARS_VARS_PATH'] = '/opt/intel/oneapi/tbb/latest/env/vars.sh'
        os.environ['VT_MPI'] = 'impi4'
        os.environ['ACL_BOARD_VENDOR_PATH'] = '/opt/Intel/OpenCLFPGA/oneAPI/Boards'
        os.environ['VT_ADD_LIBS'] = '-ldwarf -lelf -lvtunwind -lm -lpthread'
        os.environ['I_MPI_ROOT'] = '/opt/intel/oneapi/mpi/2021.3.0'
        os.environ['FI_PROVIDER_PATH'] = '/opt/intel/oneapi/mpi/2021.3.0//libfabric/lib/prov:/usr/lib64/libfabric'
        os.environ['SETVARS_COMPLETED'] = '1'
        os.environ['VT_ROOT'] = '/opt/intel/oneapi/itac/2021.3.0'
        os.environ['LIBRARY_PATH'] = '/opt/intel/oneapi/tbb/2021.3.0/env/../lib/intel64/gcc4.8:/opt/intel/oneapi/mpi/2021.3.0//libfabric/lib:/opt/intel/oneapi/mpi/2021.3.0//lib/release:/opt/intel/oneapi/mpi/2021.3.0//lib:/opt/intel/oneapi/compiler/2021.3.0/linux/compiler/lib/intel64_lin:/opt/intel/oneapi/compiler/2021.3.0/linux/lib'
        os.environ['OCL_ICD_FILENAMES'] = 'libintelocl_emu.so:libalteracl.so:/opt/intel/oneapi/compiler/2021.3.0/linux/lib/x64/libintelocl.so'
        os.environ['CLASSPATH'] = '/opt/intel/oneapi/mpi/2021.3.0//lib/mpi.jar'
        os.environ['INTELFPGAOCLSDKROOT'] = '/opt/intel/oneapi/compiler/2021.3.0/linux/lib/oclfpga'
        os.environ['LD_LIBRARY_PATH'] = '/opt/intel/oneapi/tbb/2021.3.0/env/../lib/intel64/gcc4.8:/opt/intel/oneapi/mpi/2021.3.0//libfabric/lib:/opt/intel/oneapi/mpi/2021.3.0//lib/release:/opt/intel/oneapi/mpi/2021.3.0//lib:/opt/intel/oneapi/itac/2021.3.0/slib:/opt/intel/oneapi/debugger/10.1.1/dep/lib:/opt/intel/oneapi/debugger/10.1.1/libipt/intel64/lib:/opt/intel/oneapi/debugger/10.1.1/gdb/intel64/lib:/opt/intel/oneapi/compiler/2021.3.0/linux/lib:/opt/intel/oneapi/compiler/2021.3.0/linux/lib/x64:/opt/intel/oneapi/compiler/2021.3.0/linux/lib/emu:/opt/intel/oneapi/compiler/2021.3.0/linux/lib/oclfpga/host/linux64/lib:/opt/intel/oneapi/compiler/2021.3.0/linux/lib/oclfpga/linux64/lib:/opt/intel/oneapi/compiler/2021.3.0/linux/compiler/lib/intel64_lin:/opt/intel/oneapi/compiler/2021.3.0/linux/compiler/lib'
        os.environ['VT_LIB_DIR'] = '/opt/intel/oneapi/itac/2021.3.0/lib'
        os.environ['VT_SLIB_DIR'] = '/opt/intel/oneapi/itac/2021.3.0/slib'
        os.environ['INTEL_PYTHONHOME'] = '/opt/intel/oneapi/debugger/10.1.1/dep'
        os.environ['INTEL_LICENSE_FILE'] = '/opt/intel/licenses:/root/intel/licenses:'
        os.environ['CPATH'] = '/opt/intel/oneapi/tbb/2021.3.0/env/../include:/opt/intel/oneapi/mpi/2021.3.0//include:/opt/intel/oneapi/dpl/2021.3.0/linux/include:/opt/intel/oneapi/dev-utilities/2021.3.0/include:/opt/intel/oneapi/compiler/2021.3.0/linux/include'

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "mpirun -check_mpi -genv VT_CHECK_TRACING on", execcmd)
        execcmd = re.sub('\${EXE}', f'./{binary}', execcmd)
        execcmd = re.sub('\$zero_buffer', "", execcmd)
        execcmd = re.sub('\$infty_buffer', "", execcmd)

        run_cmd(
            buildcmd=f"mpiicc {filename} -O0 -g -o {binary}",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

        subprocess.run("rm -f vgcore.*", shell=True, check=True) # Save disk space ASAP

    def teardown(self): 
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True) # Remove generated cruft (binary files)
        subprocess.run("rm -f smpitmp-* core", shell=True, check=True) 

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/itac/{cachefile}.timeout'):
            outcome = 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/itac/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/itac/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

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

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ({cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
