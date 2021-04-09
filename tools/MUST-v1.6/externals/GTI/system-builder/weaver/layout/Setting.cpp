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
 * @file Setting.cpp
 * 		@see gti::weaver::Setting
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "Setting.h"

using namespace gti::weaver::layout;

//=============================
// Setting
//=============================
Setting::Setting ( )
 : myName (""),
   myValue ("")
{
	/*Nothing to do*/
}

//=============================
// Setting
//=============================
Setting::Setting (std::string name, std::string value)
 : myName (name),
   myValue (value)
{
	/*Nothing to do*/
}

//=============================
// ~Setting
//=============================
Setting::~Setting ( )
{
	/*Nothing to do*/
}

//=============================
// setName
//=============================
void Setting::setName ( std::string new_var )
{
	myName = new_var;
}

//=============================
// getName
//=============================
std::string Setting::getName ( )
{
	return myName;
}

//=============================
// seValue
//=============================
void Setting::seValue ( std::string new_var )
{
	myValue = new_var;
}

//=============================
// getValue
//=============================
std::string Setting::getValue ( )
{
	return myValue;
}

//=============================
// print
//=============================
std::ostream& Setting::print (std::ostream& out) const
{
	out
		<< "setting={name="
		<< myName << ", "
		<< "value=" << myValue
		<< "}";

	return out;
}

//=============================
// clone
//=============================
Setting* Setting::clone (void)
{
	return new Setting (myName, myValue);
}

/*EOF*/
