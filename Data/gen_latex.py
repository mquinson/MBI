#! /usr/bin/python3
import os,re,sys

possible_features=["P2P", "iP2P", "PERS", "COLL", "iCOLL", "TOPO", "RMA", "IO", "PROB", "COM", "GRP", "DATA", "OP"]
possible_characterization=["Lacking", "Incorrect", "Correct"]
possible_expected=['noerror', 'deadlock', 'datarace', 'mpierr', 'numstab', 'resleak', 'various']

feat_to_color = {'P2P':'red', 'iP2P':'red!80', 'PERS':'purple', 'COLL':'green', 'iCOLL':'green!80', 'TOPO':'purple!20', 'RMA':'black',
    "PROB":'black', "COM":'black', "GRP":'black', "DATA":'black', "OP":'black'}
feat_to_color = {'P2P':'viridis0', 'iP2P':'viridis1', 'PERS':'viridis3', 'COLL':'viridis5', 'iCOLL':'viridis6', 'TOPO':'viridis8', 'RMA':'viridis10',
    "PROB":'viridis11', "COM":'viridis13', "GRP":'viridis15', "DATA":'viridis16', "OP":'viridis17'}
feat_to_bgcolor = {'P2P':'white', 'iP2P':'white', 'PERS':'white', 'COLL':'white', 'iCOLL':'white', 'TOPO':'white', 'RMA':'black',
    "PROB":'black', "COM":'black', "GRP":'black', "DATA":'black', "OP":'black'}

def parse_file_features(file):
    """Takes a filename and returns a tuple (correct, incorrect, lacking) of lists of features"""
    correct = []
    incorrect = []
    lacking = []
    with open(file, 'r') as f:
        line = f.readline()
        
        # Search for the feature block
        while line != '//// List of features\n':
            if line == '':
                raise Exception("Impossible to find the feature block in {}".format(file))
            line = f.readline() 

        while line != '//// List of errors\n':
            if line == '':
                raise Exception("Impossible to find the end of the feature block in {}".format(file))

            matching = re.match("^// ([a-zA-Z0-9]*): ([a-zA-Z0-9]*)$", line)
            if matching is not None:
                (feat, chara) = (matching.group(1), matching.group(2))
                if feat not in possible_features:
                    raise Exception("ERROR: file {} contains an unknown feature: '{}'".format(file, feat))
                if chara not in possible_characterization:
                    raise Exception("ERROR: file {} have feature {} with unknown characterization: '{}'".format(file, feat, chara))
                if chara == 'Correct':
                    correct.append(feat)
                elif chara == 'Incorrect':
                    incorrect.append(feat)
                elif chara == 'Lacking':
                    lacking.append(feat)
                else:
                    raise Exception("Impossible")
            line = f.readline()
    if len(correct) + len(incorrect) > 4:
        raise Exception(f"ERROR: file {file} has more than one 4 features: {correct} {incorrect}")
#    if len(incorrect) > 1:
#        raise Exception(f"ERROR: file {file} has more than one broken feature: {incorrect}")
    return (correct, incorrect, lacking)

def parse_file_expected(file):
    """Takes a file name, and returns the list of Expect headers (there may be more than one per file)"""
    res = list(filter(lambda line: line.startswith("// Expect: "), open(file, 'r').readlines()))
    if len(res)==0:
        raise Exception("No 'Expect:' header in {}".format(file))
    res = list(map(lambda line: re.sub("// Expect: ", "", line.strip()), res))
    for expected in res:
        if expected not in possible_expected:
            raise Exception("Unexpected 'Expect:' header in {}: '{}'".format(file, expected))
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
    return re.sub(".*?//", "", re.sub("\.c","", file))

def parse_files_per_expected(list):
    """
    Reads all C files from the list and returns a hash [expected -> list_of_lists_of_files_having_that_feature].
    list_of_lists_files elements are of type [file, test_number_in_that_file]
    """
    result = {}
    for file in list:
        test = 0
        for expected in parse_file_expected(file):
            result[expected] = [] if expected not in result else result[expected]
            result[expected].append([file, test])
            test += 1
    return result

def generate_features(files, outfile):
    lineheight = 0.4
    feat_width = 0.7
    cell_width = feat_width * 4
    cell_per_line = 6 
    files_per_expected = parse_files_per_expected(files)

    line = 50
    with open(outfile, 'w') as output:
        output.write("\\begin{tikzpicture}\n")
        for expected in possible_expected:
            output.write(f" \\draw(0,{line*lineheight}) node {{{expected}}};\n")
            line -= 1
            cell = 0 # Position of this file on the line
            # Draw the boxes
            initial_line = line
            for (file,test) in files_per_expected[expected]:
                (features, incorrect_feat, _) = parse_file_features(file)
                file = f'{filename_to_binary(file)}\\#{test}'
                output.write(f" \\draw ({cell*cell_width-(0.4*feat_width)}, {(line+0.4)*lineheight}) rectangle ({cell*cell_width+(3.45*feat_width)}, {(line-0.4)*lineheight});\n")
                xpos = 0
                for feat in incorrect_feat:
                    output.write(f"  \\draw [fill={feat_to_color[feat]}] ({cell*cell_width + xpos-(0.4*feat_width)}, {(line-0.4)*lineheight}) rectangle ({cell*cell_width + xpos + (0.45*feat_width)}, {(line+0.4)*lineheight});\n")
                    xpos += feat_width
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
                (features, incorrect_feat, _) = parse_file_features(file)
                file = f'{filename_to_binary(file)}\\#{test}'
                xpos = 0
                for feat in incorrect_feat:
                    output.write(f"  \\draw ({cell*cell_width + xpos}, {line*lineheight}) node {{\\scriptsize{{\\tooltip****[{feat_to_bgcolor[feat]}]{{\\sout{{{feat}}}}}{{{file} -- incorrect: {feat}}}}}}};\n")
                    xpos += feat_width
                for feat in features:
                    output.write(f"  \\draw ({cell*cell_width + xpos}, {line*lineheight}) node {{\\scriptsize{{\\tooltip****[{feat_to_bgcolor[feat]}]{{{feat}}}{{{file} -- correct: {feat}}}}}}};\n")
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
        output.write("\\end{tikzpicture}\n")

generate_features(get_C_files_from_dir("../Benchmarks/microbenchs"), "/tmp/features.tex")