#! /usr/bin/python3

import shutil, os, sys, stat, subprocess, re, argparse, queue, time, shlex
import multiprocessing as mp

# Some scripts may fail if error messages get translated
os.environ["LC_ALL"] = "C"

########################
## Argument Parsing
########################

parser = argparse.ArgumentParser(description='This runner intends to provide a bridge from a MPI compiler/executor + a test written with MPI bugs collection header and the actual result compared to the expected.')

parser.add_argument('filenames', metavar='example.c', nargs="+", help='a list of MPI c sources.')

parser.add_argument('-x', metavar='tool', default='mpirun', help='the tool you want at execution : one among [aislinn, civl, isp, must, simgrid, parcoach]')

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
       
        start_time = time.time()
        q = mp.Queue()
        
        if args.x == 'mpirun':
            print("No tool was provided, please retry with -x parameter. (see -h for further information on usage)")
            sys.exit(1)
            
        elif args.x == 'must':
            func = mustrun
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
            print("The tool parameter you provided ({}) is either incorect or not yet implemented.".format(args.x))
            sys.exit(1)
            
        p = mp.Process(target=return_to_queue, args=(q, func, (cmd, filename, binary, test_count)))
        p.start()
        print("Wait up to {} seconds".format(args.timeout))
        sys.stdout.flush()
        p.join(args.timeout)
        try:
            ans = q.get(block=False)
        except queue.Empty:
            if p.is_alive():
                print("Timeout!")
                p.terminate()
                ans = 'timeout'
            else:
                ans = 'RSF'
        

        curr_time = time.time()
        print("The tool returned {} (expected: {}; elapsed: {:f} sec)\n\n".format(ans, outcome, curr_time-start_time))
            
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



##########################
## Helper function to run tests
##########################

def run_cmd(buildcmd, execcmd, binary, read_line_lambda=None):
    output = "Compiling https://gitlab.com/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/-/tree/master/Benchmarks/microbenchs/{}.c\n\n".format(binary)
    output += "$ {}".format(buildcmd)

    compil = subprocess.run(buildcmd, shell=True, stderr=subprocess.STDOUT)
    if compil.stdout is not None:
        output += compil.stdout
    if compil.returncode != 0:
        output += "Compilation of {}.c raised an error (retcode: %d)".format(binary, compil.returncode)
        for line in (output.split('\n')):
            print ("| {}".format(line), file=sys.stderr)
        return 'CUN', compil.returncode, output

    output += "\n\nExecuting https://gitlab.com/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/-/tree/master/Benchmarks/microbenchs/{}.c\n\n$ {}\n".format(binary,execcmd)
    for line in (output.split('\n')):
        print ("| {}".format(line), file=sys.stderr)

    try:
        # We run the subprocess and parse its output line by line, so that we can kill it as soon as it detects a timeout
        process = subprocess.Popen(shlex.split(execcmd), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        while True:
            line = process.stdout.readline()
            if line:
                try:
                    line = line.decode('UTF-8') # From byte array to string
                except UnicodeDecodeError:
                    pass # The output seem to be binary. Fine, we can live with it.
                output = output + line
                print ("| {}".format(line), end='', file=sys.stderr)
                if read_line_lambda != None:
                    read_line_lambda(line, process)
            if process.poll() is not None:
                break
        rc = process.poll()
    except subprocess.TimeoutExpired:
        return 'timeout', output

    return None, rc, output

##########################
## Aislinn runner
##########################
def aislinnrun(execcmd, filename, binary, id):
    execcmd = re.sub("mpirun", "aislinn", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "--send-protocol=rendezvous", execcmd)
    execcmd = re.sub('\$infty_buffer', "--send-protocol=eager", execcmd)
    execcmd = re.sub('-np ', '-p=', execcmd)

    res, rc, output = run_cmd(
        buildcmd="aislinn-cc -g {} -o {} > {}_{}.txt 2>&1".format(filename,binary,binary,id),
        execcmd=execcmd, 
        binary=binary)

    with open('{}_{}.txt'.format(binary, id), 'w') as outfile:
        outfile.write(output)  

    os.rename("./report.html", "{}_{}.html".format(binary,id))
    
    if res != None:
        return res
    
    if re.search('No errors found', output):
        return 'noerror'
    
    if re.search('Deadlock', output):
        return 'deadlock'
    if re.search('Collective operation mismatch', output):
        return 'deadlock'
    
    if re.search('Invalid rank', output):
        return 'mpierr'
    if re.search('Invalid datatype', output):
        return 'mpierr'
    
    if re.search('Collective operation: root mismatch', output):
        return 'compliance'

    if re.search('Unkown function call', output):
        return 'RSF'
    
    return 'other'

##########################
## CIVL runner
##########################
def civlrun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "java -jar /builds/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/CIVL/CIVL-1.20_5259/lib/civl-1.20_5259.jar verify", execcmd)
    execcmd = re.sub('-np ', "-input_mpi_nprocs=", execcmd)
    execcmd = re.sub('\${EXE}', filename, execcmd)
    execcmd = re.sub('\$zero_buffer', "", execcmd)
    execcmd = re.sub('\$infty_buffer', "", execcmd)

    res, rc, output = run_cmd(
        buildcmd=": # Nothing to compile",
        execcmd=execcmd, 
        binary=binary)

    with open('{}_{}.txt'.format(binary, id), 'w') as outfile:
        outfile.write(output)  
    
    if res != None:
        return res
    
    if re.search('DEADLOCK', output):
        return 'deadlock'
   
    if re.search('has a different root', output):
        return 'compliance'
    if re.search('has a different MPI_Op', output):
        return 'compliance'

    if re.search('MPI message leak', output):
        return 'mpierr'
    if re.search('MPI_ERROR', output):
        return 'mpierr'

    if re.search('MEMORY_LEAK', output):
        return 'resleak'
   
    if re.search('The standard properties hold for all executions', output):
        return 'noerror'

    if re.search('A CIVL internal error has occurred', output):
        return 'RSF'
    
    if re.search('This feature is not yet implemented', output):
        return 'CUN'
    if re.search('doesn.t have a definition', output):
        return 'CUN'
    if re.search('Undeclared identifier', output):
        return 'CUN'
    
    return 'other'

##########################
## ISP runner
##########################
def isprun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "isp.exe", execcmd)
    execcmd = re.sub('-np', '-n', execcmd)
    execcmd = re.sub('\${EXE}', "./{}".format(binary), execcmd)
    execcmd = re.sub('\$zero_buffer', "-b", execcmd)
    execcmd = re.sub('\$infty_buffer', "-g", execcmd)

    print("\nClearing port before executing ISP\n")
    subprocess.run("kill -9 $(lsof -t -i:9999) 2>/dev/null", shell=True)

    res, rc, output = run_cmd(
        buildcmd="ispcc -o {} {} > {}_{}.txt".format(binary,filename,binary,id),
        execcmd=execcmd, 
        binary=binary)

    with open('{}_{}.txt'.format(binary, id), 'w') as outfile:
        outfile.write(output)  
    
    if res != None:
        return res
    
    if re.search('ISP detected deadlock!!!', output):
        return 'deadlock'

    if re.search('resource leaks detected', output):
        return 'resleak'

    if re.search('ISP detected no deadlocks', output):
        return 'noerror'

    if re.search('Fatal error in PMPI', output):
        return 'mpierr'
    if re.search('Fatal error in MPI', output):
        return 'mpierr'
    
    return 'other'

##########################
## MUST runner
##########################
def must_filter(line, process):
    if re.search("ERROR: MUST detected a deadlock", line):
        process.terminate()
def mustrun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "mustrun --must:distributed", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "", execcmd)
    execcmd = re.sub('\$infty_buffer', "", execcmd)	

    res, rc, output = run_cmd(
        buildcmd="mpicc {} -o {} > {}_{}.txt 2>&1".format(filename,binary,binary,id),
        execcmd=execcmd, 
        binary=binary,
        read_line_lambda=must_filter)

    with open('{}_{}.txt'.format(binary, id), 'w') as outfile:
        outfile.write(output)    
    
    if not os.path.isfile("./MUST_Output.html"):
        return 'RSF'

    html = ""
    with open('MUST_Output.html') as input:
        for line in (input.readlines()):
            html += line
    os.rename("./MUST_Output.html", "{}_{}.html".format(binary,id))

    if res != None:
        return res
    
    if re.search('deadlock', html):
        return 'deadlock'
    
    if re.search('not freed', html):
        return 'resleak'

    if re.search('conflicting roots', html):
        return 'compliance'

    if re.search('unknown datatype', html) or re.search('has to be a non-negative integer', html) or re.search('must use equal type signatures', html):
        return 'mpierr'
    
    if re.search('caught MPI error', output):
        return 'mpierr'     
    
    if re.search('Error', html):
        return 'other'

    if re.search('MUST-ERROR', output):
        return 'RSF'
    
    return 'noerror'

##########################
## Parcoach runner
##########################
def parcoachrun(execcmd, filename, binary, id):

    execcmd = "opt-9 -load /builds/MpiCorrectnessBenchmark/mpicorrectnessbenchmark/Parcoach/parcoach/build/src/aSSA/aSSA.so -parcoach -check-mpi {}.bc ".format(binary,binary,id)

    res, rc, output = run_cmd(
        buildcmd="clang -c -g -emit-llvm {} -I/usr/lib/x86_64-linux-gnu/mpich/include/ -o {}.bc".format(filename,binary),
        execcmd=execcmd, 
        binary=binary)

    with open('{}_{}.txt'.format(binary, id), 'w') as outfile:
        outfile.write(output)  
    
    if res != None:
        return res
    
    if re.search('0 warning(s) issued', output):
        return 'noerror'

    if re.search('missing info for external function', output):
        return 'CUN'
    
    return 'deadlock'

##########################
## SimGrid runner
##########################
def simgridrun(execcmd, filename, binary, id):

    execcmd = re.sub("mpirun", "smpirun -wrapper simgrid-mc -platform ./cluster.xml --cfg=smpi/list-leaks:10", execcmd)
    execcmd = re.sub('\${EXE}', binary, execcmd)
    execcmd = re.sub('\$zero_buffer', "--cfg=smpi/buffering:zero", execcmd)
    execcmd = re.sub('\$infty_buffer', "--cfg=smpi/buffering:infty", execcmd)
    
    res, rc, output = run_cmd(
        buildcmd="smpicc {} -o {}  > {}_{}.txt 2>&1".format(filename,binary,binary,id),
        execcmd=execcmd, 
        binary=binary)

    with open('{}_{}.txt'.format(binary, id), 'w') as outfile:
        outfile.write(output)    
        
    if res != None:
        return res
    if re.search('No property violation found', output):
        return 'noerror'
    if re.search('DEADLOCK DETECTED', output):
        return 'deadlock'
    if re.search('returned MPI_ERR', output):
        return 'mpierr'
    if re.search('Not yet implemented', output):
        return 'CUN'
    if re.search('leak detected', output):
        return 'resleak'

    print("Couldn't assign output to specific behaviour (ret: {}) : this will be treated as 'other'".format(rc))
    return 'other'
