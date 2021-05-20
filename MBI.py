#! /MBI/scripts/ensure_python3

import shutil
import os
import signal
import sys
import stat
import re
import argparse
import time
import glob
import multiprocessing as mp

# Add our lib directory to the PYTHONPATH, and load our utilitary libraries
sys.path.append(f'{os.path.dirname(os.path.abspath(__file__))}/scripts')
import aislinn, civl, isp, mpisv, must, simgrid, parcoach

# Some scripts may fail if error messages get translated
os.environ["LC_ALL"] = "C"

tool = None # The correct tool will be chosen when parsing the parameters, at the bottom of this file

########################
# Extract the TODOs from the codes
########################

possible_details = [
    # scope limited to one call
    'InvalidCommunicator','InvalidDatatype','InvalidRoot','InvalidWindow', 'InvalidOperator', 'ActualDatatype',
    # scope: Process-wide
    'OutOfInitFini', 'CommunicatorLeak', 'DatatypeLeak', 'GroupLeak', 'OperatorLeak', 'TypeLeak', 'MissingStart', 'MissingWait', 
    'RequestLeak', 'LocalConcurrency',
    # scope: communicator
    'MessageRace', 'CallMatching', 'CommunicatorMatching', 'DatatypeMatching', 'InvalidSrcDest', 'RootMatching', 'OperatorMatching',
    # larger scope
    'BufferingHazard']
    # BufferLength/BufferOverlap
    # RMA concurrency errors (local and distributed)

todo = []

def extract_todo(filename):
    """
    Reads the header of the filename, and extract a list of todo item, each of them being a (cmd, expect, test_num) tupple.
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
                        raise Exception(f"\n{filename}:{line_num}: MBI parse error: Detailled outcome {detail} is not one of the allowed ones.")
                test = {'filename': filename, 'id': test_num, 'cmd': cmd, 'expect':expect, 'detail':detail }
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

########################
# cmd_run(): what to do when -c run is used (running the tests)
########################
def cmd_run():
    # Basic verification
    tool.ensure_image()

    # Do the tool-specific setups
    tool.setup(rootdir)

    for test in todo:
        binary = re.sub('\.c', '', os.path.basename(test['filename']))

        print(f"Test '{binary}_{test['id']}'", end=": ")
        sys.stdout.flush()

        p = mp.Process(target=tool.run, args=(test['cmd'], test['filename'], binary, test['id'], args.timeout))
        p.start()
        sys.stdout.flush()
        p.join(args.timeout+60)
        if p.is_alive():
            print("HARD TIMEOUT! The child process failed to timeout by itself. Sorry for the output.")
            p.terminate()

    tool.teardown()

########################
# cmd_stats(): what to do when '-c stats' is used (extract the statistics)
########################
def cmd_stats():
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

    for test in todo:
        binary = re.sub('\.c', '', os.path.basename(test['filename']))
        test_ID = f"{binary}_{test['id']}"
        outcome = tool.parse(test_ID)
        expected = test['expect']

        if os.path.exists(f'{test_ID}.elapsed'):
            with open(f'{test_ID}.elapsed', 'r') as infile:
                elapsed = infile.read()

        # Stats on the codes, even if the tool fails
        if expected == 'OK':
            code_correct += 1
        else:
            code_incorrect += 1

        # Properly categorize this run
        if outcome == 'timeout':
            res_category = 'timeout'
            if elapsed is None:
                timeout.append(f'{test_ID} (hard timeout)')
            else:
                timeout.append(f'{test_ID} (elapsed: {elapsed} sec)')
        elif outcome == 'failure':
            res_category = 'failure'
            failure.append(f'{test_ID}')
        elif outcome == 'UNIMPLEMENTED':
            res_category = 'portability issue'
            unimplemented.append(f'{test_ID}')
        elif expected == 'OK':
            if outcome == 'OK':
                res_category = 'TRUE_NEG'
                true_neg.append(f'{test_ID}')
            else:
                res_category = 'FALSE_POS'
                false_pos.append(f'{test_ID} (expected {expected} but returned {outcome})')
        elif expected == 'ERROR':
            if outcome == 'OK':
                res_category = 'FALSE_NEG'
                false_neg.append(f'{test_ID} (expected {expected} but returned {outcome})')
            else:
                res_category = 'TRUE_POS'
                true_pos.append(f'{test_ID}')
        else: 
            raise Exception(f"Unexpected expectation: {expected} (must be OK or ERROR)")

        print(f"Test '{test_ID}' result: {res_category}: {args.x} returned {outcome} while {expected} was expected. Elapsed: {elapsed} sec")

        if res_category != 'timeout' and elapsed is not None:
            total_elapsed += float(elapsed)

        np = re.search(r"(?:-np) [0-9]+", test['cmd'])
        np = int(re.sub(r"-np ", "", np.group(0)))

        zero_buff = re.search(r"\$zero_buffer", test['cmd'])
        infty_buff = re.search(r"\$infty_buffer", test['cmd'])
        if zero_buff != None:
            buff = '0'
        elif infty_buff != None:
            buff = 'inf'
        else:
            buff = 'NA'

        with open("./" + args.o, "a") as result_file:
            result_file.write(
                f"{binary};{test['id']};{args.x};{args.timeout};{np};{buff};{expected};{outcome};{elapsed}\n")

    ########################
    # Statistics summary
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
    try:
        print(f"Portability: {percent(1-len(unimplemented)/total)}% ({len(unimplemented)} tests failed)")
        print(f"Robustness: {percent(1-(len(timeout)+len(failure))/(total-len(unimplemented)))}% ({len(timeout)} timeouts and {len(failure)} failures)\n")

        print(f"Recall: {percent(TP/(TP+FN))}% (found {TP} errors out of {TP+FN})")
        print(f"Specificity: {percent(TN/(TN+FP))}% (recognized {TN} correct codes out of {TN+FP})")
        print(f"Precision: {percent(TP/(TP+FP))}% ({TP} diagnostic of error are correct out of {TP+FP})")
        print(f"Accuracy: {percent((TP+TN)/(TP+TN+FP+FN))}% ({TP+TN} correct diagnostics in total, out of {TP+TN+FP+FN} diagnostics)")
    except ZeroDivisionError:
        print("Got a ZeroDivisionError while computing the metrics. Are you using all tests?")
    print(f"\nTotal time of all tests (not counting the timeouts): {total_elapsed}")
    print(f"\nMBI stats: {code_correct} correct codes; {code_incorrect} incorrect codes.")

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

if args.x == 'mpirun':
    raise Exception("No tool was provided, please retry with -x parameter. (see -h for further information on usage)")
elif args.x in ['aislinn', 'civl', 'isp', 'must', 'mpisv', 'simgrid', 'parcoach']:
    exec(f'tool = {args.x}.Tool()')
else:
    raise Exception(f"The tool parameter you provided ({args.x}) is either incorect or not yet implemented.")
if args.o == 'out.csv':
    args.o = f'bench_{args.x}.csv'

# Choose the files that will be used by this runner, depending on the -b argument
match = re.match('(\d+)/(\d+)', args.b)
if not match:
    print(f"The parameter to batch option ({args.b}) is invalid. Must be something like 'N/M', with N and M numbers.")
pos = int(match.group(1))
runner_count = int(match.group(2))
assert pos > 0
assert pos <= runner_count
filenames = glob.glob("/MBI/gencodes/*.c")
batch = int(len(filenames) / runner_count)+1
min_rank = batch*(pos-1)
max_rank = (batch*pos)-1
print(f'Handling files from #{min_rank} to #{max_rank}, out of {len(filenames)} in {os.getcwd()}')

for filename in filenames[min_rank:max_rank]:
    todo = todo + extract_todo(filename)

rootdir=os.path.dirname(os.path.abspath(__file__))
os.makedirs(f'{rootdir}/logs/{args.x}', exist_ok=True)
os.chdir(   f'{rootdir}/logs/{args.x}')

if args.c == 'all':
    cmd_run()
    cmd_stats()
elif args.c == 'run':
    cmd_run()
elif args.c == 'stats':
    cmd_stats()
else:
    print(f"Invalid command '{args.c}'. Please choose one of 'all', 'run', 'stats'")
    sys.exit(1)
