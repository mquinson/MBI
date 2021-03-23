#! /usr/bin/python3

import shutil, os, sys, stat, subprocess, re, argparse, queue
import multiprocessing as mp
import runner_must, runner_civl, runner_simgrid, runner_parcoach, runner_isp, runner_mpisv, runner_aislinn

# Some scripts may fail if error messages get translated
os.environ["LC_ALL"] = "C"

########################
## Argument Parsing
########################

parser = argparse.ArgumentParser(description='This runner intends to provide a bridge from a MPI compiler/executor + a test written with MPI bugs collection header and the actual result compared to the expected.')

parser.add_argument('filenames', metavar='example.c', nargs="+", help='a list of MPI c sources.')

parser.add_argument('-x', metavar='tool', default='mpirun', help='the tool you want at execution : one among [isp, must, mustdist, mpisv, aislinn, civl, simgrid, parcoach]')

parser.add_argument('-t', '--timeout', metavar='int', default=300, type=int, help='timeout value at execution time, given in seconds')

parser.add_argument('-o', metavar='output.csv', default='out.csv', type=str, help='name of the csv file in which results will be written')

parser.add_argument('--job', metavar='int', default='NA', type=str, help='Gitlab job-id, in order to fetch execution artifacts. If not run as a Gitlab job, do not consider.')

args = parser.parse_args()

########################
## Usefull globals
########################

todo=[]

ok_noerror=[]
ok_deadlock=[]
ok_numstab=[]
ok_segfault=[]
ok_mpierr=[]
ok_resleak=[]
ok_livelock=[]
ok_compliance=[]
ok_datarace=[]
failed=[]
notimplemented=[]

########################
## Going through files
########################

def extract_todo(filename):
    """
    Reads the header of the filename, and extract a list of todo item, each of them being a (cmd, expect, test_count) tupple.
    The test_count is useful to build a log file containing both the binary and the test_count, when there is more than one test in the same binary.
    """
    res = []
    test_count = 0
    with open(filename, "r") as input:
        state = 0 # 0: before header; 1: in header; 2; after header
        line_num=1
        for line in input:
            if re.match(".*End of MPI bugs collection header.*", line):
                if state == 1:
                    state = 2
                else:
                    print("\nUnexpected end of header at line {}: \n{}".format(line_num,line))
                    sys.exit(1)
            elif re.match(".*MPI bugs collection header.*", line):
                if state == 0:
                    state = 1
                else:
                    print("\nBug header appears more than once at line {}: \n{}".format(line_num,line))
                    sys.exit(1)
            if state == 1 and re.match(".*Test:.*", line):
                m = re.match('.*Test: (.*)', line)
                cmd = m.group(1)
                nextline = next(input)
                m = re.match('.*Expect: (\w+)\|?(\w+)?', nextline)
                if not m:
                    print("\n{}:{}: 'Test' line not followed by a proper 'Expect' line:\n{}{}".format(filename,line_num, line, nextline))
                expect = [expects for expects in m.groups() if expects!=None]
                if not expect[0] in ["noerror", "deadlock",  "numstab", "segfault", "mpierr", "resleak", "livelock", "compliance", "datarace"]:
                    print("\n{}:{}: expectation >>{}<< not understood."
                          .format(filename, line_num, expect))
                    continue
                res.append((cmd, expect, test_count))
                test_count+=1
                line_num+=1

    if state == 0:
        print("\nBug header not found in file '{}'.".format(filename))
        sys.exit(1)
    if state == 1:
        print("\nNo end of bug header found in file '{}'.".format(filename))
        sys.exit(1)

    return res

def return_to_queue(queue, func, args):
    queue.put(func(*args))

for filename in args.filenames:
    if filename == "template.c":
        continue
    
    binary = re.sub('\.c','',os.path.basename(filename))
    sys.stdout.flush()
        
    todo = todo + extract_todo(filename)
                
    if len(todo) == 0:
        print(" no test found. Please fix it.")
        notimplemented.append(filename)
        continue



########################
## Running the tests
########################
            
    for cmd, outcome, test_count in todo:
        print("Test {}'{}'".format("" if test_count == 0 else "{} ".format(test_count+1), binary), end=":")
        sys.stdout.flush()
       
        if args.x != 'mustdist' and args.x != 'simgrid':
            cmd = re.sub('^', "echo 'Executing https://gitlab.com/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/-/tree/master/Benchmarks/microbenchs/{}.c';echo;".format(binary), cmd)

        if args.x == 'mpirun':
            print("No tool was provided, please retry with -x parameter. (see -h for further information on usage)")
            sys.exit(1)
#        elif args.x == 'must':
#            ans = runner_must.mustrun(cmd, args.timeout, filename, binary, test_count)
        elif args.x == 'mustdist' or args.x == 'simgrid':
            q = mp.Queue()
            if args.x == 'mustdist':
                func = runner_must.mustrun
            elif args.x == 'simgrid':
                func = runner_simgrid.simgridrun
            p = mp.Process(target=return_to_queue, args=(q, func, (cmd, filename, binary, test_count)))
            p.start()
            print("Wait up to {} seconds".format(args.timeout))
            p.join(args.timeout)
            p.terminate()
            try:
                ans = q.get(block=True)
            except queue.Empty:
                ans = 'RSF'
        elif args.x == 'civl':
            ans = runner_civl.civlrun(cmd, args.timeout, filename, binary, test_count)
#        elif args.x == 'simgrid':
#            ans = (cmd, args.timeout, filename, binary, test_count)
        elif args.x == 'parcoach':
            ans = runner_parcoach.parcoachrun(cmd, args.timeout, filename, binary, test_count)
        elif args.x == 'isp':
            ans = runner_isp.isprun(cmd, args.timeout, filename, binary, test_count)
        elif args.x == 'mpisv':
            ans = runner_mpisv.mpisvrun(cmd, args.timeout, filename, binary, test_count)
        elif args.x == 'aislinn':
            ans = runner_aislinn.aislinnrun(cmd, args.timeout, filename, binary, test_count)
        else:
            print("The tool parameter you provided ({}) is either incorect or not yet implemented.".format(args.x))
            sys.exit(1)

        print("Tool output (30 last lines only; outcome: {}; expected: {})".format(ans, outcome))
        with open('{}_{}.txt'.format(binary, test_count), 'rb') as input:
            for line in (input.readlines() [-30:]):
                print ("| {}".format(line))
            
        if ans not in outcome:    
            failed.append("{} (expected {} but returned {})".format(binary, outcome, ans))
        elif 'noerror' in outcome:
            ok_noerror.append(binary)    
        elif 'deadlock' in outcome:
            ok_deadlock.append(binary)
        elif 'numstab' in outcome:
            ok_numstab.append(binary)
        elif 'segfault' in outcome:
            ok_segfault.append(binary)
        elif 'mpierr' in outcome:
            ok_mpierr.append(binary)
        elif 'resleak' in outcome:
            ok_resleak.append(binary)
        elif 'livelock' in outcome:
            ok_livelock.append(binary)
        elif 'compliance' in outcome:
            ok_compliance.append(binary)
        elif 'datarace' in outcome:
            ok_datarace.append(binary)
        
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
            result_file.write("{};{};{};{};{};{};{};{};{}\n".format(
                binary,
                test_count,
                args.x,
                args.timeout,
                np,
                buff,
                outcome,
                ans,
                args.job))

########################
## Termination
########################
        
passed_count = 0    
passed_count += len(ok_noerror)
passed_count += len(ok_deadlock)
passed_count += len(ok_numstab)
passed_count += len(ok_segfault)
passed_count += len(ok_mpierr)
passed_count += len(ok_resleak)
passed_count += len(ok_livelock)
passed_count += len(ok_compliance)
passed_count += len(ok_datarace)

print("XXXXXXXXX\nResult: {} test{} out of {} passed."
      .format(passed_count, '' if passed_count==1 else 's', passed_count+len(failed)))
print("{} failed tests:".format(len(failed)))
for p in failed:
    print("  {}".format(p))
for n in notimplemented:
    print(n)
