# Copyright 2022. The MBI project. All rights reserved.
# This program is free software; you can redistribute it and/or modify it under the terms of the license (GNU GPL).

from MBIutils import *

possible_features=['P2P!basic', 'P2P!nonblocking', 'P2P!persistent', 'COLL!basic', 'COLL!nonblocking', 'COLL!persistent', 'COLL!tools', 'RMA']
possible_characterization=["Lacking", "Yes"]

def parse_file_features(file):
    """Takes a filename and returns a tuple (correct, lacking) of lists of features"""
    correct = []
    lacking = []
    with open(file, 'r') as f:
        line = f.readline()

        # Search for the feature block
        while line != 'BEGIN_MPI_FEATURES\n':
            if line == '':
                raise Exception("Impossible to find the feature block in {}".format(file))
            line = f.readline()

        while line != 'END_MPI_FEATURES\n':
            if line == '':
                raise Exception("Impossible to find the end of the feature block in {}".format(file))

            line = line.strip()
            matching = re.match("^ *([!a-zA-Z0-9]*): ([a-zA-Z0-9]*)$", line)
            if matching is not None:
                (feat, chara) = (matching.group(1), matching.group(2))
                if feat not in possible_features:
                    raise Exception("ERROR: file {} contains an unknown feature: '{}'".format(file, feat))
                if chara not in possible_characterization:
                    raise Exception("ERROR: file {} have feature {} with unknown characterization: '{}'".format(file, feat, chara))
                if chara == 'Yes':
                    correct.append(feat)
                elif chara == 'Lacking':
                    lacking.append(feat)
                else:
                    raise Exception("Impossible")
            line = f.readline()
    if len(correct) > 4:
        raise Exception(f"ERROR: file {file} has more than one 4 features: {correct}")
    return (correct, lacking)

def parse_file_expected(file):
    """Takes a file name, and returns the list of Expect headers (there may be more than one per file)"""
    res  = list(filter(lambda line: line.startswith("  | ERROR: "), open(file, 'r').readlines()))
    res += list(filter(lambda line: line.startswith("  | OK"), open(file, 'r').readlines()))
    if len(res)==0:
        raise Exception("No 'ERROR' nor 'OK' header in {}".format(file))
    res = list(map(lambda line: re.sub("[| ]*ERROR: ", "", line.strip()), res))
    res = list(map(lambda line: re.sub("[| ]*OK *", "OK", line), res))
    for expected in res:
        if expected not in possible_details:
            raise Exception("Unexpected expectation header in {}: '{}'".format(file, expected))
    res = list(map(lambda line: possible_details[line], res))
    return res

def get_C_files_from_dir(dir):
    files = []
    if dir[-1] != '/': # dir must be ended by a / for later separation between the path and basename
        dir = "{}/".format(dir)
    for filename in os.listdir(dir):
        if filename.endswith(".c"):
            files.append("{}/{}".format(dir,filename))
    return files
def filename_to_binary(file):
    return re.sub("_", "\\_", re.sub(".*?//", "", re.sub("\.c","", file)))

def parse_files_per_expected(list):
    """
    Reads all C files from the list and returns a hash [expected -> list_of_lists_of_files_having_that_feature].
    list_of_lists_files elements are of type [file, test_number_in_that_file]
    """
    result = {}
    for expected in possible_details:
        result[ possible_details[expected] ] = []
    for file in list:
        test = 0
        for expected in parse_file_expected(file):
            result[expected].append([file, test])
            test += 1
    return result

def generate_errors(files, outfile):
    files_per_expected = parse_files_per_expected(files)
    def get_counts(categories):
        count = {'total':0}
        for feat in possible_features:
            count[feat] = 0
        seen = []
        for category in categories:
            for (file,test) in files_per_expected[category]:
                if not file in seen:
                    seen.append(file)
                    (features,  _) = parse_file_features(file)
                    count['total'] += 1
                    for feat in features:
                        count[feat] += 1
                else:
                    print(f"Ignore duplicate {file} while counting files per feature.")
        return count
    def show_counts(categories):
        count = get_counts(categories)
        output.write(f"{count['P2P!basic']}&{count['P2P!nonblocking']}&{count['P2P!persistent']}&")
        output.write(f"{count['COLL!basic']}&{count['COLL!nonblocking']}&{count['COLL!tools']} & {count['RMA']} & {count['total']} \\\\")

    with open(outfile, 'w') as output:
        output.write('\\begin{tabular}{|l|l|c|c|c| c|c|c |c||c|}\\cline{3-10}\n')
        output.write('\\multicolumn{2}{c|}{}&\\multicolumn{3}{c|}{Point-to-point}&\\multicolumn{3}{c|}{Collective}&\multirow{6}{*}{RMA}&\multirow{6}{*}{Unique files}\\\\\\cline{3-8}\n')
        output.write('\\multicolumn{2}{c|}{}&\\R{base calls}&\\R{~nonblocking~}&\R{persistent} & \\R{base calls}&\R{~nonblocking~}& \\R{tools} &&\\\\\\hline\n')

        output.write('\\multirow{1}{*}{{Single call}} &Invalid Parameter & ');   show_counts(['AInvalidParam']); output.write(' \\hline')

        output.write('\\multirow{3}{*}{{Single process}}&Resource Leak    & ');  show_counts(['BResLeak'])     ; output.write('\\cline{2-10}\n')
        output.write( '                                 &Request lifecycle& ');  show_counts(['BReqLifecycle']); output.write('\\cline{2-10}\n')
        output.write( '                                 &Epoch lifecycle& ');  show_counts(['BEpochLifecycle']); output.write('\\cline{2-10}\n')
        output.write( '                                 &Local concurrency& ');  show_counts(['BLocalConcurrency']); output.write('\\hline\n')

        output.write('\\multirow{4}{*}{{Multi-processes}}&Parameter matching& ');  show_counts(['CMatch'])        ; output.write('\\cline{2-10}\n')
        output.write( '                                  &Message Race      & ');  show_counts(['DRace'])        ; output.write('\\cline{2-10}\n')
        output.write( '                                  &Call ordering     & ');  show_counts(['DMatch'])       ; output.write('\\cline{2-10}\n')
        output.write( '                                  &Global concurrency& ');  show_counts(['DGlobalConcurrency']); output.write('\\hline\n')

        output.write( '      System & Buffering Hazard    &') ; show_counts(['EBufferingHazard']);output.write('\\hline\n')
        output.write( '      Data   & Input Hazard    &') ; show_counts(['InputHazard']);output.write('\\hline\\hline\n')
        output.write('\\multicolumn{2}{|c|}{Correct codes}&') ; show_counts(['FOK']);output.write('\\hline\\hline\n')

        output.write('\\multicolumn{2}{|c|}{\\textbf{Total}}&')
        show_counts(['AInvalidParam', 'BResLeak','BReqLifecycle','BEpochLifecycle','BLocalConcurrency', 'CMatch', 'DRace','DMatch','DGlobalConcurrency', 'EBufferingHazard', 'InputHazard', 'FOK'])
        output.write('\\hline\n')

        output.write('\\end{tabular}\n')


def generate_labels(files, outfile):
    files_per_expected = parse_files_per_expected(files)

    # Get the data
    OK = {'total':0}
    Error = {'total':0}
    for feat in possible_features:
        OK[feat] = 0
        Error[feat] = 0
    seen = []
    for detail in possible_details:
        category = possible_details[detail]
        for (file,test) in files_per_expected[category]:
            if not file in seen:
                seen.append(file)
                (features,  _) = parse_file_features(file)
                if detail == 'OK':
                    OK['total'] += 1
                    for feat in features:
                        OK[feat] += 1
                else:
                    Error['total'] += 1
                    for feat in features:
                        Error[feat] += 1
            else:
                print(f"Ignore duplicate {file} while counting files per label.")

    # Produce the output
    with open(outfile, 'w') as output:
        output.write('\\begin{tabular}{|l| l | l | c | c |}\\hline\n')
        output.write('\\multicolumn{2}{|c|}{ \\textbf{MPI}} & \\multirow{2}{*}{\\textbf{Description}} & \\multicolumn{2}{c|}{\\textbf{Number of codes using the label}} \\\\')
        output.write('\\multicolumn{2}{|c|}{ \\textbf{Feature Label}} &  & \\# Incorrect codes & \\# Correct codes \\\\ \\hline\n')

        output.write("\\parbox[t]{4mm}{\\multirow{3}{*}{\\R{P2P}}} & base calls & Use of blocking point-to-point communication)")
        output.write(f" & {Error['P2P!basic']} & {OK['P2P!basic']} \\\\ \n")
        output.write("& nonblocking & Use of  nonblocking point-to-point communication")
        output.write(f" & {Error['P2P!nonblocking']} & {OK['P2P!nonblocking']} \\\\ \n")
#        output.write(f" &  116 &  19 \\\\ \n")
        output.write("& persistent & Use of point-to-point persistent communications")
        output.write(f" & {Error['P2P!persistent']} & {OK['P2P!persistent']} \\\\ \\hline \n")
#        output.write(f" &  45 &  8 \\\\ \\hline \n")
        output.write("\\parbox[t]{2mm}{\\multirow{3}{*}{\\R{COLL}}} & base calls & Use of blocking collective communication")
        output.write(f" & {Error['COLL!basic']} & {OK['COLL!basic']} \\\\ \n")
#        output.write(f" &  312 &  202 \\\\ \n")
        output.write("& nonblocking & Use of nonblocking collective communication")
        output.write(f" & {Error['COLL!nonblocking']} & {OK['COLL!nonblocking']} \\\\ \n")
#        output.write(f" &  129 &   114 \\\\ \n")
        output.write("& tools & Use of resource function (e.g.,  communicators, datatypes)")
        output.write(f" & {Error['COLL!tools']} & {OK['COLL!tools']} \\\\ \\hline \n")
#        output.write(f" &  94 &  23 \\\\ \\hline \n")
        output.write("\\multicolumn{2}{|c|}{RMA} & Use of Remote Memory Access")
        output.write(f" & {Error['RMA']} & {OK['RMA']} \\\\ \\hline \n")
#        output.write(f" &  30 &  3 \\\\ \\hline \n")
        output.write("\\end{tabular}\n")


    # def show_counts(categories):
    #     count = get_counts(categories)
    #     output.write(f"{count['P2P!basic']}&{count['P2P!nonblocking']}&{count['P2P!persistent']}&")
    #     output.write(f"{count['COLL!basic']}&{count['COLL!nonblocking']}&{count['COLL!tools']} & {count['RMA']} & {count['total']} \\\\")
