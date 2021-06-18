import os
import time
import subprocess
import sys
import shlex
import select
import signal
import hashlib

class AbstractTool:
    def ensure_image(self, params=""):
        """Verify that this is executed from the right docker image, and complain if not."""
        if os.path.exists("/MBI"):
            print("This seems to be a MBI docker image. Good.")
        else:
            print("Please run this script in a MBI docker image. Run these commands:")
            print("  docker build -f Dockerfile -t mpi-bugs-initiative:latest . # Only the first time")
            print(f"  docker run -it --rm --name MIB --volume $(pwd):/MBI mpi-bugs-initiative /MBI/MBI.py {params}")
            sys.exit(1)

    def build(self, rootdir, cached=True):
        """Rebuilds the tool binaries. By default, we try to reuse the existing build."""
        print ("Nothing to do to rebuild the tool binaries.")

    def setup(self, rootdir):
        """
        Ensure that this tool (previously built) is usable in this environment: setup the PATH, etc.
        This is called only once for all tests, from the logs directory.
        """
        pass

    def run(execcmd, filename, binary, id, timeout):
        """Compile that test code and anaylse it with the Tool if needed (a cache system should be used)"""
        pass

    def teardown(self):
        """
        Clean the results of all test runs: remove temp files and binaries.
        This is called only once for all tests, from the logs directory.
        """
        pass

    def parse(self, cachefile):
        """Read the result of a previous run from the cache, and compute the test outcome"""
        return 'failure'

def run_cmd(buildcmd, execcmd, cachefile, filename, binary, timeout, batchinfo, read_line_lambda=None):
    """
    Runs the test on need. Returns True if the test was ran, and False if it was cached.
    
    The result is cached if possible, and the test is rerun only if the `test.txt` (containing the tool output) or the `test.elapsed` (containing the timing info) do not exist, or if `test.md5sum` (containing the md5sum of the code to compile) does not match.

    Parameters:
     - buildcmd and execcmd are shell commands to run. 
     - cachefile is the name of the test
     - filename is the source file containing the code
     - binary the file name in which to compile the code
     - batchinfo: something like "1/1" to say that this run is the only batch (see -b parameter of MBI.py)
     - read_line_lambda: a lambda to which each line of the tool output is feed ASAP. It allows MUST to interrupt the execution when a deadlock is reported.
    """
    if os.path.exists(f'{cachefile}.txt') and os.path.exists(f'{cachefile}.elapsed') and os.path.exists(f'{cachefile}.md5sum'):
        hash_md5 = hashlib.md5()
        with open(filename, 'rb') as sourcefile :
            for chunk in iter(lambda: sourcefile.read(4096), b""):
                hash_md5.update(chunk)
        newdigest = hash_md5.hexdigest()
        with open(f'{cachefile}.md5sum', 'r') as md5file:
            olddigest = md5file.read()
        #print(f'Old digest: {olddigest}; New digest: {newdigest}')
        if olddigest == newdigest:
            print(f" (cached result found for {cachefile} -- digest of {filename}: {olddigest})")
            return False
        else:
            os.remove(f'{cachefile}.txt')

    print(f"Wait up to {timeout} seconds")

    start_time = time.time()
    if buildcmd == None:
        output = f"No need to compile {binary}.c (batchinfo:{batchinfo})\n\n"
    else:
        output = f"Compiling {binary}.c (batchinfo:{batchinfo})\n\n"
        output += f"$ {buildcmd}\n"

        compil = subprocess.run(buildcmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if compil.stdout is not None:
            output += str(compil.stdout, errors='replace')
        if compil.returncode != 0:
            output += f"Compilation of {binary}.c raised an error (retcode: {compil.returncode})"
            for line in (output.split('\n')):
                print(f"| {line}", file=sys.stderr)
            with open(f'{cachefile}.elapsed', 'w') as outfile:
                outfile.write(str(time.time() - start_time))
            with open(f'{cachefile}.txt', 'w') as outfile:
                outfile.write(output)
            return True

    output += f"\n\nExecuting the command\n $ {execcmd}\n"
    for line in (output.split('\n')):
        print(f"| {line}", file=sys.stderr)

    # We run the subprocess and parse its output line by line, so that we can kill it as soon as it detects a timeout
    process = subprocess.Popen(shlex.split(execcmd), stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, preexec_fn=os.setsid)
    poll_obj = select.poll()
    poll_obj.register(process.stdout, select.POLLIN)

    pid = process.pid
    pgid = os.getpgid(pid)  # We need that to forcefully kill subprocesses when leaving
    outcome = None
    while True:
        if poll_obj.poll(5):  # Something to read? Do check the timeout status every 5 sec if not
            line = process.stdout.readline()
            # From byte array to string, replacing non-representable strings with question marks
            line = str(line, errors='replace')
            output = output + line
            print(f"| {line}", end='', file=sys.stderr)
            if read_line_lambda != None:
                read_line_lambda(line, process)
        if time.time() - start_time > timeout:
            outcome = 'timeout'
            with open(f'{cachefile}.timeout', 'w') as outfile:
                outfile.write(f'{time.time() - start_time} seconds')
            break
        if process.poll() is not None:  # The subprocess ended. Grab all existing output, and return
            line = 'more'
            while line != None and line != '':
                line = process.stdout.readline()
                if line is not None:
                    # From byte array to string, replacing non-representable strings with question marks
                    line = str(line, errors='replace')
                    output = output + line
                    print(f"| {line}", end='', file=sys.stderr)

            break

    # We want to clean all forked processes in all cases, no matter whether they are still running (timeout) or supposed to be off. The runners easily get clogged with zombies :(
    try:
        os.killpg(pgid, signal.SIGTERM)  # Terminate all forked processes, to make sure it's clean whatever the tool does
        process.terminate()  # No op if it's already stopped but useful on timeouts
        time.sleep(0.2)  # allow some time for the tool to finish its childs
        os.killpg(pgid, signal.SIGKILL)  # Finish 'em all, manually
        os.kill(pid, signal.SIGKILL)  # die! die! die!
    except ProcessLookupError:
        pass  # OK, it's gone now

    elapsed = time.time() - start_time

    rc = process.poll()
    if rc < 0:
        status = f"Command killed by signal {-rc}, elapsed time: {elapsed}\n"
    else:
        status = f"Command return code: {rc}, elapsed time: {elapsed}\n"
    print(status)
    output += status

    with open(f'{cachefile}.elapsed', 'w') as outfile:
        outfile.write(str(elapsed))

    with open(f'{cachefile}.txt', 'w') as outfile:
        outfile.write(output)
    with open(f'{cachefile}.md5sum', 'w') as outfile:
        md5_hash = hashlib.md5()
        with open(filename, 'rb') as sourcefile :
            for chunk in iter(lambda: sourcefile.read(4096), b""):
                hash_md5.update(chunk)
        newdigest = md5_hash.hexdigest()

        outfile.write(newdigest)
    
    return True
