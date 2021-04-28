#! /usr/bin/python3

import shutil
import os
import signal
import sys
import stat
import re
import argparse
import queue
import time
import multiprocessing as mp

# Add our lib directory to the PYTHONPATH, and load our utilitary libraries
sys.path.append(f'{os.path.dirname(os.path.abspath(__file__))}/scripts')
from MBIutils import *

# Some scripts may fail if error messages get translated
os.environ["LC_ALL"] = "C"

##########################
# Aislinn runner
##########################

def aislinnparse(cachefile):
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


def aislinnrun(execcmd, filename, binary, id, timeout):
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

    if os.path.exists(f'{cachefile}.elapsed'):
        with open(f'{cachefile}.elapsed', 'r') as infile:
            elapsed = infile.read()
    else:
        elapsed = 0

    return aislinnparse(cachefile), elapsed

##########################
# CIVL runner
##########################

def civlparse(cachefile):
    if os.path.exists(f'{cachefile}.timeout'):
        outcome = 'timeout'
    if not os.path.exists(f'{cachefile}.txt'):
        return 'failure'

    with open(f'{cachefile}.txt', 'r') as infile:
        output = infile.read()

    if re.search('DEADLOCK', output):
        return 'deadlock'

    if re.search('has a different root', output):
        return 'various'
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

    if re.search('This feature is not yet implemented', output):
        return 'UNIMPLEMENTED'
    if re.search('doesn.t have a definition', output):
        return 'UNIMPLEMENTED'
    if re.search('Undeclared identifier', output):
        return 'UNIMPLEMENTED'

    return 'other'

def civlrun(execcmd, filename, binary, id, timeout):
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

    if os.path.exists(f'{cachefile}.elapsed'):
        with open(f'{cachefile}.elapsed', 'r') as infile:
            elapsed = infile.read()
    else:
        elapsed = 0

    return civlparse(cachefile), elapsed

##########################
# ISP runner
##########################

def ispparser(cachefile):
    if os.path.exists(f'{cachefile}.timeout'):
        outcome = 'timeout'
    if not os.path.exists(f'{cachefile}.txt'):
        return 'failure'

    with open(f'{cachefile}.txt', 'r') as infile:
        output = infile.read()

    if re.search('ISP detected deadlock!!!', output):
        return 'deadlock'
    if re.search('Detected a DEADLOCK in interleaving', output):
        return 'deadlock'

    if re.search('resource leaks detected', output):
        return 'resleak'

    if re.search('ISP detected no deadlocks', output):
        return 'OK'

    if re.search('Fatal error in PMPI', output):
        return 'mpierr'
    if re.search('Fatal error in MPI', output):
        return 'mpierr'

    return 'other'

def isprun(execcmd, filename, binary, id, timeout):
    cachefile = f'{binary}_{id}'

    execcmd = re.sub("mpirun", "isp.exe", execcmd)
    execcmd = re.sub('-np', '-n', execcmd)
    execcmd = re.sub('\${EXE}', f"./{binary}", execcmd)
    execcmd = re.sub('\$zero_buffer', "-b", execcmd)
    execcmd = re.sub('\$infty_buffer', "-g", execcmd)

    print("\nClearing port before executing ISP\n")
    subprocess.run("kill -9 $(lsof -t -i:9999) 2>/dev/null", shell=True)

    run_cmd(
        buildcmd=f"ispcc -o {binary} {filename}",
        execcmd=execcmd,
        cachefile=cachefile,
        binary=binary,
        timeout=timeout)

    if os.path.exists(f'{cachefile}.elapsed'):
        with open(f'{cachefile}.elapsed', 'r') as infile:
            elapsed = infile.read()
    else:
        elapsed = 0

    return ispparser(cachefile), elapsed

##########################
# MPI-SV runner
##########################

def mpisvparser(cachefile):
    if os.path.exists(f'{cachefile}.timeout'):
        outcome = 'timeout'
    if not os.path.exists(f'{cachefile}.txt') or not os.path.exists(f"{binary}_{id}-klee-out/info"):
        return 'failure'

    with open(f"{binary}_{id}-klee-out/info", 'r') as infofile:
        info = infofile.read()
    with open(f'{cachefile}.txt', 'r') as infile:
        output = infile.read()

    if re.search('failed external call', output):
        return 'UNIMPLEMENTED'

    if re.search('found deadlock', output):
        return 'deadlock'

    if re.search('No Violation detected by MPI-SV', info):
        return 'OK'

    return 'other'

def mpisvrun(execcmd, filename, binary, id, timeout):
    cachefile = f'{binary}_{id}'

    execcmd = re.sub("mpirun", "mpisv", execcmd)
    execcmd = re.sub('-np ', "", execcmd)
    execcmd = re.sub('\${EXE}', f'{binary}.bc', execcmd)
    execcmd = re.sub('\$zero_buffer', "", execcmd)
    execcmd = re.sub('\$infty_buffer', "", execcmd)

    run_cmd(
        buildcmd=f"mpisvcc {filename} -o {binary}.bc",
        execcmd=execcmd,
        cachefile=cachefile,
        binary=binary,
        timeout=timeout,
        read_line_lambda=must_filter)

    if os.path.exists(f'{cachefile}.elapsed'):
        with open(f'{cachefile}.elapsed', 'r') as infile:
            elapsed = infile.read()
    else:
        elapsed = 0

    if os.path.exists('klee-last') and not os.path.exists(f"{binary}_{id}-klee-out"):
        os.rename(os.readlink('klee-last'), f"{binary}_{id}-klee-out")
        os.remove('klee-last')

    return mpisvparser(cachefile), elapsed

##########################
# MUST runner
##########################
def must_filter(line, process):
    if re.search("ERROR: MUST detected a deadlock", line):
        pid = process.pid
        pgid = os.getpgid(pid)
        try:
            process.terminate()
            # Send the signal to all the processes in the group. The command and everything it forked
            os.killpg(pgid, signal.SIGTERM)
        except ProcessLookupError:
            pass  # Ok, it's gone now

def mustparser(cachefile):
    # do not report timeouts ASAP, as MUST still deadlocks when it detects a root mismatch
    if not os.path.exists(f'{cachefile}.txt') or not os.path.exists(f'{cachefile}.html'):
        return 'failure'

    with open(f'{cachefile}.html', 'r') as infile:
        html = infile.read()

    if re.search('deadlock', html):
        return 'deadlock'

    if re.search('not freed', html):
        return 'resleak'

    if re.search('conflicting roots', html):
        return 'various'

    if re.search('unknown datatype', html) or re.search('has to be a non-negative integer', html) or re.search('must use equal type signatures', html):
        return 'mpierr'

    with open(f'{cachefile}.txt', 'r') as infile:
        output = infile.read()

    if re.search('caught MPI error', output):
        return 'mpierr'

    if re.search('Error', html):
        return 'mpierr'

    # No interesting output found, so return the timeout as is if it exists
    if os.path.exists(f'{cachefile}.timeout'):
        return 'timeout'

    return None # Give a chance to return 'timeout' if some was detected by run_cmd


def mustrun(execcmd, filename, binary, id, timeout):
    cachefile = f'{binary}_{id}'

    execcmd = re.sub("mpirun", "mustrun --must:distributed", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "", execcmd)
    execcmd = re.sub('\$infty_buffer', "", execcmd)

    subprocess.run("killall -9 mpirun 2>/dev/null", shell=True)

    run_cmd(
        buildcmd=f"mpicc {filename} -o {binary}",
        execcmd=execcmd,
        cachefile=cachefile,
        binary=binary,
        timeout=timeout,
        read_line_lambda=must_filter)

    if os.path.isfile("./MUST_Output.html"):
        os.rename(f"./MUST_Output.html", f"{cachefile}.html")

    if os.path.exists(f'{cachefile}.elapsed'):
        with open(f'{cachefile}.elapsed', 'r') as infile:
            elapsed = infile.read()
    else:
        elapsed = 0

    return mustparser(cachefile), elapsed

##########################
# Parcoach runner
##########################

def parcoachparser(cachefile):
    if os.path.exists(f'{cachefile}.timeout'):
        return 'timeout'
    if not os.path.exists(f'{cachefile}.txt'):
        return 'failure'

    with open(f'{cachefile}.txt', 'r') as infile:
        output = infile.read()

    if re.search('0 warning\(s\) issued', output):
        return 'OK'

    if re.search('missing info for external function', output):
        return 'UNIMPLEMENTED'

    return 'deadlock'


def parcoachrun(execcmd, filename, binary, id, timeout):
    cachefile = f'{binary}_{id}'

    run_cmd(
        buildcmd=f"clang -c -g -emit-llvm {filename} -I/usr/lib/x86_64-linux-gnu/mpich/include/ -o {binary}.bc",
        execcmd=f"opt-9 -load ../../builds/parcoach/src/aSSA/aSSA.so -parcoach -check-mpi {binary}.bc -o /dev/null",
        cachefile=cachefile,
        binary=binary,
        timeout=timeout)

    if os.path.exists(f'{cachefile}.elapsed'):
        with open(f'{cachefile}.elapsed', 'r') as infile:
            elapsed = infile.read()
    else:
        elapsed = 0

    return parcoachparser(cachefile), elapsed

##########################
# SimGrid runner
##########################

def simgridparser(cachefile):
    if os.path.exists(f'{cachefile}.timeout'):
        return 'timeout'
    if not os.path.exists(f'{cachefile}.txt'):
        return 'failure'

    with open(f'{cachefile}.txt', 'r') as infile:
        output = infile.read()

    if re.search('DEADLOCK DETECTED', output):
        return 'deadlock'
    if re.search('returned MPI_ERR', output):
        return 'mpierr'
    if re.search('Not yet implemented', output):
        return 'UNIMPLEMENTED'
    if re.search('CRASH IN THE PROGRAM', output):
        return 'segfault'
    if re.search('Probable memory leaks in your code: SMPI detected', output):
        return 'resleak'
    if re.search('No property violation found', output):
        return 'OK'

    print("Couldn't assign output to specific behaviour; This will be treated as 'other'")
    return 'other'

def simgridrun(execcmd, filename, binary, id, timeout):
    cachefile = f'{binary}_{id}'

    if not os.path.exists("cluster.xml"):
        with open('cluster.xml', 'w') as outfile:
            outfile.write("<?xml version='1.0'?>\n")
            outfile.write("<!DOCTYPE platform SYSTEM \"https://simgrid.org/simgrid.dtd\">\n")
            outfile.write('<platform version="4.1">\n')
            outfile.write(' <cluster id="acme" prefix="node-" radical="0-99" suffix="" speed="1Gf" bw="125MBps" lat="50us"/>\n')
            outfile.write('</platform>\n')

    execcmd = re.sub("mpirun", "smpirun -wrapper simgrid-mc -platform ./cluster.xml --cfg=smpi/list-leaks:10", execcmd)
    if re.search("rma", binary):  # DPOR reduction in simgrid cannot deal with RMA calls as they contain mutexes
        execcmd = re.sub("smpirun", "smpirun --cfg=model-check/reduction:none", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "--cfg=smpi/buffering:zero", execcmd)
    execcmd = re.sub('\$infty_buffer', "--cfg=smpi/buffering:infty", execcmd)

    run_cmd(
        buildcmd=f"smpicc {filename} -g -Wl,-znorelro -Wl,-znoseparate-code -o {binary}",
        execcmd=execcmd,
        cachefile=cachefile,
        binary=binary,
        timeout=timeout)

    if os.path.exists(f'{cachefile}.elapsed'):
        with open(f'{cachefile}.elapsed', 'r') as infile:
            elapsed = infile.read()
    else:
        elapsed = 0

    return simgridparser(cachefile), elapsed

########################
# Main script argument parsing
########################

parser = argparse.ArgumentParser(
    description='This runner intends to provide a bridge from a MPI compiler/executor + a test written with MPI bugs collection header and the actual result compared to the expected.')

parser.add_argument('filenames', metavar='example.c', nargs="+", help='a list of MPI c sources.')

parser.add_argument('-x', metavar='tool', default='mpirun',
                    help='the tool you want at execution : one among [aislinn, civl, isp, mpisv, must, simgrid, parcoach]')

parser.add_argument('-t', '--timeout', metavar='int', default=300, type=int,
                    help='timeout value at execution time, given in seconds (default: 300)')

parser.add_argument('-o', metavar='output.csv', default='out.csv', type=str,
                    help='name of the csv file in which results will be written')

args = parser.parse_args()

########################
# Usefull globals
########################

todo = []

# To compute statistics on the performance of this tool
true_pos = []
false_pos = []
true_neg = []
false_neg = []
unimplemented = []
timeout = []
failure = []

# To compute statistics on the MBI codes
code_correct = 0
code_incorrect = 0

# To compute timing statistics 
total_elapsed = 0


########################
# Going through files
########################


def extract_todo(filename):
    """
    Reads the header of the filename, and extract a list of todo item, each of them being a (cmd, expect, test_count) tupple.
    The test_count is useful to build a log file containing both the binary and the test_count, when there is more than one test in the same binary.
    """
    res = []
    test_count = 0
    with open(filename, "r") as input:
        state = 0  # 0: before header; 1: in header; 2; after header
        line_num = 1
        for line in input:
            if re.match(".*BEGIN_MBI_TESTS.*", line):
                if state == 0:
                    state = 1
                else:
                    raise Exception(f"MBI_TESTS header appears a second time at line {line_num}: \n{line}")
            elif re.match(".*END_MBI_TESTS.*", line):
                if state == 1:
                    state = 2
                else:
                    raise Exception(f"Unexpected end of MBI_TESTS header at line {line_num}: \n{line}")
            if state == 1 and re.match("\s+\$ ?.*", line):
                m = re.match('\s+\$ ?(.*)', line)
                cmd = m.group(1)
                nextline = next(input)
                if re.match('[ |]*OK *', nextline):
                    expect = 'OK'
                else:
                    m = re.match('[ |]*ERROR: *(.*)', nextline)
                    if not m:
                        print(
                            f"\n{filename}:{line_num}: MBI parse error: Test not followed by a proper 'ERROR' line:\n{line}{nextline}")
                    expect = 'ERROR' # [expects for expects in m.groups() if expects != None]
                res.append((filename, cmd, expect, test_count))
                test_count += 1
                line_num += 1

    if state == 0:
        raise Exception(f"MBI_TESTS header not found in file '{filename}'.")
    if state == 1:
        raise Exception(f"MBI_TESTS header not properly ended in file '{filename}'.")

    return res


def return_to_queue(queue, func, args):
    outcome, elapsed = func(*args)
    if elapsed is None:
        raise Exception(f"Elapsed not set for {func}({args})!")
    queue.put((outcome, elapsed))


for filename in args.filenames:
    if filename == "template.c":
        continue

    binary = re.sub('\.c', '', os.path.basename(filename))

    todo = todo + extract_todo(filename)

    if len(todo) == 0:
        raise Exception(f"No test found in {filename}. Please fix it.")

########################
# Running the tests
########################

for filename, cmd, expected, test_count in todo:
    binary = re.sub('\.c', '', os.path.basename(filename))

    print(f"Test '{binary}_{test_count}'", end=": ")
    sys.stdout.flush()

    q = mp.Queue()

    if args.x == 'mpirun':
        raise Exception("No tool was provided, please retry with -x parameter. (see -h for further information on usage)")

    elif args.x == 'must':
        func = mustrun
    elif args.x == 'mpisv':
        func = mpisvrun
    elif args.x == 'simgrid':
        func = simgridrun
    elif args.x == 'civl':
        func = civlrun
    elif args.x == 'parcoach':
        func = parcoachrun
    elif args.x == 'isp':
        func = isprun
    elif args.x == 'aislinn':
        func = aislinnrun
    else:
        raise Exception(f"The tool parameter you provided ({args.x}) is either incorect or not yet implemented.")

    p = mp.Process(target=return_to_queue, args=(q, func, (cmd, filename, binary, test_count, args.timeout)))
    p.start()
    print(f"Wait up to {args.timeout} seconds")
    sys.stdout.flush()
    p.join(args.timeout+60)
    try:
        (outcome, elapsed) = q.get(block=False)
    except queue.Empty:
        if p.is_alive():
            print("HARD TIMEOUT! The child process failed to timeout by itself. Sorry for the output.")
            p.terminate()
            outcome = 'timeout'
        else:
            outcome = 'failure'

    # Stats on the codes, even if the tool fails
    if expected == 'OK':
        code_correct += 1
    else:
        code_incorrect += 1

    # Properly categorize this run
    if outcome == 'timeout':
        res_category = 'timeout'
        if elapsed is None:
            timeout.append(f'{binary}_{test_count} (hard timeout)')
        else:
            timeout.append(f'{binary}_{test_count} (elapsed: {elapsed} sec)')
    elif outcome == 'failure':
        res_category = 'failure'
        failure.append(f'{binary}_{test_count}')
    elif outcome == 'UNIMPLEMENTED':
        res_category = 'portability issue'
        unimplemented.append(f'{binary}_{test_count}')
    elif expected == 'OK':
        if outcome == 'OK':
            res_category = 'TRUE_NEG'
            true_neg.append(f'{binary}_{test_count}')
        else:
            res_category = 'FALSE_POS'
            false_pos.append(f'{binary}_{test_count} (expected {expected} but returned {outcome})')
    elif expected == 'ERROR':
        if outcome == 'OK':
            res_category = 'FALSE_NEG'
            false_neg.append(f'{binary}_{test_count} (expected {expected} but returned {outcome})')
        else:
            res_category = 'TRUE_POS'
            true_pos.append(f'{binary}_{test_count}')
    else: 
        raise Exception(f"Unexpected expectation: {expected} (must be OK or ERROR)")

    print(f"\nTest '{binary}' result: {res_category}: {args.x} returned {outcome} while {expected} was expected. Elapsed: {elapsed} sec\n\n")

    if res_category != 'timeout':
        total_elapsed += float(elapsed)

    np = re.search(r"(?:-np) [0-9]+", cmd)
    np = int(re.sub(r"-np ", "", np.group(0)))

    zero_buff = re.search(r"\$zero_buffer", cmd)
    infty_buff = re.search(r"\$infty_buffer", cmd)
    if zero_buff != None:
        buff = '0'
    elif infty_buff != None:
        buff = 'inf'
    else:
        buff = 'NA'

    with open("./" + args.o, "a") as result_file:
        result_file.write(
            f"{binary};{test_count};{args.x};{args.timeout};{np};{buff};{expected};{outcome};{elapsed}\n")

########################
# Termination
########################

TP = len(true_pos)
TN = len(true_neg)
FP = len(false_pos)
FN = len(false_neg)
passed = TP + TN
total = passed + FP + FN + len(timeout) + len(unimplemented) + len(failure)

print(f"XXXXXXXXX Final results")
if len(false_pos) > 0:
    print(f"XXX {len(false_pos)} false positives:")
    for p in false_pos:
        print(f"  {p}")
if len(false_neg) > 0:
    print(f"XXX {len(false_neg)} false negatives:")
    for p in false_neg:
        print(f"  {p}")
if len(timeout) > 0:
    print(f"XXX {len(timeout)} timeouts:")
    for p in timeout:
        print(f"  {p}")
if len(unimplemented) > 0:
    print(f"XXX {len(unimplemented)} portability issues:")
    for p in unimplemented:
        print(f"  {p}")
if len(failure) > 0:
    print(f"XXX {len(failure)} tool failures:")
    for p in failure:
        print(f"  {p}")

def percent(ratio):
    """Returns the ratio as a percentage, rounded to 2 digits only"""
    return int(ratio*10000)/100
print(f"\nXXXX Summary for {args.x} XXXX  {passed} test{'' if passed == 1 else 's'} passed (out of {total})")
print(f"Portability: {percent(1-len(unimplemented)/total)}% ({len(unimplemented)} tests failed)")
print(f"Robustness: {percent(1-(len(timeout)+len(failure))/(total-len(unimplemented)))}% ({len(timeout)} timeouts and {len(failure)} failures)\n")
print(f"Recall: {percent(TP/(TP+FN))}% (found {TP} errors out of {TP+FN})")
print(f"Specificity: {percent(TN/(TN+FP))}% (recognized {TN} correct codes out of {TN+FP})")
print(f"Precision: {percent(TP/(TP+FP))}% ({TP} diagnostic of error are correct out of {TP+FP})")
print(f"Accuracy: {percent((TP+TN)/(TP+TN+FP+FN))}% ({TP+TN} correct diagnostics in total, out of {TP+TN+FP+FN} diagnostics)")
print(f"\nTotal time of all tests (not counting the timeouts): {total_elapsed}")
print(f"\nMBI stats: {code_correct} correct codes; {code_incorrect} incorrect codes.")