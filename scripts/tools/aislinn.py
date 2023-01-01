import re
import os
import sys
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "Aislinn wrapper"

    def ensure_image(self):
        id = subprocess.run("grep '^ID=' /etc/os-release|sed 's/.*=//'", shell=True, capture_output=True, text=True)
        ver = subprocess.run("grep '^VERSION_ID=' /etc/os-release|sed 's/.*=//'", shell=True, capture_output=True, text=True)
        if id.stdout == "ubuntu\n" and ver.stdout == '"18.04"\n':
            print("This is an Ubuntu 18.04 OS. Good.")
        else:
            print(f"id: '{id.stdout}'; version: '{ver.stdout}'")
            print("Please run this script in a ubuntu:18.04 image. Run these commands:")
            print("  docker image pull ubuntu:18.04")
            print("  docker run -it --rm --name MIB --volume $(pwd):/MBI ubuntu:18.04 /MBI/MBI.py -x aislinn")
            sys.exit(1)

    def build(self, rootdir, cached=True):
        print("Aislinn is only built during setup, if really needed (it's not using the same docker image).")

    def setup(self):
        os.environ['PATH'] = os.environ['PATH'] + ":/MBI-builds/aislinn/bin/"

        if os.path.exists("/tmp/aislinn.configured"):
            return
        subprocess.run("touch /tmp/aislinn.configured", shell=True, check=True)

        subprocess.run("apt-get install -y gcc python2.7 python-jinja2", shell=True, check=True)
        if not os.path.exists(f"/MBI-builds/aislinn/bin/aislinn-cc"):
            print("XX Building aislinn")
            subprocess.run("apt-get update && apt-get install -y gcc python3.8 autotools-dev automake build-essential git", shell=True, check=True)

            # Get a GIT checkout. Either create it, or refresh it
            if os.path.exists("/MBI-builds/aislinn/.git"):
                subprocess.run("cd /MBI-builds/aislinn && git pull &&  cd ../..", shell=True, check=True)
            else:
                subprocess.run(f"rm -rf /MBI-builds/aislinn; mkdir -p /MBI-builds", shell=True, check=True)
                subprocess.run(f"git clone --depth=1 https://github.com/spirali/aislinn.git /MBI-builds/aislinn", shell=True, check=True)
                subprocess.run(f"git clone --recursive https://github.com/spirali/aislinn-valgrind /MBI-builds/aislinn/valgrind", shell=True, check=True)

            # Build it
            here = os.getcwd() # Save where we were
            os.chdir(f"/MBI-builds/aislinn/valgrind")
            subprocess.run("sh autogen.sh && ./configure && make -j$(nproc)", shell=True, check=True)

            os.chdir(f"/MBI-builds/aislinn")
            subprocess.run("./waf configure && ./waf", shell=True, check=True)

            # Back to our previous directory
            os.chdir(here)
            print("XX Done building aislinn")
        
    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "aislinn", execcmd)
        execcmd = re.sub('\${EXE}', binary, execcmd)
        execcmd = re.sub('\$zero_buffer', "--send-protocol=rendezvous", execcmd)
        execcmd = re.sub('\$infty_buffer', "--send-protocol=eager", execcmd)
        execcmd = re.sub('-np ', '-p=', execcmd)

        ran = self.run_cmd(
            buildcmd=f"aislinn-cc -g {filename} -o {binary}",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

        if os.path.exists("./report.html"):
            os.rename("./report.html", f"{binary}_{id}.html")

        if ran:
            subprocess.run(f"rm -f {binary} vgcore.*", shell=True, check=True) # Save disk space ASAP

    def teardown(self): # Remove generated cruft (binary files)
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/aislinn/{cachefile}.timeout'):
            return 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/aislinn/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/aislinn/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

        if re.search('No errors found', output):
            return 'OK'

        if re.search("INFO: Found error 'Deadlock'", output):
            return 'deadlock'
        if re.search("INFO: Found error 'Pending message'", output):
            return 'Pending message'

        if re.search("INFO: Found error 'Invalid color'", output):
            return 'Invalid color'
        if re.search("INFO: Found error 'Invalid communicator'", output):
            return 'Invalid communicator'
        if re.search("INFO: Found error 'Invalid count'", output):
            return 'Invalid count'
        if re.search("INFO: Found error 'Invalid datatype'", output):
            return 'Invalid datatype'
        if re.search("INFO: Found error 'Invalid group'", output):
            return 'Invalid group'
        if re.search("INFO: Found error 'Invalid operation'", output):
            return 'Invalid operation'
        if re.search("INFO: Found error 'Invalid rank'", output):
            return 'Invalid rank'
        if re.search("INFO: Found error 'Invalid request'", output):
            return 'Invalid request'
        if re.search("INFO: Found error 'Invalid tag'", output):
            return 'Invalid tag'

        if re.search("INFO: Found error 'Invalid write'", output):
            return 'concurrency error'
        if re.search("INFO: Found error 'Request is not persistent'", output):
            return 'Request is not persistent'
        if re.search("INFO: Found error 'Pending request'", output):
            return 'Pending request'

        if re.search("INFO: Found error 'Collective operation: root mismatch'", output):
            return 'Collective operation: root mismatch'
        if re.search("INFO: Found error 'Collective operation mismatch'", output):
            return 'Collective operation mismatch'
        if re.search("INFO: Found error 'Mixing blocking and nonblocking collective operation'", output):
            return 'Mixing blocking and nonblocking collective operation'


        if re.search('Unkn?own function call', output) or re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search("Traceback \(most recent call last\):", output):
            return 'failure'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> (aislinn/{cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'
