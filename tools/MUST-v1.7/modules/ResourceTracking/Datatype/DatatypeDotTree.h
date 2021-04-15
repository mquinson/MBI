/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file DatatypeDotTree.h
 *       @see MUST::DatatypeDotTree.
 *
 *  @date 17.10.2012
 *  @author Tobias Hilbrich, Joachim Protze
 */



#include <list>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "MustTypes.h"

#ifndef DATATYPEDOTTREE_H
#define DATATYPEDOTTREE_H

/**
 * Class to represent a node in the dot tree to display datatype structures.
 *
 */
class DatatypeDotNode
{
private:
    bool critical;
    int ref;
    std::string name;
    std::vector<std::string> texts;
public:
    /**
     * Constructor
     *
     * @param name name of the node
     * @param text textual descriptions of the node
     * @param critical true if node is in the critical path.
     */
    DatatypeDotNode(const std::string& name, const std::string& text, bool critical=false );

    /**
     * generate dot output for the node
     *
     * @param out iostream to append the output 
     */
    void toString(std::ostream& out);

    /**
     * getter for the name of the node
     *
     * @return name of the node.
     */
    std::string& getName();

    /**
     * generate output of node name - for records append the subnode path
     *
     * @param out iostream to append the output 
     * @param type id of type (0/1)
     * @return true if successful.
     */
    void printName(std::ostream& out, int type);

    /**
     * Add text to node, if not already in the text list.
     *
     * @param text to add
     * @return true if added.
     */
    bool addText(const std::string& text);

    /**
     * Node in the critical path?
     *
     * @return true if node is in the critical path.
     */
    bool isCritical();

    /**
     * Will the node be printed?
     *
     * @return true if the node is to print.
     */
    bool toPrint();

    /**
     * is a second edge needed?
     *
     * @return true if number of connected nodes equals to number of descriptions.
     */
    bool reqEdge();

};

/**
 * Class to represent an edge in the dot tree to display datatype structures.
 *
 */
class DatatypeDotEdge
{
private:
    bool critical;
    std::string name;
    DatatypeDotNode *start, *end;
    int type;
public:

    /**
     * Constructor
     *
     * @param name description of the edge
     * @param start pointer of starting node
     * @param end pointer of end node
     * @param type id of the datatype (0/1)
     * @param critical true if edge is in the critical path.
     */
    DatatypeDotEdge(const std::string& name, DatatypeDotNode* start, DatatypeDotNode* end, int type, bool critical=false);

    /**
     * generate dot output for the edge
     *
     * @param out iostream to append the output 
     */
    void toString(std::ostream& out);

    /**
     * Edge in the critical path?
     *
     * @return true if edge is in the critical path.
     */
    bool isCritical();

};

/**
 * Class to generate dot tree to display datatype structures.
 * 
 * 
 *
 */
class DatatypeForest
{
private:
    std::vector<std::map<MustAddressType, DatatypeDotNode *> > nodes;
    std::list<DatatypeDotEdge *> edges;
public:

    /**
     * Constructor
     */
    DatatypeForest():nodes(),edges() {}

    /**
     * Destructor
     */
    ~DatatypeForest();

    /**
     * Start with inserting the leafnode of the critical path;
     *
     * @param text textual description of the node
     * @param address is used as key for the node matching + name of the node.
     */
    DatatypeDotNode* insertLeafNode(const std::string& text, const MustAddressType& address);

    /**
     * Backtrack the critical path and insert the parent node
     *
     * @param level tree level of the node (leaf starts with 0, root has highest level)
     * @param child pointer to the last inserted critical node
     * @param nodeText textual description of the node
     * @param address is used as key for the node matching + name of the node.
     * @param edgeText textual description of the edge
     * @param type id of the type
     */
    DatatypeDotNode* insertParentNode(int level, DatatypeDotNode* child, const std::string& nodeText, const MustAddressType& address , const std::string& edgeText, int type);

    /**
     * While backtracking, add siblings to the critical node
     *
     * @param level tree level of the node (leaf starts with 0, root has highest level)
     * @param parent pointer to the last inserted critical node
     * @param nodeText textual description of the node
     * @param address is used as key for the node matching + name of the node.
     * @param edgeText textual description of the edge
     * @param type id of the type
     */
    DatatypeDotNode* insertChildNode(int level, DatatypeDotNode* parent, const std::string& nodeText, const MustAddressType& address , const std::string& edgeText, int type);

    /**
     * generate dot output of the graph
     *
     * @param out iostream to append the output 
     */
    void toString(std::ostream& out);
};

#endif