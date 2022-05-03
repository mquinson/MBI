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

        with open(f"{rootdir}/compile_commands.json", "w") as out:
            out.write("""[
  {
    \"directory\": \"/MBI/\",
    \"command\": \"mpicc /MBI/gencodes/*.c -I/usr/include/x86_64-linux-gnu/mpich/\",
    \"file\": \"/MBI/gencodes/*.c\"
  }
]
""")

        subprocess.run("clang-tidy-11 --list-checks", shell=True, check=True)

        os.chdir(here)


    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        ran = self.run_cmd(
            buildcmd="",
            execcmd=f"clang-tidy-11 {filename} -checks='-*,clang-analyzer-optin.mpi.MPI-Checker'",
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/mpi-checker/{cachefile}.timeout'):
            outcome = 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/mpi-checker/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/mpi-checker/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if (re.search('no matching nonblocking call', output)
            or re.search('no matching wait', output)):
            return 'REQUEST_LIFECYCLE'

        if (re.search('error:', output)
            or re.search('warning:', output)):
            return 'error'

        return 'OK'
