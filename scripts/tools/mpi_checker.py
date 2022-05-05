import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "MPI-Checker wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x mpi-checker")

    def build(self, rootdir, cached=True):
        here = os.getcwd()
        os.chdir(rootdir)

        subprocess.run("clang-tidy-11 --list-checks", shell=True, check=True)

        os.chdir(here)


    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        with open("/MBI/compile_commands.json", "w") as out:
            out.write(f'[{{"directory": "/MBI/", "command": "mpicc {filename} -I/usr/include/x86_64-linux-gnu/mpich/", "file": "{filename}"}}]')

        ran = self.run_cmd(
            buildcmd=f"run-clang-tidy-11.py -export-fixes='/MBI/logs/mpi-checker/{binary}_{id}.yaml' -checks='-*,mpi-type-mismatch,mpi-buffer-deref'",
            execcmd=f"analyze-build --output /MBI/logs/mpi-checker/{cachefile} --cdb /MBI/compile_commands.json --enable-checker optin.mpi.MPI-Checker",
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

        subprocess.run(f"chmod -R +r /MBI/logs/mpi-checker/{cachefile}", shell=True, check=True)
        subprocess.run("rm /MBI/compile_commands.json", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/mpi-checker/{cachefile}.timeout'):
            outcome = 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/mpi-checker/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/mpi-checker/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        # with open(f'{cachefile}.yaml' if os.path.exists(f'{cachefile}.yaml') else f'logs/mpi-checker/{cachefile}.yaml', 'r') as infile:
        #     output = infile.read()

        # Request lifecycle errors
        if re.search('no matching wait', output):
            return 'Missing wait'

        if re.search('no matching nonblocking call', output):
            return 'Unmatched wait'

        # Read HTML report
        report = []
        for root, dirs, files in os.walk(f"logs/mpi-checker/{cachefile}"):
            if "index.html" in files:
                report.append(os.path.join(root, "index.html"))

        for html in report:
            with open(html, 'r') as infile:
                output = infile.read()

            if re.search('Missing wait', output):
                return 'Missing wait'

            if re.search('Unmatched wait', output):
                return 'Unmatched wait'

            if re.search('MPI Error', output):
                return 'mpierror'

        # Non catched errors
        if (re.search('error:', output)
            or re.search('warning:', output)):
            return 'failure'

        # if re.search('warning', output):
        #     return 'SUPPRESSED_WARNING'

        return 'OK'
