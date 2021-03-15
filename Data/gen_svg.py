import drawSvg as draw
import csv
import argparse
import ast

#############################
## Argument handling
#############################

parser = argparse.ArgumentParser(description='This script will generate nice dashboard with link to C sources or execution logs, as well as the actual result. It takes a csv as input and generate both a svg and png')


parser.add_argument('-i', metavar='input', default='result.csv', help='the input csv file containing all the data generated from our runner.')

parser.add_argument('-o', metavar='output', default='out', type=str, help='name of the file that will be generated (both output.svg and output.png)')

parser.add_argument('--header', metavar='header', default=True, type=bool, help='wether the input csv contains an header')

parser.add_argument('--mode', metavar='mode', default="log", type=str, help='wether the link should point to execution log or C src code (either <log> or <src>)')

args = parser.parse_args()

DEBUG = False
if args.mode == "log":
    DEBUG = True
    
#############################
## Specific SVG class
#############################

    
class Hyperlink(draw.DrawingParentElement):
    TAG_NAME = 'a'
    def __init__(self, href, target=None, **kwargs):
        super().__init__(href=href, target=target, **kwargs)

class Image(draw.DrawingBasicElement):
    TAG_NAME = 'image'
    def __init__(self, x, y, width, height, href, **kwargs):
        try:
            y = -y-height
        except TypeError:
            pass
        super().__init__(x=x, y=y, width=width, height=height, href=href, **kwargs)

class Group(draw.DrawingDef):
    TAG_NAME = 'g'
    def __init__(self, ids, **kwargs):
        super().__init__(id=ids, **kwargs)

        
#############################
## GLOBAL CONSTANT
#############################

WIDTH = 800
HEIGHT = 600
HEADER_SIZE = 15


#############################
## Handling CSV
#############################

csvread = []
with open(args.i, newline='') as result_file:
    csvread = csv.reader(result_file, delimiter=';')
    isHeader = args.header
    data = []
    for row in csvread:
        if isHeader:          
            header = row
        else:
            data.append(row)
        isHeader = False

d = draw.Drawing(WIDTH, HEIGHT, origin='center', displayInline=True)

#############################
## Data dependant constant
#############################


tools = list(set([row[2] for row in data]))
tools.sort()

width_per_tool = WIDTH / (len(tools) + 1)

error_type = list(set([ast.literal_eval(row[6])[0] for row in data]))
error_type.sort()

height_per_error = HEIGHT / (len(error_type) + 1)

nb_uniq_testcase = len([R for R in data if R[2]==tools[0]])

CASE_HEIGHT = HEIGHT / ((nb_uniq_testcase // 5)*1.1 + len(error_type) + 1)
CASE_WIDTH = width_per_tool / (5.7)

case_per_error = []
for error in error_type:
    case_per_error.append([row for row in data if ast.literal_eval(row[6])[0] == error])


#############################
## Actual printing method
#############################

def print_result(top_left_x, top_left_y, i, j, row):


    name = row[0]
    id = row[1]
    tool = row[2]
    to = row[3]
    np = row[4]
    buf = row[5]
    expected = ast.literal_eval(row[6])
    result = row[7]
    job_id = row[8]

    if DEBUG:
        link = "https://gitlab.com/Mwapl/benchmarking-mpi/-/jobs/{}/artifacts/raw/Benchmarks/{}_{}.txt".format(job_id,name,id)
    else:
        link = "https://gitlab.com/Mwapl/benchmarking-mpi/-/tree/master/Benchmarks/microbenchs/{}.c".format(name)
    
    r = Hyperlink(link)
    
    fig = "tick.svg"
    
    if result == "timeout":
        fig = "TO.svg"
    elif result == "CUN":
        fig = "CUN.svg"
    elif result == "RSF":
        fig = "RSF.svg"
    elif result not in expected:
        fig = "cross.svg"
    # elif expected[0] == "noerror" and result != "noerror":
    #     fig = "cross.svg"
    # elif expected[0] != "noerror" and result == "noerror":
    #     fig = "cross.svg"
    

    r.append(draw.Image(top_left_x + 0.1*CASE_WIDTH + i * (CASE_WIDTH*1.1),
            top_left_y - 0.1*CASE_HEIGHT - j * (CASE_HEIGHT*1.1),
            CASE_WIDTH,
            CASE_HEIGHT,
            fig,
            embed=True))

    r.append(draw.Rectangle(top_left_x + 0.1*CASE_WIDTH + i * (CASE_WIDTH*1.1),
            top_left_y - 0.1*CASE_HEIGHT - j * (CASE_HEIGHT*1.1),
            CASE_WIDTH,
            CASE_HEIGHT,
            fill='none',
            stroke="black",
            stroke_width="0.2"
    ))
    
    desc = "TEST : " + tool + " np=" + np + " to=" + to
    if buf != 'NA':
        desc += " buf=" + buf
    desc += " " + name

    desc += "\nEXPECTED : " + expected[0]

    if result == 'CUN': 
        desc += "\nRETURNED : Compilation Failure"  
    elif result == 'RSF':
        desc += "\nRETURNED : Runtime Failure"  
    else :
        desc += "\nRETURNED : " + result  
        
    r.append(draw.Title(desc))
    
    return r

    
#############################
## Going throught data
#############################


for i in range(len(tools)):
    d.append(draw.Text(tools[i], HEADER_SIZE, -(WIDTH/2) + (i+1)*width_per_tool,  (HEIGHT/2) - 15, fill='black'))


adjust_height = 50
    
for i in range(len(error_type)):

    # Print the error name
    d.append(draw.Text(error_type[i], HEADER_SIZE/2, -(WIDTH/2) + 5,  (HEIGHT/2) - adjust_height, fill='black'))
    
    
    for j in range(len(tools)):
        
        to_print = [cases for cases in case_per_error[i] if cases[2]==tools[j]]
        to_print.sort()
        
        for k in range(len(to_print)):
            row = to_print[k]
            d.append(print_result(-(WIDTH/2) + (j+1)*width_per_tool,
                                  (HEIGHT/2) - adjust_height,
                                  k%5,
                                  k//5,
                                  row))

    to_add = len(to_print)//5
    if len(to_print)%5!=0:
        to_add+=1
    adjust_height += (to_add)*CASE_HEIGHT*1.1
          


d.setPixelScale(2)  # Set number of pixels per geometry unit
#d.setRenderSize(400,200)  # Alternative to setPixelScale

d.saveSvg(args.o + '.svg')
d.savePng(args.o + '.png')


#############################
## Generating the caption
#############################

caption = draw.Drawing(CASE_WIDTH*15, CASE_HEIGHT*10, displayInline=True)

x = 10
y = CASE_HEIGHT*10 - 20

caption.append(draw.Image(x, y, CASE_WIDTH, CASE_HEIGHT, "tick.svg", embed=True))

caption.append(draw.Rectangle(x, y, CASE_WIDTH, CASE_HEIGHT,
            fill='none',
            stroke="black",
            stroke_width="0.2"
    ))

caption.append(draw.Text("Right", HEADER_SIZE, x + 1.5*CASE_WIDTH, y, fill='black'))

y -= CASE_HEIGHT*1.5

caption.append(draw.Image(x,
                    y,
                    CASE_WIDTH,
                    CASE_HEIGHT,
                    "cross.svg",
                    embed=True))

caption.append(draw.Rectangle(x,
                              y,
                              CASE_WIDTH, CASE_HEIGHT,
                              fill='none',
                              stroke="black",
                              stroke_width="0.2"))

caption.append(draw.Text("Wrong", HEADER_SIZE, x + 1.5*CASE_WIDTH, y, fill='black'))

y -= CASE_HEIGHT*1.5

caption.append(draw.Image(x,
                    y,
                    CASE_WIDTH,
                    CASE_HEIGHT,
                    "TO.svg",
                    embed=True))

caption.append(draw.Rectangle(x,
                              y,
                              CASE_WIDTH, CASE_HEIGHT,
                              fill='none',
                              stroke="black",
                              stroke_width="0.2"))

caption.append(draw.Text("Time Out", HEADER_SIZE, x + 1.5*CASE_WIDTH, y, fill='black'))

y -= CASE_HEIGHT*1.5

caption.append(draw.Image(x,
                    y,
                    CASE_WIDTH,
                    CASE_HEIGHT,
                    "CUN.svg",
                    embed=True))

caption.append(draw.Rectangle(x,
                              y,
                              CASE_WIDTH, CASE_HEIGHT,
                              fill='none',
                              stroke="black",
                              stroke_width="0.2"))

caption.append(draw.Text("Unsupported feature", HEADER_SIZE, x + 1.5*CASE_WIDTH, y, fill='black'))

y -= CASE_HEIGHT*1.5

caption.append(draw.Image(x,
                    y,
                    CASE_WIDTH,
                    CASE_HEIGHT,
                    "RSF.svg",
                    embed=True))

caption.append(draw.Rectangle(x,
                              y,
                              CASE_WIDTH, CASE_HEIGHT,
                              fill='none',
                              stroke="black",
                              stroke_width="0.2"))

caption.append(draw.Text("Run time error", HEADER_SIZE, x + 1.5*CASE_WIDTH, y, fill='black'))

caption.saveSvg('caption.svg')
