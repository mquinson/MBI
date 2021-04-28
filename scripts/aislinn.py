import re
import os
from MBIutils import *

def run(execcmd, filename, binary, id, timeout):
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

def parse(cachefile):
    if os.path.exists(f'{cachefile}.timeout'):
        outcome = 'timeout'
    if not os.path.exists(f'{cachefile}.txt'):
        return 'failure'

    with open(f'{cachefile}.txt', 'r') as infile:
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

    if re.search('Invalid rank', output):
        return 'mpierr'
    if re.search('Invalid datatype', output):
        return 'mpierr'
    if re.search('Invalid communicator', output):
        return 'mpierr'
    if re.search('Invalid color', output):
        return 'mpierr'
    if re.search('Invalid operation', output):
        return 'mpierr'
    if re.search('Invalid count', output):
        return 'mpierr'

    if re.search('Collective operation: root mismatch', output):
        return 'various'

    if re.search('Unkown function call', output):
        return 'UNIMPLEMENTED'

    return 'other'
