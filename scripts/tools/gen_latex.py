#! /usr/bin/python3
import os,re,sys

possible_features=['P2P!basic', 'P2P!nonblocking', 'P2P!persistent', 'COLL!basic', 'COLL!nonblocking', 'COLL!persistent', 'COLL!tools', 'RMA']
possible_characterization=["Lacking", "Yes"]

possible_details = {
    # scope limited to one call
    'InvalidCommunicator':'AInvalidParam', 'InvalidDatatype':'AInvalidParam', 'InvalidRoot':'AInvalidParam', 'InvalidTag':'AInvalidParam', 'InvalidWindow':'AInvalidParam', 'InvalidOperator':'AInvalidParam', 'InvalidOtherArg':'AInvalidParam', 'ActualDatatype':'AInvalidParam',
    'InvalidSrcDest':'AInvalidParam',
    # scope: Process-wide
#    'OutOfInitFini':'BInitFini',
    'CommunicatorLeak':'BResLeak', 'DatatypeLeak':'BResLeak', 'GroupLeak':'BResLeak', 'OperatorLeak':'BResLeak', 'TypeLeak':'BResLeak', 'RequestLeak':'BResLeak',
    'MissingStart':'BReqLifecycle', 'MissingWait':'BReqLifecycle',
    'LocalConcurrency':'BLocalConcurrency',
    # scope: communicator
    'CallMatching':'DMatch',
    'CommunicatorMatching':'CMatch', 'DatatypeMatching':'CMatch', 'OperatorMatching':'CMatch', 'RootMatching':'CMatch', 'TagMatching':'CMatch',
    'MessageRace':'DRace',

    'GlobalConcurrency':'DGlobalConcurrency',
    # larger scope
#    'BufferingHazard':'EBufferingHazard',
    'OK':'FOK'}

displayed_name = {
    'AInvalidParam':'Invalid parameter',
    'BResLeak':'Resource leak',
#    'BInitFini':'MPI call before initialization/after finalization',
    'BReqLifecycle':'Request lifecycle',
    'BLocalConcurrency':'Local concurrency',
    'CMatch':'Parameter matching',
    'DMatch':"Call matching",
    'DRace':'Message race',
    'DGlobalConcurrency':'Global concurrency',
#    'EBufferingHazard':'Buffering hazard',
    'FOK':"Correct code",

    'P2P!basic':'P2P', 'P2P!nonblocking':'iP2P', 'P2P!persistent':'pP2P',
    'COLL!basic':'Coll', 'COLL!nonblocking':'iColl', 'COLL!tools':'Coll+',
    'RMA':'RMA',

    'aislinn':'Aislinn','civl':'CIVL', 'isp':'ISP', 'simgrid':'Mc SimGrid','smpi':'SMPI', 'mpisv':'MPI-SV', 'must':'MUST', 'parcoach':'PARCOACH'
}


#feat_to_color = {'P2P!basic':'red', 'iP2P':'red!80', 'PERS':'purple', 'COLL':'green', 'iCOLL':'green!80', 'TOPO':'purple!20', 'RMA':'black',
#    "PROB":'black', "COM":'black', "GRP":'black', "DATA":'black', "OP":'black'}
feat_to_color = {'P2P!basic':'viridis0', 'P2P!nonblocking':'viridis1', 'P2P!persistent':'viridis3',
    'RMA':'viridis10',
    "COLL!basic":'viridis15', "COLL!nonblocking":'viridis16', "COLL!tools":'viridis17'}
feat_to_bgcolor = {'P2P!basic':'white', 'P2P!nonblocking':'white', 'P2P!persistent':'white',
    'RMA':'black',
    "COLL!basic":'black', "COLL!nonblocking":'black', "COLL!tools":'black'}

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

def generate_features(files, outfile):
    lineheight = 0.4
    feat_width = 0.7
    cell_width = feat_width * 3
    cell_per_line = 10
    files_per_expected = parse_files_per_expected(files)

    line = 800
    with open(outfile, 'w') as output:
        output.write("\\resizebox{\\linewidth}{!}{\\begin{tikzpicture}\n")
        categories = []
        for expected in possible_details:
            if not possible_details[expected] in categories:
                categories.append(possible_details[expected])
        for expected in sorted(categories):
            output.write(f" \\draw({cell_width*cell_per_line/2},{line*lineheight}) node {{\\large{{{displayed_name[expected]}}}}};\n")
            line -= 1
            cell = 0 # Position of this file on the line
            # Draw the boxes
            initial_line = line
            for (file,test) in files_per_expected[expected]:
                (features, _) = parse_file_features(file)
                file = f'{filename_to_binary(file)}\\#{test}'
                output.write(f" \\draw ({cell*cell_width-(0.4*feat_width)}, {(line+0.4)*lineheight}) rectangle ({cell*cell_width+(3.45*feat_width)}, {(line-0.4)*lineheight});\n")
                xpos = 0
#                for feat in incorrect_feat:
#                    output.write(f"  \\draw [fill={feat_to_color[feat]}] ({cell*cell_width + xpos-(0.4*feat_width)}, {(line-0.4)*lineheight}) rectangle ({cell*cell_width + xpos + (0.45*feat_width)}, {(line+0.4)*lineheight});\n")
#                    xpos += feat_width
                for feat in features:
                    output.write(f"  \\draw [fill={feat_to_color[feat]}] ({cell*cell_width + xpos-(0.4*feat_width)}, {(line-0.4)*lineheight}) rectangle ({cell*cell_width + xpos + (0.45*feat_width)}, {(line+0.4)*lineheight});\n")
                    xpos += feat_width
                if cell+1 == cell_per_line:
                    cell = 0
                    line -= 1
                    if line < 0:
                        raise Exception("Too much lines. Please increase the initial value of line")
                else :
                    cell += 1

            # Put the texts (must come after all boxes for the tooltip to not be hidden behind)
            cell = 0
            line = initial_line
            for (file,test) in files_per_expected[expected]:
                (features,  _) = parse_file_features(file)
                file = f'{filename_to_binary(file)}\\#{test}'
                xpos = 0
#                for feat in incorrect_feat:
#                    output.write(f"  \\draw ({cell*cell_width + xpos}, {line*lineheight}) node {{\\scriptsize{{\\tooltip****[{feat_to_bgcolor[feat]}]{{\\sout{{{feat}}}}}{{{file} -- incorrect: {feat}}}}}}};\n")
#                    xpos += feat_width
                for feat in features:
#                    output.write(f"  \\draw ({cell*cell_width + xpos}, {line*lineheight}) node {{\\scriptsize{{\\tooltip****[{feat_to_bgcolor[feat]}]{{{feat}}}{{{file} -- correct: {feat}}}}}}};\n")
                    output.write(f"  \\draw ({cell*cell_width + xpos}, {line*lineheight}) node {{\\scriptsize{{\\color{{{feat_to_bgcolor[feat]}}}{{{displayed_name[feat]}}}}}}};\n")
                    xpos += feat_width
                if cell+1 == cell_per_line:
                    cell = 0
                    line -= 1
                    if line < 0:
                        raise Exception("Too much lines. Please increase the initial value of line")
                else :
                    cell += 1
            if cell != 0: # we did not output anything on the new line, no need to go further
                line -= 1
        output.write("\\end{tikzpicture}}\n")

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


    def show_counts(categories):
        count = get_counts(categories)
        output.write(f"{count['P2P!basic']}&{count['P2P!nonblocking']}&{count['P2P!persistent']}&")
        output.write(f"{count['COLL!basic']}&{count['COLL!nonblocking']}&{count['COLL!tools']} & {count['RMA']} & {count['total']} \\\\")

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
        output.write( '                                 &Local concurrency& ');  show_counts(['BLocalConcurrency']); output.write('\\hline\n')

        output.write('\\multirow{4}{*}{{Multi-processes}}&Parameter matching& ');  show_counts(['CMatch'])        ; output.write('\\cline{2-10}\n')
        output.write( '                                  &Message Race      & ');  show_counts(['DRace'])        ; output.write('\\cline{2-10}\n')
        output.write( '                                  &Call ordering     & ');  show_counts(['DMatch'])       ; output.write('\\cline{2-10}\n')
        output.write( '                                  &Global concurrency& ');  show_counts(['DGlobalConcurrency']); output.write('\\hline\n')

#        output.write( '      System & Buffering Hazard    &') ; show_counts(['EBufferingHazard']);output.write('\\hline\\hline\n')
        output.write('\\multicolumn{2}{|c|}{Correct codes}&') ; show_counts(['FOK']);output.write('\\hline\\hline\n')

        output.write('\\multicolumn{2}{|c|}{\\textbf{Total}}&')
        # 'EBufferingHazard', hidden from the following list
        show_counts(['AInvalidParam', 'BResLeak','BReqLifecycle','BLocalConcurrency', 'CMatch', 'DRace','DMatch','DGlobalConcurrency', 'FOK'])
        output.write('\\hline\n')

        output.write('\\end{tabular}\n')

# Obsolete view with all collored boxes
#generate_features(get_C_files_from_dir("../gencodes"), "../latex/features.tex")
generate_labels(get_C_files_from_dir("../gencodes"), "../latex/labels.tex")
generate_errors(get_C_files_from_dir("../gencodes"), "../latex/errors.tex")