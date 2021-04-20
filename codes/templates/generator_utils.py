import re


def find_line(content, target, filename):
    res = 1
    for line in content.split('\n'):
        if re.search(f'[^:]{target}', line):
            #print(f'Found {target} at {line}')
            return res
        res += 1
    raise Exception(f"Line target {target} not found in {filename}.")


def make_file(template, filename, replace):
    output = template
    filename = re.sub("_MPI_", "_", filename)
    replace['filename'] = filename
    # Replace all variables that don't have a ':' in their name
    while re.search("@\{[^@:]*\}@", output):
        m = re.search("@\{([^@:]*)\}@", output)
        target = m.group(1)
        #print(f"Replace @{{{target}}}@")
        if target in replace.keys():
            output = re.sub(f'@\{{{target}\}}@', replace[target], output)
            #print(f"Replace {target} -> {replace[target]}")
        else:
            raise Exception(f"Variable {target} used in template, but not defined.")
    # Now replace all variables with a ':' in their name: line targets are like that, and we don't want to resolve them before the others change the lines
    while re.search("@\{([^:@]*):([^@]*)\}@", output):
        m = re.search("@\{([^:@]*):([^@]*)\}@", output)
        (kind, target) = (m.group(1), m.group(2))
        if kind == 'line':
            replace = f'{find_line(output, target, filename)}'
            #print(f"Replace @{{line:{target}}}@ with '{replace}'")
            output = re.sub(f'@\{{line:{target}\}}@', replace, output)
        else:
            raise Exception(f"Unknown variable kind: {kind}:{target}")
    # Ready to output it
    with open(filename, 'w') as outfile:
        outfile.write(output)
