import os
import time
import subprocess
import sys
import shlex
import select
import signal


def run_cmd(buildcmd, execcmd, cachefile, binary, timeout, read_line_lambda=None):
    if os.path.exists(f'{cachefile}.txt') and os.path.exists(f'{cachefile}.elapsed'):
        print(f" (cached result found for {cachefile})", end='')
        return

    start_time = time.time()
    if buildcmd == None:
        output = f"No need to compile https://gitlab.com/MbiBugsInitiative/MbiBugsInitiative/-/tree/master/codes/{binary}.c\n\n"
    else:
        output = f"Compiling https://gitlab.com/MbiBugsInitiative/MbiBugsInitiative/-/tree/master/codes/{binary}.c\n\n"
        output += f"$ {buildcmd}\n"

        compil = subprocess.run(buildcmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if compil.stdout is not None:
            output += str(compil.stdout, errors='replace')
        if compil.returncode != 0:
            output += f"Compilation of {binary}.c raised an error (retcode: {compil.returncode})"
            for line in (output.split('\n')):
                print(f"| {line}", file=sys.stderr)
            return 'UNIMPLEMENTED', compil.returncode, output

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
        if process.poll() is not None:  # The subprocess ended
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

    rc = process.poll()
    if rc < 0:
        status = f"Command killed by signal {-rc}\n"
    else:
        status = f"Command return code: {rc}\n"
    print(status)
    output += status

    elapsed = time.time() - start_time
    with open(f'{cachefile}.elapsed', 'w') as outfile:
        outfile.write(str(elapsed))

    with open(f'{cachefile}.txt', 'w') as outfile:
        outfile.write(output)