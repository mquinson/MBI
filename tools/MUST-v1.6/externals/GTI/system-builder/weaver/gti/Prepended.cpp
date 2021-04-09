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
 * @file Prepended.cpp
 * 		@see gti::weaver::Prepended
 *
 * @author Tobias Hilbrich
 * @date 14.01.2010
 */

#include <assert.h>

#include "Prepended.h"

using namespace gti::weaver::modules;

//=============================
// Prepended
//=============================
Prepended::Prepended ( )
	: myPrependModules ()
{
	/*Nothing to do*/
}

//=============================
// Prepended
//=============================
Prepended::Prepended (std::list<Module*> prependModules)
	: myPrependModules()
{
	std::list<Module*>::iterator i;
	for (i = prependModules.begin(); i != prependModules.end(); i++)
	{
		myPrependModules.push_back (*i);
	}
}

//=============================
// ~Prepended
//=============================
Prepended::~Prepended ( )
{
	std::list<Module*>::iterator i;
	for (i = myPrependModules.begin(); i != myPrependModules.end(); i++)
	{
		Module *p = *i;

		if (p)
			delete (p);
	}

	myPrependModules.clear();
}

//=============================
// addPrependModule
//=============================
void Prepended::addPrependModule ( Module* add_object )
{
	myPrependModules.push_back(add_object);
}

//=============================
// removePrependModule
//=============================
void Prepended::removePrependModule ( Module* remove_object )
{
	if (!remove_object)
		return;

	std::list<Module*>::iterator i;
	for (i = myPrependModules.begin(); i != myPrependModules.end(); i++)
	{
		if (*i == remove_object)
			delete (*i);

		myPrependModules.erase(i);
		break;
	}

	return;
}

//=============================
// getPrependedModules
//=============================
std::list<Module*> Prepended::getPrependedModules (void)
{
	return myPrependModules;
}

//=============================
// print
//=============================
std::ostream& Prepended::print (std::ostream& out) const
{
	if (myPrependModules.size() > 0)
	{
		out << "prependModules={";
		for (std::list<Module*>::const_iterator i = myPrependModules.begin();
			 i != myPrependModules.end();
			 i++)
		{
			if (i != myPrependModules.begin())
				out << ", ";

			out << **i;
		}

		out << "}";
	}
	else
	{
		out << "noPrependModules";
	}
	return out;
}

/*EOF*/
