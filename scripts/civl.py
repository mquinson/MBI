import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "CIVL wrapper"

    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x civl")

    def run(self, execcmd, filename, binary, id, timeout):
        cachefile = f'{binary}_{id}'

        execcmd = re.sub("mpirun", "java -jar ../../tools/CIVL-1.20_5259/lib/civl-1.20_5259.jar verify", execcmd)
        execcmd = re.sub('-np ', "-input_mpi_nprocs=", execcmd)
        execcmd = re.sub('\${EXE}', filename, execcmd)
        execcmd = re.sub('\$zero_buffer', "", execcmd)
        execcmd = re.sub('\$infty_buffer', "", execcmd)

        subprocess.run("killall -9 java 2>/dev/null", shell=True)

        run_cmd(
            buildcmd=None,
            execcmd=execcmd,
            cachefile=cachefile,
            binary=binary,
            timeout=timeout)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/civl/{cachefile}.timeout'):
            outcome = 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/civl/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/civl/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'
        if re.search("cannot be invoked without MPI_Init\(\) being called before", output):
            return 'mpierr'

        if re.search('DEADLOCK', output):
            return 'deadlock'

        if re.search('which has an inconsistent datatype specification with at least one of others', output):
            return 'datatype mismatch'
        if re.search('which has a different root with at least one of others', output):
            return 'root mismatch'
        if re.search('has a different MPI_Op', output):
            return 'various'

        if re.search('MPI message leak', output):
            return 'mpierr'
        if re.search('MPI_ERROR', output):
            return 'mpierr'

        if re.search('MEMORY_LEAK', output):
            return 'resleak'

        if re.search('The standard properties hold for all executions', output):
            return 'OK'

        if re.search('A CIVL internal error has occurred', output):
            return 'failure'

        if re.search('kind: UNDEFINED_VALUE, certainty: MAYBE', output):
            return 'UNDEFINED_VALUE'

        if re.search('This feature is not yet implemented', output):
            return 'UNIMPLEMENTED'
        if re.search('doesn.t have a definition', output):
            return 'UNIMPLEMENTED'
        if re.search('Undeclared identifier', output):
            return 'UNIMPLEMENTED'

        return 'other'
