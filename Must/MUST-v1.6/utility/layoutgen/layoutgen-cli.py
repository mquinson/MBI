#!/usr/bin/python

# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2011-2014 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2011-2014 Lawrence Livermore National Laboratories, United States of America
#   2013-2014 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

#import pygtk
#pygtk.require('2.0')
#import gtk

import math
import sys

#
# The generation of a MUST layout is based on a template layout.
# The template is supposed to have 3 layers:
#  - application layer with analyses mapped on it
#  - 1st non-application layer
#  - root layer
# Analyses mapped on the root layer will be moved to the new root
# Analyses mapped on the 1st non-application layer stay there
#
# For new layers an empty communication entry is generated, supposed to use the
# default communication configuration. 
#




#
# import the etree module either from system or from subdirectory elementtree
#

try:
  from lxml import etree
  print("running with lxml.etree")
except ImportError:
  try:
    # Python 2.5
    import xml.etree.cElementTree as etree
    print("running with cElementTree on Python 2.5+")
  except ImportError:
    try:
      # Python 2.5
      import xml.etree.ElementTree as etree
      print("running with ElementTree on Python 2.5+")
    except ImportError:
      try:
        # normal cElementTree install
        import cElementTree as etree
        print("running with cElementTree")
      except ImportError:
        try:
            # normal ElementTree install
            # TODO: install the elementtree directory and configure the path by cmake
            #sys.path+=[@CMAKE_INSTALL_PREFIX@]
            import elementtree.ElementTree as etree
            print("running with ElementTree")
        except ImportError:
            print("Failed to import ElementTree from any known place")

#
# parse the commandline arguments
#
try:
    #raise ImportError
    import argparse

    parser = argparse.ArgumentParser(description='Manipulate the tree of a MUST layout.')
    parser.add_argument('--in', '-i', dest='infile', metavar='template-file', type=str, help='inputfile', default='layout.xml', action='store')
    parser.add_argument('--out', '-o', dest='outfile', action='store', default='layout-new.xml',
                    help='outputfile', type=str)
    parser.add_argument('--np', '-n', dest='np', action='store', default='4',
                    help='number of application processes', type=int)
    parser.add_argument('--max-fan-in', '--fan-in', '-f', dest='fan', action='store', default='2',
                    help='aka branching factor of the tree', type=int)
    parser.add_argument('--nodesize', "-s", dest='nodesize', action='store', default='0',
                    help='nodesize of the machine: nodesize-1 application ranks + 1 "1st non-application layer" rank are distributed to one node, fan-in is ignored for this layer', type=int)
    args = parser.parse_args()
except ImportError:
    try:
        from optparse import OptionParser
        parser = OptionParser()
        parser.add_option('--in', '-i', dest='infile', metavar='template-file', type="string", help='inputfile', default='layout.xml', action='store')
        parser.add_option('--out', '-o', dest='outfile', action='store', default='layout-new.xml',
                        help='outputfile', type="string")
        parser.add_option('--np', '-n', dest='np', action='store', default='4',
                        help='number of application processes', type="int")
        parser.add_option('--max-fan-in', '--fan-in', '-f', dest='fan', action='store', default='2',
                        help='aka branching factor of the tree', type="int")
        parser.add_option('--nodesize', "-s", dest='nodesize', action='store', default='0',
                        help='nodesize of the machine: nodesize-1 application ranks + 1 "1st non-application layer" rank are distributed to one node, fan-in is ignored for this layer', type="int")
        (args, bonus_args) = parser.parse_args()
    except ImportError:
        print("Failed to import argparse and optparse library")

#
# model of the xml tree of a layout file
#

class LayoutTree:
    def __init__(self,arg):
        self.tree = etree.parse(arg)
        self.root = self.tree.getroot()
        self.levels = self.root.iter("levels").next()
        self.communications = self.root.iter("communications").next()
        self.update_levelnames()

    def update_levelnames(self):
        """updates the list of level ids"""
        self.levelnames=[]
        for l in self.root.iter("level"):
            self.levelnames+=[l.attrib["order"]]

    def get_analyses_for_level(self, level):
        """returns an iteratable list of analysis nodes"""
        for l in self.root.iter("level"):
            if l.attrib["order"]==str(level):
                return l.iter("analysis")


class Model:
    def write(self,arg):
        """write to an xml file"""
        self.tree.tree.write(arg)


    def add_analysis_to_level(self, analysis, level):
        for l in self.tree.levels.iter("level"):
            if l.attrib["order"]==str(level):
                l[0].append(etree.Element("analysis",name=analysis[0],group=analysis[1]))
                return

    def remove_analysis_from_level(self, analysis, level):
        for l in self.tree.levels.iter("level"):
            if l.attrib["order"]==str(level):
                for a in l.iter("analysis"):
                    if a.attrib["name"]==analysis[0]:
                        l[0].remove(a)
                        return
                return

    def add_level(self):
        """append a new level"""
        index=len(self.tree.levels)
        while str(index) in self.tree.levelnames:
            index+=1
        etree.SubElement(etree.SubElement(self.tree.levels, "level", {"order":str(index), "size":"1", "place-name":"mpi_place"}), "analyses")
        self.tree.update_levelnames()
        #<connection top-level="1" bottom-level="0"></connection>
        etree.SubElement(self.tree.communications, "connection", {"top-level":str(index), "bottom-level":str(index-1)})

    def del_level(self):
        """delete last level"""
        index=len(self.tree.levels)
        self.tree.levels.remove(self.tree.levels[index-1])
        self.tree.update_levelnames()

    def update_level(self, level, attribs):
        """write attribute values to level, attribs is expected to be a dict()"""
        for l in self.tree.levels.iter("level"):
            if l.attrib["order"]==str(level):
                for k,v in attribs.iteritems():
                    if v==None:
                        if k in l.attrib:
                            del l.attrib[k]
                    else:
                        l.attrib[k]=v
                break
        self.tree.update_levelnames()

    def update_connection(self, levels, attribs):
        """write attribute values to level, attribs is expected to be a dict()"""
        for l in self.tree.communications.iter("connection"):
            if l.attrib["top-level"]==str(levels[0]) and l.attrib["bottom-level"]==str(levels[1]):
                for k,v in attribs.iteritems():
                    if v==None:
                        if k in l.attrib:
                            del l.attrib[k]
                    else:
                        l.attrib[k]=v
                break
        self.tree.update_levelnames()

    def get_level_attribs(self, level):
        """get attribs dict for level"""
        for l in self.tree.levels.iter("level"):
            if l.attrib["order"]==str(level):
                return l.attrib

    def get_analyses_for_level(self, level):
        """returns an iteratable list of analysis nodes"""
        for analysis in list(self.tree.get_analyses_for_level(level)):
            yield (analysis.attrib["name"], analysis.attrib["group"])

    def move_analyses_to_level(self, old_level, new_level):
        """moves all analyses from old_level to new_level"""
        for analysis in self.get_analyses_for_level(old_level):
            self.add_analysis_to_level(analysis, new_level)
            self.remove_analysis_from_level(analysis, old_level)

    def __init__(self,arg):
        self.tree = LayoutTree(arg)


if __name__ == "__main__":
    sum_size=0;
    model = Model(args.infile)
    if args.nodesize>0:
        levels = int(math.log(1.0*args.np/(args.nodesize-1)) / math.log(args.fan) +2.9999)
        levelsizes = [args.np] + [int((1.0*args.np/(args.nodesize-1)/args.fan**i)+.9999) for i in range(levels-1)]
        model.update_connection((1,0), {"distribution":"by-block","blocksize":str(args.nodesize-1)})
    else:
        levels = int(math.log(args.np) / math.log(args.fan) +1.9999)
        levelsizes = [int((1.0*args.np/args.fan**i)+.9999) for i in range(levels)]
        model.update_connection((1,0), {"distribution":"uniform","blocksize":None})
    for i in range(3,levels):
        model.add_level()
    for n,i in enumerate(levelsizes):
        model.update_level(n, {"size":str(i)})
        sum_size+=i
    if levels>3:
        model.move_analyses_to_level(2,levels-1)
    model.write(args.outfile)
    print "%i ranks are needed for this layout." % sum_size

