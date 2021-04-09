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
 * @file Verbose.cpp
 * 		@see gti::Verbose
 *
 * @author Tobias Hilbrich
 * @date 06.07.2010
 */

#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "Verbose.h"

using namespace gti::weaver;

Verbose* Verbose::ourInstance = NULL;

//=============================
// getInstance
//=============================
Verbose* Verbose::getInstance (void)
{
	if (ourInstance == NULL)
	{
		ourInstance = new Verbose ();
		assert (ourInstance);
	}

	return ourInstance;
}

//=============================
// getStream
//=============================
std::ostream& Verbose::getStream (int verbosity)
{
	assert (verbosity >= 0); //no negative verbosities ...

	if (verbosity > myVerbosity)
		return *myNull;

	return *myStream;
}

//=============================
// Verbose
//=============================
Verbose::Verbose (void)
{
	//query environmental for verbosity
	if (getenv("GTI_VERBOSE") != NULL)
	{
		myVerbosity = atoi(getenv("GTI_VERBOSE"));

		if (myVerbosity < 0)
		{
			myVerbosity = 0;
		}
	}
	else
	{
		myVerbosity = 0;
	}

	myNull = new std::ofstream ("/dev/null");
	myStream = &std::cout;
}

//=============================
// ~Verbose
//=============================
Verbose::~Verbose ()
{
	if (myNull)
	{
		myNull->clear();
		myNull->close();
		delete (myNull);
	}
	myNull = NULL;

	if (myStream)
		myStream->flush ();
	myStream = NULL;

	myVerbosity = 0;
}

//=============================
// getLevel
//=============================
int Verbose::getLevel (void)
{
	return myVerbosity;
}

/*EOF*/
