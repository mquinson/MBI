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
 * @file RequiresApi.cpp
 * 		@see gti::weaver::RequiresApi
 *
 * @author Tobias Hilbrich
 * @date 14.01.2010
 */

#include <assert.h>
#include <iostream>

#include "RequiresApi.h"

using namespace gti::weaver::modules;

//=============================
// RequiresApi
//=============================
RequiresApi::RequiresApi ()
	: myApis()
{

}

//=============================
// RequiresApi
//=============================
RequiresApi::RequiresApi (std::list<std::string> apis)
	: myApis()
{
	std::list<std::string>::iterator i;
	for (i = apis.begin(); i != apis.end(); i++)
	{
		myApis.push_back (*i);
	}
}

//=============================
// ~RequiresApi
//=============================
RequiresApi::~RequiresApi ( )
{

}

//=============================
// addApi
//=============================
void RequiresApi::addApi (std::string api)
{
	myApis.push_back (api);
}

//=============================
// removeApi
//=============================
void RequiresApi::removeApi (std::string api)
{
	std::list<std::string>::iterator i;
	for (i = myApis.begin(); i != myApis.end(); i++)
	{
		if (*i == api)
		{
			myApis.erase(i);
			break;
		}
	}
}

//=============================
// getRequiredApis
//=============================
std::list<std::string> RequiresApi::getRequiredApis (void)
{
	return myApis;
}

//=============================
// checkRequiredApis
//=============================
bool RequiresApi::checkRequiredApi (std::string api) const
{
  for(std::list<std::string>::const_iterator i = myApis.begin();
			 i != myApis.end(); i++)
    if (*i == api)
      return true;
	return false;
}

//=============================
// print
//=============================
std::ostream& RequiresApi::print (std::ostream& out) const
{
	if (myApis.size() > 0)
	{
		out << "requiredAPIs={";
		for (std::list<std::string>::const_iterator i = myApis.begin();
			 i != myApis.end(); i++)
		{
			if (i != myApis.begin())
				out << ", ";

			out << *i;
		}
		out << "}";
	}
	else
	{
		out << "noRequiredAPIs";
	}
	return out;
}

/*EOF*/
