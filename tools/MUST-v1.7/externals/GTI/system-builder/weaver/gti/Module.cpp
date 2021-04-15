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
 * @file Module.cpp
 * 		@see gti::weaver::Module
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "Module.h"
#include <iostream>

using namespace gti::weaver::modules;

//=============================
// Module
//=============================
Module::Module ( )
	: myModuleName(""), myConfigName(""), myInstanceType(""), myHeaderName(""), myIncDir("")
{
	/*Nothing to do*/
}

//=============================
// Module
//=============================
Module::Module (
		std::string moduleName,
		std::string configName
)
	: myModuleName(moduleName), myConfigName(configName), myInstanceType(""), myHeaderName(""), myIncDir("")
{
	/*Nothing to do*/
}

//=============================
// Module
//=============================
Module::Module (
		std::string moduleName,
		std::string configName,
		std::string instanceType,
		std::string headerName,
		std::string incDir
)
	:  myModuleName(moduleName), myConfigName(configName), myInstanceType(instanceType), myHeaderName(headerName), myIncDir(incDir)
{
	/*Nothing to do*/
}

//=============================
// Module
//=============================
Module::~Module (void)
{
	/*Nothing to do*/
}

//=============================
// setModuleName
//=============================
void Module::setModuleName ( std::string new_var )
{
	myModuleName = new_var;
}

//=============================
// getModuleName
//=============================
std::string Module::getModuleName (void)
{
	return myModuleName;
}

//=============================
// setConfigName
//=============================
void Module::setConfigName ( std::string new_var )
{
	myConfigName = new_var;
}

//=============================
// getConfigName
//=============================
std::string Module::getConfigName (void)
{
	return myConfigName;
}

//=============================
// setInstanceType
//=============================
void Module::setInstanceType (std::string instanceType)
{
	myInstanceType = instanceType;
}

//=============================
// getInstanceType
//=============================
std::string Module::getInstanceType (void)
{
	return myInstanceType;
}

//=============================
// setHeaderName
//=============================
void Module::setHeaderName (std::string headerName)
{
	myHeaderName = headerName;
}

//=============================
// getHeaderName
//=============================
std::string Module::getHeaderName (void)
{
	return myHeaderName;
}

//=============================
// setIncDir
//=============================
void Module::setIncDir (std::string incDir)
{
	myIncDir = incDir;
}

//=============================
// getIncDir
//=============================
std::string Module::getIncDir (void)
{
	return myIncDir;
}

//=============================
// print
//=============================
std::ostream& Module::print (std::ostream& out) const
{
	out << "registeredName=" << myModuleName << ", configName=" << myConfigName;

	if (myInstanceType != "")
		out << ", instanceType=" << myInstanceType;

	if (myHeaderName != "")
		out << ", header=" << myHeaderName;

	if (myIncDir != "")
		out << ", includeDir=" << myIncDir;

	return out;
}

/*EOF*/
