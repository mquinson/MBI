#! /MBI/scripts/ensure_python3

# autopep8 -i --max-line-length 130 MBI.py

import shutil
import os
import signal
import sys
import stat
import re
import argparse
import time
import glob
import subprocess
import multiprocessing as mp

# Add our lib directory to the PYTHONPATH, and load our utilitary libraries
sys.path.append(f'{os.path.dirname(os.path.abspath(__file__))}/scripts')

import parcoach
import simgrid
import must
import mpisv
import isp
import civl
import aislinn

tools = {'aislinn': aislinn.Tool(), 'civl': civl.Tool(), 'isp': isp.Tool(), 'mpisv': mpisv.Tool(),
         'must': must.Tool(), 'simgrid': simgrid.Tool(), 'parcoach': parcoach.Tool()}

# Some scripts may fail if error messages get translated
os.environ["LC_ALL"] = "C"

########################
# Extract the TODOs from the codes
########################

possible_details = [
    # scope limited to one call
    'InvalidCommunicator', 'InvalidDatatype', 'InvalidRoot', 'InvalidTag', 'InvalidWindow', 'InvalidOperator', 'InvalidOtherArg', 'ActualDatatype',
    # scope: Process-wide
    'OutOfInitFini', 'CommunicatorLeak', 'DatatypeLeak', 'GroupLeak', 'OperatorLeak', 'TypeLeak', 'MissingStart', 'MissingWait',
    'RequestLeak', 'LocalConcurrency',
    # scope: communicator
    'MessageRace', 'CallMatching', 'CommunicatorMatching', 'DatatypeMatching', 'InvalidSrcDest', 'OperatorMatching', 'OtherArgMatching', 'RootMatching', 'TagMatching',
    # larger scope
    'BufferingHazard']
# BufferLength/BufferOverlap
# RMA concurrency errors (local and distributed)

todo = []


def extract_todo(filename):
    """
    Reads the header of the provided filename, and extract a list of todo item, each of them being a (cmd, expect, test_num) tupple.
    The test_num is useful to build a log file containing both the binary and the test_num, when there is more than one test in the same binary.
    """
    res = []
    test_num = 0
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
                detail = None
                if re.match('[ |]*OK *', nextline):
                    expect = 'OK'
                else:
                    m = re.match('[ |]*ERROR: *(.*)', nextline)
                    if not m:
                        raise Exception(
                            f"\n{filename}:{line_num}: MBI parse error: Test not followed by a proper 'ERROR' line:\n{line}{nextline}")
                    expect = 'ERROR'
                    detail = m.group(1)
                    if detail not in possible_details:
                        raise Exception(
                            f"\n{filename}:{line_num}: MBI parse error: Detailled outcome {detail} is not one of the allowed ones.")
                test = {'filename': filename, 'id': test_num, 'cmd': cmd, 'expect': expect, 'detail': detail}
                res.append(test.copy())
                test_num += 1
                line_num += 1

    if state == 0:
        raise Exception(f"MBI_TESTS header not found in file '{filename}'.")
    if state == 1:
        raise Exception(f"MBI_TESTS header not properly ended in file '{filename}'.")

    if len(res) == 0:
        raise Exception(f"No test found in {filename}. Please fix it.")
    return res


def extract_all_todo(batch):
    """Extract the TODOs from all existing files, applying the batching request"""
    if os.path.exists("/MBI/gencodes"):  # Docker run
        filenames = glob.glob("/MBI/gencodes/*.c")
    elif os.path.exists("gencodes/"):  # Gitlab-ci run
        filenames = glob.glob(f"{os.getcwd()}/gencodes/*.c")  # our code expects absolute paths
    elif os.path.exists("../../gencodes/"):  # Local runs
        filenames = glob.glob(f"{os.getcwd()}/../../gencodes/*.c")  # our code expects absolute paths
    else:
        subprocess.run("ls ../..", shell=True)
        raise Exception(f"Cannot find the input codes (cwd: {os.getcwd()}). Did you run the generators before running the tests?")
    # Choose the files that will be used by this runner, depending on the -b argument
    match = re.match('(\d+)/(\d+)', batch)
    if not match:
        print(f"The parameter to batch option ({batch}) is invalid. Must be something like 'N/M', with N and M numbers.")
    pos = int(match.group(1))
    runner_count = int(match.group(2))
    assert pos > 0
    assert pos <= runner_count
    batch = int(len(filenames) / runner_count)+1
    min_rank = batch*(pos-1)
    max_rank = (batch*pos)-1
    print(f'Handling files from #{min_rank} to #{max_rank}, out of {len(filenames)} in {os.getcwd()}')

    global todo
    for filename in filenames[min_rank:max_rank]:
        todo = todo + extract_todo(filename)

########################
# cmd_gencodes(): what to do when '-c generate' is used (Generating the codes)
########################


def cmd_gencodes():
    if os.path.exists("/MBI/scripts/CollOpGenerator.py"):  # Docker run
        print("Docker run")
        generators = glob.glob("/MBI/scripts/*Generator.py")
        dir = "/MBI/gencodes"
    elif os.path.exists("../../scripts/CollOpGenerator.py"):  # Local run, from logs dir
        print("Local run, from tools' logs dir")
        generators = glob.glob(f"{os.getcwd()}/../../scripts/*Generator.py")
        dir = "../../gencodes/"
    elif os.path.exists("scripts/CollOpGenerator.py"):  # Local run, from main dir
        print("Local run, from MBI main dir")
        generators = glob.glob(f"{os.getcwd()}/scripts/*Generator.py")
        dir = "gencodes/"
    else:
        raise Exception("Cannot find the codes' generators. Please report that bug.")
    subprocess.run(f"rm -rf {dir} ; mkdir {dir}", shell=True, check=True)
    here = os.getcwd()
    os.chdir(dir)
    print("Generate the codes: ", end='')
    for generator in generators:
        m = re.match("^.*?/([^/]*)Generator.py$", generator)
        if m:
            print(m.group(1), end=", ")
        else:
            print(generator, end=", ")
        subprocess.run(generator, check=True)
    print(f" (files generated in {os.getcwd()})")
    print("Test count: ", end='')
    sys.stdout.flush()
    subprocess.run("ls *.c|wc -l", shell=True, check=True)
    os.chdir(here)


########################
# cmd_run(): what to do when '-c run' is used (running the tests)
########################
def cmd_run(rootdir, toolname):
    # Go to the tools' logs directory on need
    rootdir = os.path.dirname(os.path.abspath(__file__))
    os.makedirs(f'{rootdir}/logs/{toolname}', exist_ok=True)
    os.chdir(f'{rootdir}/logs/{toolname}')

    # Basic verification
    tools[toolname].ensure_image()

    # Do the tool-specific setups
    tools[toolname].setup(rootdir)

    count = 1
    for test in todo:
        binary = re.sub('\.c', '', os.path.basename(test['filename']))

        print(f"\nTest '{binary}_{test['id']} (test #{count} out of {len(todo)})'", end=": ")
        count += 1
        sys.stdout.flush()

        p = mp.Process(target=tools[toolname].run, args=(test['cmd'], test['filename'], binary, test['id'], args.timeout))
        p.start()
        sys.stdout.flush()
        p.join(args.timeout+60)
        if p.is_alive():
            print("HARD TIMEOUT! The child process failed to timeout by itself. Sorry for the output.")
            p.terminate()

    tools[toolname].teardown()

########################
# cmd_stats(): what to do when '-c stats' is used (extract the statistics of this tool)
########################
def categorize(toolname, test_ID, expected):
    outcome = tools[toolname].parse(test_ID)

    if not os.path.exists(f'{test_ID}.elapsed') and not os.path.exists(f'logs/{toolname}/{test_ID}.elapsed'):
        if outcome == 'failure':
            elapsed = 0
        else:
            raise Exception(f"Invalid test result: {test_ID}.txt exists but not {test_ID}.elapsed")
    else:
        with open(f'{test_ID}.elapsed', 'r') as infile:
            elapsed = infile.read()

    # Properly categorize this run
    if outcome == 'timeout':
        res_category = 'timeout'
        if elapsed is None:
            diagnostic = f'{test_ID} (hard timeout)'
        else:
            diagnostic = f'{test_ID} (elapsed: {elapsed} sec)'
    elif outcome == 'failure':
        res_category = 'failure'
        diagnostic = f'{test_ID}'
    elif outcome == 'UNIMPLEMENTED':
        res_category = 'unimplemented'
        diagnostic = f'{test_ID}'
    elif expected == 'OK':
        if outcome == 'OK':
            res_category = 'TRUE_NEG'
            diagnostic = f'{test_ID}'
        else:
            res_category = 'FALSE_POS'
            diagnostic = f'{test_ID} (expected {expected} but returned {outcome})'
    elif expected == 'ERROR':
        if outcome == 'OK':
            res_category = 'FALSE_NEG'
            diagnostic = f'{test_ID} (expected {expected} but returned {outcome})'
        else:
            res_category = 'TRUE_POS'
            diagnostic =  f'{test_ID}'
    else:
        raise Exception(f"Unexpected expectation: {expected} (must be OK or ERROR)")

    return (res_category, elapsed, diagnostic)

def cmd_stats(rootdir, toolnames=[]):
    for toolname in toolnames:
        if not toolname in tools:
            raise Exception(f"Tool {toolname} does not seem to be a valid name.")

        # To compute statistics on the performance of this tool
        results= {'failure':[], 'timeout':[], 'unimplemented':[], 'TRUE_NEG':[], 'TRUE_POS':[], 'FALSE_NEG':[], 'FALSE_POS':[]}

        # To compute timing statistics
        total_elapsed = 0

        for test in todo:
            
            binary=re.sub('\.c', '', os.path.basename(test['filename']))
            test_ID = f"{binary}_{test['id']}"
            expected=test['expect']
            (res_category, elapsed, diagnostic) = categorize(toolname=toolname, test_ID=test_ID, expected=expected)

            results[res_category].append(diagnostic)
            if res_category != 'timeout' and elapsed is not None:
                total_elapsed += float(elapsed)

            print(f"Test '{test_ID}' result: {res_category}: {diagnostic}. Elapsed: {elapsed} sec")

            np = re.search(r"(?:-np) [0-9]+", test['cmd'])
            np = int(re.sub(r"-np ", "", np.group(0)))

            with open("./" + args.o, "a") as result_file:
                result_file.write(
                    f"{binary};{test['id']};{args.x};{args.timeout};{np};0;{expected};{res_category};{elapsed}\n")

        ########################
        # Statistics summary
        ########################

        TP = len(results['TRUE_POS'])
        TN = len(results['TRUE_NEG'])
        FP = len(results['FALSE_POS'])
        FN = len(results['FALSE_NEG'])
        nPort = len(results['unimplemented'])
        nFail = len(results['failure'])
        nTout = len(results['timeout'])
        passed = TP + TN
        total = passed + FP + FN + nTout + nPort + nFail

        print(f"XXXXXXXXX Final results")
        if FP > 0:
            print(f"XXX {FP} false positives:")
            for p in results['TRUE_POS']:
                print(f"  {p}")
        if FN > 0:
            print(f"XXX {FN} false negatives:")
            for p in results['TRUE_NEG']:
                print(f"  {p}")
        if nTout > 0:
            print(f"XXX {nTout} timeouts:")
            for p in results['timeout']:
                print(f"  {p}")
        if nPort > 0:
            print(f"XXX {nPort} portability issues:")
            for p in results['unimplemented']:
                print(f"  {p}")
        if nFail > 0:
            print(f"XXX {nFail} tool failures:")
            for p in results['failure']:
                print(f"  {p}")

        def percent(ratio):
            """Returns the ratio as a percentage, rounded to 2 digits only"""
            return int(ratio*10000)/100
        print(f"\nXXXX Summary for {args.x} XXXX  {passed} test{'' if passed == 1 else 's'} passed (out of {total})")
        try:
            print(f"Portability: {percent(1-nPort/total)}% ({nPort} tests failed)")
            print(
                f"Robustness: {percent(1-(nTout+nFail)/(total-nPort))}% ({nTout} timeouts and {nFail} failures)\n")

            print(f"Recall: {percent(TP/(TP+FN))}% (found {TP} errors out of {TP+FN})")
            print(f"Specificity: {percent(TN/(TN+FP))}% (recognized {TN} correct codes out of {TN+FP})")
            print(f"Precision: {percent(TP/(TP+FP))}% ({TP} diagnostic of error are correct out of {TP+FP})")
            print(f"Accuracy: {percent((TP+TN)/(TP+TN+FP+FN))}% ({TP+TN} correct diagnostics in total, out of {TP+TN+FP+FN} diagnostics)")
        except ZeroDivisionError:
            print("Got a ZeroDivisionError while computing the metrics. Are you using all tests?")
        print(f"\nTotal time of all tests (not counting the timeouts): {total_elapsed}")

########################
# Main script argument parsing
########################


parser = argparse.ArgumentParser(
    description='This runner intends to provide a bridge from a MPI compiler/executor + a test written with MPI bugs collection header and the actual result compared to the expected.')

parser.add_argument('-c', metavar='cmd', default='all',
                    help="The command you want to execute. By default, 'all', runs all commands in sequence. Other choices:\n"
                    "  run: run the tests on all codes.\n"
                    "  stats: produce the statistics, using the cached values from a previous 'run'.\n")

parser.add_argument('-x', metavar='tool', default='mpirun',
                    help='the tool you want at execution: one among [aislinn, civl, isp, mpisv, must, simgrid, parcoach]')

parser.add_argument('-t', '--timeout', metavar='int', default=300, type=int,
                    help='timeout value at execution time, given in seconds (default: 300)')

parser.add_argument('-o', metavar='output.csv', default='out.csv', type=str,
                    help='name of the csv file in which results will be written')

parser.add_argument('-b', metavar='batch', default='1/1',
                    help="Limits the test executions to the batch #N out of M batches (Syntax: 'N/M'). To get 3 runners, use 1/3 2/3 3/3")

args = parser.parse_args()
rootdir = os.path.dirname(os.path.abspath(__file__))

# Parameter checking: Did we get a valid tool to use?
if args.c != 'generate' and args.c != 'stats':
    if args.x == 'mpirun':
        raise Exception("No tool was provided, please retry with -x parameter. (see -h for further information on usage)")
    elif args.x in ['aislinn', 'civl', 'isp', 'must', 'mpisv', 'simgrid', 'parcoach']:
        pass
    else:
        raise Exception(f"The tool parameter you provided ({args.x}) is either incorect or not yet implemented.")

if args.o == 'out.csv':
    args.o = f'bench_{args.x}.csv'

if args.c == 'all':
    extract_all_todo(args.b)
    cmd_run(rootdir=rootdir, toolname=args.x)
    cmd_stats(rootdir, toolnames=[args.x])
elif args.c == 'generate':
    cmd_gencodes()
elif args.c == 'run':
    extract_all_todo(args.b)
    cmd_run(rootdir=rootdir, toolname=args.x)
elif args.c == 'stats':
    extract_all_todo(args.b)
    cmd_stats(rootdir, toolnames=['aislinn', 'civl', 'isp', 'simgrid', 'mpisv', 'must', 'parcoach'])
else:
    print(f"Invalid command '{args.c}'. Please choose one of 'all', 'run', 'stats'")
    sys.exit(1)
