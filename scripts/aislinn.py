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

    def setup(self, rootdir):
        subprocess.run("apt-get install -y gcc python2.7 python-jinja2", shell=True, check=True)
        os.environ['PATH'] = os.environ['PATH'] + ":" + rootdir + "/tools/aislinn-git/bin/"

    def run(self, execcmd, filename, binary, id, timeout):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "aislinn", execcmd)
        execcmd = re.sub('\${EXE}', binary, execcmd)
        execcmd = re.sub('\$zero_buffer', "--send-protocol=rendezvous", execcmd)
        execcmd = re.sub('\$infty_buffer', "--send-protocol=eager", execcmd)
        execcmd = re.sub('-np ', '-p=', execcmd)

        run_cmd(
            buildcmd=f"aislinn-cc -g {filename} -o {binary}",
            execcmd=execcmd,
            cachefile=cachefile,
            binary=binary,
            timeout=timeout)

        if os.path.exists("./report.html"):
            os.rename("./report.html", f"{binary}_{id}.html")

    def teardown(self): # Remove generated cruft (binary files)
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/aislinn/{cachefile}.timeout'):
            outcome = 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/aislinn/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/aislinn/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('No errors found', output):
            return 'OK'

        if re.search('Deadlock', output):
            return 'deadlock'
        if re.search('Collective operation mismatch', output):
            return 'deadlock'
        if re.search('Mixing blocking and nonblocking collective operation', output):
            return 'deadlock'
        if re.search('Pending message', output):
            return 'deadlock'

        if re.search("INFO: Found error 'Invalid rank'", output):
            return 'mpierr'
        if re.search("INFO: Found error 'Invalid tag'", output):
            return 'mpierr'
        if re.search("INFO: Found error 'Invalid datatype'", output):
            return 'mpierr'
        if re.search("INFO: Found error 'Invalid communicator'", output):
            return 'mpierr'
        if re.search("INFO: Found error 'Invalid color'", output):
            return 'mpierr'
        if re.search("INFO: Found error 'Invalid operation'", output):
            return 'mpierr'
        if re.search("INFO: Found error 'Invalid count'", output):
            return 'mpierr'

        if re.search('Collective operation: root mismatch', output):
            return 'various'

        if re.search('Unkown function call', output) or re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search("Traceback \(most recent call last\):", output):
            return 'failure'

        return 'other'
