# Copyright 2021-2022. The MBI project. All rights reserved.
# This program is free software; you can redistribute it and/or modify it under the terms of the license (GNU GPL).

import re
import os
from MBIutils import *

class Tool(AbstractTool):
    def identify(self):
        return "SimGrid wrapper"

    def build(self, rootdir, cached=True):
        if cached and os.path.exists('/usr/bin/simgrid-mc'):
            return

        here = os.getcwd() # Save where we were
        os.chdir(rootdir)
        # Get a GIT checkout. Either create it, or refresh it
        if os.path.exists("tools/simgrid/.git"):
            subprocess.run("git config --global --add safe.directory /MBI/tools/simgrid", shell=True, check=True)
            subprocess.run("cd tools/simgrid && git pull &&  cd ../..", shell=True, check=True)
        else:
            subprocess.run("rm -rf tools/simgrid && git clone --depth=1 https://github.com/simgrid/simgrid.git tools/simgrid", shell=True, check=True)

        # Build and install it
        os.chdir("tools/simgrid")
        subprocess.run(f"cmake -DCMAKE_INSTALL_PREFIX=/usr -Denable_model-checking=ON .", shell=True, check=True)
        subprocess.run("make -j$(nproc) install VERBOSE=1", shell=True, check=True)

        # Back to our previous directory
        os.chdir(here)


    def ensure_image(self):
        AbstractTool.ensure_image(self, "-x simgrid")

    def setup(self):
        os.environ['VERBOSE'] = '1'

    def run(self, execcmd, filename, binary, id, timeout, batchinfo):
        cachefile = f'{binary}_{id}'

        if not os.path.exists("cluster.xml"):
            with open('cluster.xml', 'w') as outfile:
                outfile.write("<?xml version='1.0'?>\n")
                outfile.write("<!DOCTYPE platform SYSTEM \"https://simgrid.org/simgrid.dtd\">\n")
                outfile.write('<platform version="4.1">\n')
                outfile.write(' <cluster id="acme" prefix="node-" radical="0-99" suffix="" speed="1Gf" bw="125MBps" lat="50us"/>\n')
                outfile.write('</platform>\n')

        execcmd = execcmd.replace("mpirun", "smpirun -wrapper simgrid-mc -platform ./cluster.xml -analyze --cfg=smpi/finalization-barrier:on --cfg=smpi/list-leaks:10 --cfg=model-check/max-depth:10000 --cfg=smpi/pedantic:true")
        execcmd = execcmd.replace('${EXE}', binary)
        execcmd = execcmd.replace('$zero_buffer', "--cfg=smpi/buffering:zero")
        execcmd = execcmd.replace('$infty_buffer', "--cfg=smpi/buffering:infty")

        self.run_cmd(
            buildcmd=f"smpicc {filename} -trace-call-location -g -Wl,-znorelro -Wl,-znoseparate-code -o {binary}",
            execcmd=execcmd,
            cachefile=cachefile,
            filename=filename,
            binary=binary,
            timeout=timeout,
            batchinfo=batchinfo)

    def teardown(self):
        subprocess.run("find -type f -a -executable | xargs rm -f", shell=True, check=True) # Remove generated cruft (binary files)
        subprocess.run("rm -f smpitmp-* core", shell=True, check=True)

    def parse(self, cachefile):
        if os.path.exists(f'{cachefile}.timeout') or os.path.exists(f'logs/simgrid/{cachefile}.timeout'):
            return 'timeout'
        if not (os.path.exists(f'{cachefile}.txt') or os.path.exists(f'logs/simgrid/{cachefile}.txt')):
            return 'failure'

        with open(f'{cachefile}.txt' if os.path.exists(f'{cachefile}.txt') else f'logs/simgrid/{cachefile}.txt', 'r') as infile:
            output = infile.read()

        if re.search('Compilation of .*? raised an error \(retcode: ', output):
            return 'UNIMPLEMENTED'

        if re.search('MBI_MSG_RACE', output):
            return 'MBI_MSG_RACE'

        if re.search('MC is currently not supported here', output):
            return 'failure'

        if re.search('Collective communication mismatch', output):
            return 'Collective mismatch'

        if re.search('DEADLOCK DETECTED', output):
            return 'deadlock'
        if re.search('returned MPI_ERR', output):
            return 'mpierr'
        if re.search('Not yet implemented', output):
            return 'UNIMPLEMENTED'
        if re.search('CRASH IN THE PROGRAM', output):
            return 'failure'
        if re.search('Probable memory leaks in your code: SMPI detected', output):
            return 'resleak'
        if re.search('DFS exploration ended.', output):
            return 'OK'

        print (f">>>>[ INCONCLUSIVE ]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> (simgrid/{cachefile})")
        print(output)
        print ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        return 'other'

    def is_correct_diagnostic(self, test_id, res_category, expected, detail):
        if res_category != 'TRUE_POS':
            return True

        possible_output = {
            'AInvalidParam' :    ['mpierr', 'deadlock'],
            'BResLeak' :         ['mpierr', 'deadlock', 'resleak'],
            'BReqLifecycle' :    ['mpierr', 'deadlock', 'resleak'],
            'BEpochLifecycle' :  ['mpierr', 'deadlock', 'resleak'],
            # 'BLocalConcurrency' :  ['mpierr'],
            'CMatch' :           ['mpierr', 'deadlock'],
            'DRace' :            ['mpierr', 'deadlock', 'MBI_MSG_RACE'],
            'DMatch' :           ['mpierr', 'deadlock', 'Collective mismatch'],
            # 'DGlobalConcurrency' : ['mpierr'],
            'EBufferingHazard' : ['mpierr', 'deadlock'],
            # 'InputHazard' : [],
            # 'FOK' : []
        }

        if possible_details[detail] not in possible_output:
            return True

        out = self.parse(test_id)

        if out not in possible_output[possible_details[detail]]:
            print(f'{test_id} : {possible_details[detail]} ({detail}) : {out}')
            return False

        return True
