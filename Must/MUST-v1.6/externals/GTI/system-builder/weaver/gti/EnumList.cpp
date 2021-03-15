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
 * @file EnumList.cpp
 * 		@see gti::weaver::EnumList
 *
 * @author Tobias Hilbrich
 * @date 7.7.2010
 */

#include "EnumList.h"

using namespace gti::weaver::modules;
using namespace gti;

//*****************************
// EnumList
//*****************************
EnumList::EnumList (void)
 : myId (-1), myEntries ()
{
	//Nothing to do
}

//*****************************
// EnumList
//*****************************
EnumList::EnumList (int id)
 : myId (id), myEntries ()
{
	//Nothing to do
}

//*****************************
// EnumList
//*****************************
EnumList::EnumList (int id, std::list<std::string> entries)
 : myId (id), myEntries (entries)
{
	//Nothing to do
}

//*****************************
// getId
//*****************************
int EnumList::getId (void)
{
	return myId;
}

//*****************************
// setId
//*****************************
void EnumList::setId (int id)
{
	myId = id;
}

//*****************************
// getEntries
//*****************************
std::list<std::string> EnumList::getEntries (void)
{
	return myEntries;
}

//*****************************
// getFirstEntry
//*****************************
std::string EnumList::getFirstEntry (void)
{
	return myEntries.front ();
}

//*****************************
// isValidEntry
//*****************************
bool EnumList::isValidEntry (std::string entry)
{
	if (this->findEntry (entry) != myEntries.end())
		return true;

	return false;
}

//*****************************
// addEntry
//*****************************
void EnumList::addEntry (std::string entry)
{
	myEntries.push_back (entry);
}

//*****************************
// removeEntry
//*****************************
bool EnumList::removeEntry (std::string entry)
{
	std::list<std::string>::iterator i = this->findEntry (entry);
	if (i == myEntries.end())
		return false;

	myEntries.erase (i);
	return true;
}

//*****************************
// findEntry
//*****************************
std::list<std::string>::iterator EnumList::findEntry (std::string entry)
{
	std::list<std::string>::iterator ret;
	for (ret = myEntries.begin (); ret != myEntries.end(); ret++)
	{
		if (*ret == entry)
			break;
	}

	return ret;
}

//*****************************
// print
//*****************************
std::ostream& EnumList::print (std::ostream& out) const
{
	out << "id=" << myId << " entries=";

	std::list<std::string>::const_iterator ret;
	for (ret = myEntries.begin (); ret != myEntries.end(); ret++)
	{
		out << *ret << ", ";
	}

	return out;
}

/*EOF*/
