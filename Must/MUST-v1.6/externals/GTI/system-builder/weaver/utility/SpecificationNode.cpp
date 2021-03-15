/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file SpecificationNode.cpp
 * 		@see gti::weaver::SpecificationNode
 *
 * @author Tobias Hilbrich
 * @date 08.07.2010
 */

#include "SpecificationNode.h"

#include <iostream>
#include <assert.h>

using namespace gti::weaver;

//=============================
// Constructor
//=============================
SpecificationNode::SpecificationNode ( )
 : myNode (NULL)
{
	/*Nothing to do*/
}

//=============================
// Constructor
//=============================
SpecificationNode::SpecificationNode (xmlNodePtr node)
 : myNode (node)
{
	/*Nothing to do*/
}

//=============================
// operator =
//=============================
SpecificationNode& SpecificationNode::operator = (xmlNodePtr node)
{
	myNode = node;
	return *this;
}

//=============================
// operator ==
//=============================
bool SpecificationNode::operator == (const xmlNodePtr& node) const
{
	return (myNode == node);
}

//=============================
// operator xmlNodePtr
//=============================
SpecificationNode::operator xmlNodePtr() const
{
	return myNode;
}

//=============================
// findChildNodeNamed
//=============================
SpecificationNode SpecificationNode::findChildNodeNamed (std::string name)
{
	if (!myNode)
		return NULL;

	xmlNodePtr iter = myNode->xmlChildrenNode;

	while (iter != NULL)
	{
		//Is it the looked for node ?
		if (!xmlStrcmp(iter->name, (const xmlChar *) name.c_str()))
		{
			return (iter);
		}

		iter = iter->next;
	}

	return NULL;
}

//=============================
// findChildNodeNamed
//=============================
SpecificationNode SpecificationNode::findChildNodeNamed (std::list<std::string> names, std::string *pOutName)
{
	if (!myNode)
		return NULL;

	xmlNodePtr iter = myNode->xmlChildrenNode;

	while (iter != NULL)
	{
		//Is it the looked for node ?
		std::list<std::string>::iterator nameIter;

		for (nameIter = names.begin(); nameIter != names.end(); nameIter++)
		{
			if (!xmlStrcmp(iter->name, (const xmlChar *) nameIter->c_str()))
			{
				if (pOutName)
					*pOutName = *nameIter;
				return (iter);
			}
		}

		iter = iter->next;
	}

	return NULL;
}

//=============================
// findChildNodeNamedOrErr
//=============================
SpecificationNode SpecificationNode::findChildNodeNamedOrErr (std::string name, std::string error)
{
	SpecificationNode ret = findChildNodeNamed(name);

	if (!ret)
		std::cerr << error << std::endl;

	return ret;
}

//=============================
// findSiblingNamed
//=============================
SpecificationNode SpecificationNode::findSiblingNamed (std::string name)
{
	if (!myNode)
		return NULL;

	xmlNodePtr iter = myNode->next;

	while (iter != NULL)
	{
		//Is it the looked for node ?
		if (!xmlStrcmp(iter->name, (const xmlChar *) name.c_str()))
		{
			return (iter);
		}

		iter = iter->next;
	}

	return NULL;
}

//=============================
// findSiblingNamed
//=============================
SpecificationNode SpecificationNode::findSiblingNamed (std::list<std::string> names, std::string *pOutName)
{
	if (!myNode)
		return NULL;

	xmlNodePtr iter = myNode->next;

	while (iter != NULL)
	{
		//Is it the looked for node ?
		std::list<std::string>::iterator nameIter;

		for (nameIter = names.begin(); nameIter != names.end(); nameIter++)
		{
			if (!xmlStrcmp(iter->name, (const xmlChar *) nameIter->c_str()))
			{
				if (pOutName)
					*pOutName = *nameIter;
				return (iter);
			}
		}

		iter = iter->next;
	}

	return NULL;
}

//=============================
// findSiblingNamedOrErr
//=============================
SpecificationNode SpecificationNode::findSiblingNamedOrErr (std::string name, std::string error)
{
	SpecificationNode ret = findSiblingNamed(name);

	if (!ret)
		std::cerr << error << std::endl;

	return ret;
}

//=============================
// getAttribute
//=============================
bool SpecificationNode::getAttribute (std::string name, std::string *pOutContent)
{
	if (!myNode)
		return false;

	assert (pOutContent);

	xmlChar* temp = xmlGetProp(myNode, (const xmlChar*) name.c_str());
	if (temp)
	{
		*pOutContent = reinterpret_cast<char*>(temp);
		xmlFree(temp);
		return true;
	}

	return false;
}

//=============================
// getAttributeOrErr
//=============================
bool SpecificationNode::getAttributeOrErr (std::string name, std::string error, std::string *pOutContent)
{
	if (!getAttribute (name, pOutContent))
	{
		std::cerr << error << std::endl;
		return false;
	}

	return true;
}

//=============================
// getNodeContent
//=============================
std::string SpecificationNode::getNodeContent (void)
{
	std::string ret = "";
	size_t p;

	xmlChar* content = xmlNodeListGetString(myNode->doc, myNode->xmlChildrenNode, 1);

	if (content)
	{
		ret = reinterpret_cast<char*>(content);
		xmlFree (content);
	}

	//prune leading " ", "\n", "\t"
	p = ret.find_first_not_of (" \n\t");
	ret.erase (0,p);

	//prune trailing " ", "\n", "\t"
	p = ret.find_last_not_of (" \n\t");
	ret.erase (p + 1,ret.length() - p);

	return ret;
}

//=============================
// operator ()
//=============================
xmlNodePtr SpecificationNode::operator () (void)
{
	return myNode;
}

//=============================
// textToXmlText
//=============================
std::string SpecificationNode::textToXmlText (std::string text)
{
	std::string finds[] = {"&", "<", ">"};
	std::string reps[]  = {"&amp;", "&lt;", "&gt;"};
	const int numReps = 3;

	for (int i = 0; i < numReps; i++)
	{
		size_t pos = text.find(finds[i]);
		while (pos != std::string::npos)
		{
			text.replace (pos, finds[i].length(), reps[i]);

			//next
			pos = text.find(finds[i], pos+reps[i].length());
		}
	}

	return text;
}

/*EOF*/
