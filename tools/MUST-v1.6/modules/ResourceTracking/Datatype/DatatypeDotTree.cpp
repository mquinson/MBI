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
 * @file DatatypeDotTree.cpp
 *       @see MUST::DatatypeDotTree.
 *
 *  @date 17.10.2012
 *  @author Tobias Hilbrich, Joachim Protze
 */

#include <DatatypeDotTree.h>
#include <sstream>

DatatypeDotEdge::DatatypeDotEdge(
    const std::string& name, 
    DatatypeDotNode* start, 
    DatatypeDotNode* end, 
    int type,
    bool critical
) : critical(critical), name(name), start(start), end(end), type(type)
{
}

void DatatypeDotEdge::toString(std::ostream& out)
{
    if (!start->toPrint() || !end->toPrint())
        return;
    start->printName(out, type);
    out << "->";
    end->printName(out, type);
    out << "[label=\""<< name <<"\", style=solid";
    if (critical)
        out << ", color=red";
    out << "]" << std::endl;
}

bool DatatypeDotEdge::isCritical()
{
    return critical;
}

DatatypeDotNode::DatatypeDotNode(
    const std::string& name, 
    const std::string& text, 
    bool critical
): critical(critical), ref(1), name(name)
{
    texts.push_back(text);
}

void DatatypeDotNode::toString(std::ostream& out)
{
    if (texts.size()==1)
    {
        if (texts[0].empty())
            out << name << "[label=\"\", shape=box, width=0, height=0, style=invis];" << std::endl;
        else
            out << name << "[label=\"" << texts[0] << "\", shape=box];" << std::endl;
    }
    else
    {
        out << name << "[label=\"<t0>" << texts[0] << " | <t1> " << texts[1] << "\", shape=record];" << std::endl;
    }
}

bool DatatypeDotNode::addText(const std::string& text)
{
    ref++;
    if (text != texts[0]){
        texts.push_back(text);
        return true;
    }
    return false;
}

bool DatatypeDotNode::isCritical()
{
    return critical;
}

bool DatatypeDotNode::toPrint()
{
    // Node is to print, if in critical path, matched by both types or has a description
    return (critical || ref>1 || !texts[0].empty());
}

std::string& DatatypeDotNode::getName()
{
    return name;
}

void DatatypeDotNode::printName(std::ostream& out, int type)
{
    if (texts.size()==1)
        out << name;
    else
        out << name << ":t" << type;
}

bool DatatypeDotNode::reqEdge()
{
    return texts.size()==ref;
}


DatatypeForest::~DatatypeForest()
{
    std::vector<std::map<MustAddressType, DatatypeDotNode*> >::reverse_iterator vIter;
    std::map<MustAddressType, DatatypeDotNode*>::iterator mIter;
    for (vIter = nodes.rbegin(); vIter != nodes.rend(); vIter++)
    {
        for (mIter = vIter->begin(); mIter != vIter->end(); mIter++)
        {
            delete mIter->second;
        }
    }
    std::list<DatatypeDotEdge*>::iterator lIter;
    for (lIter = edges.begin(); lIter != edges.end(); lIter++)
    {
        delete *lIter;
    }
}

DatatypeDotNode* DatatypeForest::insertLeafNode(
    const std::string& text, 
    const MustAddressType& address
)
{
    if ( nodes.empty() )
    {
        nodes.push_back(std::map<MustAddressType, DatatypeDotNode*>());
    }
    std::map<MustAddressType, DatatypeDotNode*>::iterator iter = nodes[0].find(address);
    if ( iter == nodes[0].end() )
    {
        std::stringstream stream;
        stream << "l" << 0 << "x" << std::hex << address;
        std::string tmp = stream.str();
        iter = nodes[0].insert(std::make_pair(address, new DatatypeDotNode(tmp, text, true))).first;
    }
    else
    {
        iter->second->addText(text);
    }
    return (iter->second);
}

DatatypeDotNode* DatatypeForest::insertParentNode(
    int level, 
    DatatypeDotNode* child, 
    const std::string& nodeText, 
    const MustAddressType& address, 
    const std::string& edgeText,
    int type
)
{
    if ( nodes.size() <= level )
    {
        nodes.push_back(std::map<MustAddressType, DatatypeDotNode*>());
    }
    std::map<MustAddressType, DatatypeDotNode*>::iterator iter = nodes[level].find(address);
    if ( iter == nodes[level].end() )
    {
        std::stringstream stream;
        stream << "l" << level << "x" << std::hex << address;
        std::string tmp = stream.str();
        iter = nodes[level].insert(std::make_pair(address, new DatatypeDotNode(tmp, nodeText, child->isCritical()))).first;
    }
    else
    {
        if(! iter->second->addText(nodeText) && !child->reqEdge())
            return iter->second;
    }
    DatatypeDotNode* currentNode = iter->second;
    edges.push_back(new DatatypeDotEdge(edgeText, currentNode, child, type, child->isCritical()));
    return currentNode;
}

DatatypeDotNode* DatatypeForest::insertChildNode(
    int level, 
    DatatypeDotNode* parent, 
    const std::string& nodeText, 
    const MustAddressType& address, 
    const std::string& edgeText,
    int type
)
{
    std::map<MustAddressType, DatatypeDotNode*>::iterator iter = nodes[level].find(address);
    if ( iter == nodes[level].end() )
    {
        std::stringstream stream;
        stream << "l" << level << "x" << std::hex << address;
        std::string tmp = stream.str();
        iter = nodes[level].insert(std::make_pair(address, new DatatypeDotNode(tmp, nodeText))).first;
    }
    else
    {
        if(! iter->second->addText(nodeText) && !parent->reqEdge())
            return iter->second;
    }
    DatatypeDotNode* currentNode = iter->second;
    edges.push_back(new DatatypeDotEdge(edgeText, parent, currentNode, type));
    return currentNode;
}

void DatatypeForest::toString(std::ostream& out)
{
    out << "digraph Deadlock {" << std::endl
        << "graph [bgcolor=transparent]"<< std::endl << std::endl;

    std::vector<std::map<MustAddressType, DatatypeDotNode*> >::reverse_iterator vIter;
    std::map<MustAddressType, DatatypeDotNode*>::iterator mIter;
    for (vIter = nodes.rbegin(); vIter != nodes.rend(); vIter++)
    {
        int printed=0;
        std::string rankOrder;
        out << "{" << std::endl
            << "rank=same;" << std::endl;
        for (mIter = vIter->begin(); mIter != vIter->end(); mIter++)
        {
            if (mIter->second->toPrint())
            {
                mIter->second->toString(out);
                rankOrder += mIter->second->getName() + "->";
                printed++;
            }
        }
                  // remove the tailing ->
        if (printed>1)
        {
            out << rankOrder.substr(0, rankOrder.size()-2) << "[style=invis];" << std::endl;
        }
        out << "}" << std::endl;
    }
    std::list<DatatypeDotEdge*>::iterator lIter;
    for (lIter = edges.begin(); lIter != edges.end(); lIter++)
    {
        (*lIter)->toString(out);
    }
    out << "}" << std::endl;
}
