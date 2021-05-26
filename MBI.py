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

# Associate all possible detailed outcome to a given error scope. Scopes must be sorted alphabetically.
possible_details = {
    # scope limited to one call
    'InvalidCommunicator':'ACall', 'InvalidDatatype':'ACall', 'InvalidRoot':'ACall', 'InvalidTag':'ACall', 'InvalidWindow':'ACall', 'InvalidOperator':'ACall', 'InvalidOtherArg':'ACall', 'ActualDatatype':'ACall',
    # scope: Process-wide
    'OutOfInitFini':'BProcess', 'CommunicatorLeak':'BProcess', 'DatatypeLeak':'BProcess', 'GroupLeak':'BProcess', 'OperatorLeak':'BProcess', 'TypeLeak':'BProcess', 'MissingStart':'BProcess', 'MissingWait':'BProcess',
    'RequestLeak':'BProcess', 'LocalConcurrency':'BProcess',
    # scope: communicator
    'MessageRace':'CComm', 'CallMatching':'CComm', 'CommunicatorMatching':'CComm', 'DatatypeMatching':'CComm', 'InvalidSrcDest':'CComm', 'OperatorMatching':'CComm', 'OtherArgMatching':'CComm', 'RootMatching':'CComm', 'TagMatching':'CComm',
    # larger scope
    'BufferingHazard':'DSystem',
    'OK':'EOK'}

displayed_name = {
    'ACall':'Error scope: single call',
    'BProcess':'Error scope: single process',
    'CComm':'Error scope: communicator',
    'DSystem':'Error scope: system-wide',
    'EOK':'Correct codes',

    'aislinn':'Aislinn','civl':'CIVL', 'isp':'ISP', 'simgrid':'SimGrid', 'mpisv':'MPI-SV', 'must':'MUST', 'parcoach':'PARCOACH'
}

# BufferLength/BufferOverlap
# RMA concurrency errors (local and distributed)

########################
# Extract the TODOs from the codes
########################
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
                detail = 'OK'
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
def cmd_run(rootdir, toolname, logdir=""):
    # Go to the tools' logs directory
    here=os.getcwd()
    rootdir = os.path.dirname(os.path.abspath(__file__))
    if logdir == "":
        logdir = f'{rootdir}/logs/{toolname}' 
    print(f"Enter logdir: {logdir}")
    os.makedirs(logdir, exist_ok=True)
    os.chdir(logdir)

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
    print(f"Leave logdir, back to {here}")
    os.chdir(here)

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
        with open(f'{test_ID}.elapsed' if os.path.exists(f'{test_ID}.elapsed') else f'logs/{toolname}/{test_ID}.elapsed', 'r') as infile:
            elapsed = infile.read()

    # Properly categorize this run
    if outcome == 'timeout':
        res_category = 'timeout'
        if elapsed is None:
            diagnostic = f'hard timeout'
        else:
            diagnostic = f'timeout after {elapsed} sec'
    elif outcome == 'failure':
        res_category = 'failure'
        diagnostic = f'tool error, or test not run'
    elif outcome == 'UNIMPLEMENTED':
        res_category = 'unimplemented'
        diagnostic = f'portability issue'
    elif expected == 'OK':
        if outcome == 'OK':
            res_category = 'TRUE_NEG'
            diagnostic = f'correctly reported no error'
        else:
            res_category = 'FALSE_POS'
            diagnostic = f'reported an error in a correct code'
    elif expected == 'ERROR':
        if outcome == 'OK':
            res_category = 'FALSE_NEG'
            diagnostic = f'failed to detect an error'
        else:
            res_category = 'TRUE_POS'
            diagnostic =  f'correctly detected an error'
    else:
        raise Exception(f"Unexpected expectation: {expected} (must be OK or ERROR)")

    return (res_category, elapsed, diagnostic)
def percent(ratio):
    """Returns the ratio as a percentage, rounded to 2 digits only"""
    return int(ratio*10000)/100

def cmd_stats(rootdir, toolnames=[]):
    here = os.getcwd()
    os.chdir(rootdir)
    results = {}
    total_elapsed = {}
    used_toolnames = []
    for toolname in toolnames:
        if not toolname in tools:
            raise Exception(f"Tool {toolname} does not seem to be a valid name.")

        if os.path.exists(f'logs/{toolname}'):
            used_toolnames.append(toolname)
            # To compute statistics on the performance of this tool
            results[toolname]= {'failure':[], 'timeout':[], 'unimplemented':[], 'TRUE_NEG':[], 'TRUE_POS':[], 'FALSE_NEG':[], 'FALSE_POS':[]}

            # To compute timing statistics
            total_elapsed[toolname] = 0

    ########################
    # Analyse each test, grouped by expectation, and all tools for a given test
    ########################
    with open(f"{rootdir}/index.html", "w") as outHTML:
      outHTML.write("""
<html><head><title>MBI results</title></head>
<script>
iframe {
  resize: both;
  overflow: auto;
}
</script>
<body>
<iframe width="100%" height="35%" src="summary.html"></iframe>
<iframe width="100%" height="65%" name="MBI_details"></iframe>
</body></html>
""")

    with open(f"{rootdir}/summary.html", "w") as outHTML:
      outHTML.write(f"<html><head><title>MBI outcomes for all tests</title></head>\n")
      outHTML.write("""
<style>
.tooltip {
  position: relative;
  display: inline-block;
  border-bottom: 1px dotted black; /* If you want dots under the hoverable text */
}

.tooltip .tooltiptext {
  visibility: hidden;
  width: 120px;
  background-color: #555;
  color: #fff;
  text-align: center;
  border-radius: 6px;
  padding: 5px 0;
  position: absolute;
  z-index: 1;
  bottom: 125%;
  left: 50%;
  margin-left: -60px;
  opacity: 0;
  transition: opacity 0.3s;
}

.tooltip .tooltiptext::after {
  content: "";
  position: absolute;
  top: 100%;
  left: 50%;
  margin-left: -5px;
  border-width: 5px;
  border-style: solid;
  border-color: #555 transparent transparent transparent;
}

.tooltip:hover .tooltiptext {
  visibility: visible;
  opacity: 1;
}
</style>
<body>
""")

      # Generate the table of contents
      previous_scope=''
      previous_detail=''  # To open a new section for each possible detailed outcome
      outHTML.write("<h2>Table of contents</h2>\n<ul>\n")
      for test in sorted(todo, key=lambda t: f"{possible_details[t['detail']]}|{t['detail']}"):
        if previous_scope != possible_details[test['detail']]:
            if previous_scope != '': # Close the previous item, if we are not generating the first one
                outHTML.write(f"  </ul>\n")
                outHTML.write(f" </li>\n")
            previous_scope = possible_details[test['detail']]
            previous_detail = '' # This is a new section
            outHTML.write(f" <li><a href='#{possible_details[test['detail']]}'>{displayed_name[ possible_details[test['detail']]]}</a>\n  <ul>\n")

        if previous_detail != f"{possible_details[test['detail']]}|{test['detail']}" and test['detail'] != 'OK':
            previous_detail = f"{possible_details[test['detail']]}|{test['detail']}"
            outHTML.write(f"   <li>Error: <a href='#{test['detail']}'>{test['detail']}</a></li>\n")
      outHTML.write("  </ul>\n <li><a href='#metrics'>Summary metrics</a></li></ul>\n")

      # Generate the actual content
      previous_scope=''
      previous_detail=''  # To open a new section for each possible detailed outcome
      testcount=0 # To repeat the table header every 25 lines
      for test in sorted(todo, key=lambda t: f"{possible_details[t['detail']]}|{t['detail']}"):
        testcount += 1
        if previous_scope != possible_details[test['detail']]:
            if previous_scope != '': # Close the previous table, if we are not generating the first one
                outHTML.write(f"</table>\n")
            previous_scope = possible_details[test['detail']]
            previous_detail = '' # This is a new section
            outHTML.write(f"  <a name='{possible_details[test['detail']]}'/><h2>{displayed_name[ possible_details[test['detail']]]}</h2>\n")

        if previous_detail != f"{possible_details[test['detail']]}|{test['detail']}" or testcount == 25:
            if testcount != 25: # Write the expected outcome only once, not every 25 tests
                if previous_detail != '': # Close the previous table, if we are not generating the first one of this scope
                    outHTML.write(f"</table>\n")
                previous_detail = f"{possible_details[test['detail']]}|{test['detail']}"
                if test['detail'] != 'OK':
                    outHTML.write(f"  <a name='{test['detail']}'/><h3>Expected outcome: {test['detail']}</h3>\n")
                outHTML.write( '  <table border=1>\n')
            testcount=0
            outHTML.write("   <tr><td>Test</td>")
            for toolname in used_toolnames:
                outHTML.write(f"<td>&nbsp;{displayed_name[toolname]}&nbsp;</td>")
            outHTML.write(f"</tr>\n")
        outHTML.write(f"     <tr>")

        binary=re.sub('\.c', '', os.path.basename(test['filename']))
        test_ID = f"{binary}_{test['id']}"
        expected=test['expect']

        outHTML.write(f"<td><a href='gencodes/{binary}.c'>{binary}</a></td>")

        for toolname in used_toolnames:
            (res_category, elapsed, diagnostic) = categorize(toolname=toolname, test_ID=test_ID, expected=expected)

            results[toolname][res_category].append(f"{test_ID} expected {test['detail']}, outcome: {diagnostic}")
            outHTML.write(f"<td align='center'><a href='logs/{toolname}/{test_ID}.txt' target='MBI_details'><img title='{diagnostic}' src='img/{res_category}.svg' width='24' /></a>")
            extra=None
            if os.path.exists(f'logs/{toolname}/{test_ID}.html'):
                extra=f'logs/{toolname}/{test_ID}.html'
            if os.path.exists(f'logs/{toolname}/{test_ID}-klee-out'): # MPI-SV 
                extra=f'logs/{toolname}/{test_ID}-klee-out'
            if extra is not None:
                outHTML.write(f"&nbsp;<a href='{extra}' target='MBI_details'><img title='more info' src='img/html.svg' height='24' /></a>")
            outHTML.write("</td>")

            if res_category != 'timeout' and elapsed is not None:
                total_elapsed[toolname] += float(elapsed)

            if len(used_toolnames) == 1:
                print(f"Test '{test_ID}' result: {res_category}: {diagnostic}. Elapsed: {elapsed} sec")

            np = re.search(r"(?:-np) [0-9]+", test['cmd'])
            np = int(re.sub(r"-np ", "", np.group(0)))

            if len(used_toolnames) == 1:
                with open(f"./logs/{toolname}/bench_{toolname}.csv", "a") as result_file:
                    result_file.write(
                        f"{binary};{test['id']};{args.x};{args.timeout};{np};0;{expected};{res_category};{elapsed}\n")
        outHTML.write(f"</tr>\n")
      outHTML.write(f"</table>\n")

      # Display summary metrics for each tool
      def tool_stats(toolname):
          return (len(results[toolname]['TRUE_POS']), len(results[toolname]['TRUE_NEG']),len(results[toolname]['FALSE_POS']),len(results[toolname]['FALSE_NEG']),len(results[toolname]['unimplemented']),len(results[toolname]['failure']),len(results[toolname]['timeout']))

      outHTML.write("\n<a name='metrics'/><h2>Metrics</h2><table border=1>\n<tr><td/>\n")
      for toolname in used_toolnames:
        outHTML.write(f"<td>{displayed_name[toolname]}</td>")

      outHTML.write("</tr>\n<tr><td>Portability</td>")
      for toolname in used_toolnames:
        (TP, TN, FP, FN, nPort, nFail, nTout) = tool_stats(toolname)
        total = TP + TN + FP + FN + nTout + nPort + nFail
        outHTML.write(f"<td><div class='tooltip'>{percent(1-nPort/total)}% <span class='tooltiptext'>{nPort} tests failed</span></div></td>")

      outHTML.write("</tr>\n<tr><td>Robustness</td>")
      for toolname in used_toolnames:
        (TP, TN, FP, FN, nPort, nFail, nTout) = tool_stats(toolname)
        total = TP + TN + FP + FN + nTout + nPort + nFail
        outHTML.write(f"<td><div class='tooltip'>{percent(1-(nTout+nFail)/(total-nPort))}% <span class='tooltiptext'>{nTout} timeouts, {nFail} failures</span></div></td>")

      outHTML.write("</tr>\n<tr><td>Recall</td>")
      for toolname in used_toolnames:
        (TP, TN, FP, FN, nPort, nFail, nTout) = tool_stats(toolname)
        outHTML.write(f"<td><div class='tooltip'>{percent(TP/(TP+FN))}% <span class='tooltiptext'>found {TP} errors out of {TP+FN}</span></div></td>")
      outHTML.write("</tr>\n<tr><td>Specificity</td>")
      for toolname in used_toolnames:
        (TP, TN, FP, FN, nPort, nFail, nTout) = tool_stats(toolname)
        outHTML.write(f"<td><div class='tooltip'>{percent(TN/(TN+FP))}%  <span class='tooltiptext'>recognized {TN} correct codes out of {TN+FP}</span></div></td>")
      outHTML.write("</tr>\n<tr><td>Precision</td>")
      for toolname in used_toolnames:
        (TP, TN, FP, FN, nPort, nFail, nTout) = tool_stats(toolname)
        outHTML.write(f"<td><div class='tooltip'>{percent(TP/(TP+FP))}% <span class='tooltiptext'>{TP} diagnostics of error are correct out of {TP+FP})</span></div></td>")
      outHTML.write("</tr>\n<tr><td>Accuracy</td>")
      for toolname in used_toolnames:
        (TP, TN, FP, FN, nPort, nFail, nTout) = tool_stats(toolname)
        outHTML.write(f"<td><div class='tooltip'>{percent((TP+TN)/(TP+TN+FP+FN))}% <span class='tooltiptext'>{TP+TN} correct diagnostics in total, out of {TP+TN+FP+FN} diagnostics</span></div></td>")
      outHTML.write("</tr></table>")
      outHTML.write("<p>Hover over the values for details. Portability issues, timeout and failures are not considered when computing the other metrics, thus differences in the total amount of tests.</p>")

      outHTML.write(f"</body></html>\n")

    ########################
    # Per tool statistics summary
    ########################
    for toolname in used_toolnames:
        TP = len(results[toolname]['TRUE_POS'])
        TN = len(results[toolname]['TRUE_NEG'])
        FP = len(results[toolname]['FALSE_POS'])
        FN = len(results[toolname]['FALSE_NEG'])
        nPort = len(results[toolname]['unimplemented'])
        nFail = len(results[toolname]['failure'])
        nTout = len(results[toolname]['timeout'])
        passed = TP + TN
        total = passed + FP + FN + nTout + nPort + nFail

        print(f"XXXXXXXXX Final results for {toolname}")
        if FP > 0:
            print(f"XXX {FP} false positives")
            if len(used_toolnames) == 1:
                for p in results[toolname]['FALSE_POS']:
                    print(f"  {p}")
        if FN > 0:
            print(f"XXX {FN} false negatives")
            if len(used_toolnames) == 1:
                for p in results[toolname]['FALSE_NEG']:
                    print(f"  {p}")
        if nTout > 0:
            print(f"XXX {nTout} timeouts")
            if len(used_toolnames) == 1:
                for p in results[toolname]['timeout']:
                    print(f"  {p}")
        if nPort > 0:
            print(f"XXX {nPort} portability issues")
            if len(used_toolnames) == 1:
                for p in results[toolname]['unimplemented']:
                    print(f"  {p}")
        if nFail > 0:
            print(f"XXX {nFail} tool failures")
            if len(used_toolnames) == 1:
                for p in results[toolname]['failure']:
                    print(f"  {p}")

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
            print(f"Got a ZeroDivisionError while computing the metrics for tool {toolname}. Are you using all tests?")
        print(f"\nTotal time of all tests (not counting the timeouts): {total_elapsed}")

    os.chdir(here)

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

parser.add_argument('-l', '--logdir', default="", 
                    help='Directory in which to store the logs with the run command (ignored when generating the stats -- default: logs/{toolname})')

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

if args.c == 'all':
    extract_all_todo(args.b)
    cmd_run(rootdir=rootdir, toolname=args.x, logdir=args.l)
    cmd_stats(rootdir, toolnames=[args.x])
elif args.c == 'generate':
    cmd_gencodes()
elif args.c == 'run':
    extract_all_todo(args.b)
    cmd_run(rootdir=rootdir, toolname=args.x, logdir=args.l)
elif args.c == 'stats':
    extract_all_todo(args.b)
    cmd_stats(rootdir, toolnames=['aislinn', 'civl', 'isp', 'simgrid', 'mpisv', 'must', 'parcoach'])
else:
    print(f"Invalid command '{args.c}'. Please choose one of 'all', 'run', 'stats'")
    sys.exit(1)
