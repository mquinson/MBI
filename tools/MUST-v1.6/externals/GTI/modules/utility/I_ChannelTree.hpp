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
 * @file I_ChannelTree.hpp
 *       @see gti::I_ChannelTree
 *
 * @author Tobias Hilbrich
 * @date 17.12.2009
 */

//=============================
// I_ChannelTree
//=============================
template <class IMPL>
I_ChannelTree<IMPL>::I_ChannelTree (int subIdIndex, long numChannels)
 : myChilds (), mySubIdIndex (subIdIndex), myNumChannels(numChannels)
{
	/*Nothing to do*/
}

//=============================
// ~I_ChannelTree
//=============================
template <class IMPL>
I_ChannelTree<IMPL>::~I_ChannelTree (void)
{
	typename std::map<long int, IMPL*>::iterator iter;
	for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
	{
		if (iter->second)
			delete (iter->second);
	}
	myChilds.clear();
}

//=============================
// printAsDot
//=============================
template <class IMPL>
std::ostream& I_ChannelTree<IMPL>::printAsDot (std::ostream& out, std::string nodeNamePrefix)
{
	bool isRoot = false;
	if (nodeNamePrefix == "")
	{
		isRoot = true;
		nodeNamePrefix = "root";

		out
		<< "digraph channelTree" << std::endl
		<< "{" << std::endl;
	}

	//print this node
	std::string dotName = nodeNamePrefix;
	out << "    " << dotName << " [label=\"{{" << getNodeName() << "}";

	std::string extra = getNodeExtraRows ();
	if (extra != "")
		out << "|" << extra;

	out << "}\", shape=record, style=filled, color=" << getNodeColor() << "];" << std::endl;

	//Loop over children
	typename std::map<long int, IMPL*>::iterator iter;
	for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
	{
		//recurse into child
		IMPL* child = iter->second;
		long int channel = iter->first;

		char str[256];
		sprintf (str, "%s_%ld", dotName.c_str(), channel);
		child->printAsDot(out, str);

		//print connection of this node to child
		out << "    " << dotName << "->" << str << " [label=\"" << channel << "\"];" << std::endl;
	}

	if (isRoot)
	{
		out
		<< "}" << std::endl;
	}

	return out;
}

//=============================
// getNodeName
//=============================
template <class IMPL>
std::string I_ChannelTree<IMPL>::getNodeName (void)
{
	char temp[256];
	sprintf (temp, "Index: %d Size: %ld",mySubIdIndex, myNumChannels);
	return temp;
}

//=============================
// getNodeColor
//=============================
template <class IMPL>
std::string I_ChannelTree<IMPL>::getNodeColor (void)
{
	return "green";
}

//=============================
// getNodeExtraRows
//=============================
template <class IMPL>
std::string I_ChannelTree<IMPL>::getNodeExtraRows (void)
{
	return "";
}

//=============================
// getChannelForId
//=============================
template <class IMPL>
long int I_ChannelTree<IMPL>::getChannelForId (I_ChannelId* channelId)
{
	if (mySubIdIndex < 0)
		return -1;

	if (mySubIdIndex >= channelId->getNumUsedSubIds())
		return -2;

	return channelId->getSubId (mySubIdIndex);
}

//=============================
// getChildForChannel
//=============================
template <class IMPL>
IMPL* I_ChannelTree<IMPL>::getChildForChannel (I_ChannelId *channelId)
{
	long int channel = getChannelForId (channelId);

	if (channel == -2)
		return NULL;

	if (channel == -1)
		return (IMPL*)this;

	IMPL* ret;

	typename std::map<long int, IMPL*>::iterator pos = myChilds.find (channel);

	if (pos == myChilds.end ())
	{
		int subSubId = mySubIdIndex -1;
		long numSubChannels = 0;
		if (subSubId >= 0)
			numSubChannels = channelId->getSubIdNumChannels(subSubId);
		ret = allocateChild (subSubId, numSubChannels);
		myChilds.insert (std::make_pair(channel, ret));
	}
	else
	{
		ret = pos->second;
	}

	return ret;
}

//=============================
// allocateChild
//=============================
template <class IMPL>
IMPL* I_ChannelTree<IMPL>::allocateChild (int subIdIndex, long numChannels)
{
	return new IMPL (subIdIndex, numChannels);
}


/*EOF*/
