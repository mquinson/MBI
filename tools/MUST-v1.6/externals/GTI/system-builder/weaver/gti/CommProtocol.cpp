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
 * @file CommProtocol.cpp
 * 		@see gti::weaver::CommProtocol
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "CommProtocol.h"

using namespace gti::weaver::modules;

//=============================
// CommProtocol
//=============================
CommProtocol::CommProtocol (
			std::string moduleName,
			std::string configName,
			std::list<SettingsDescription*> settings,
			std::list<Module*> prependModules,
			std::list<std::string> apis,
			bool supportsIntraComm)
	: Module(moduleName, configName),
	  Configurable(settings),
	  Prepended(prependModules),
	  RequiresApi(apis),
	  mySupportsIntraComm (supportsIntraComm)
{
	/*nothing to do*/
}

//=============================
// CommProtocol
//=============================
CommProtocol::CommProtocol (
			std::string moduleName,
			std::string configName,
			std::string instanceType,
			std::string headerName,
			std::string incDir,
			std::list<SettingsDescription*> settings,
			std::list<Module*> prependModules,
			std::list<std::string> apis,
            bool supportsIntraComm)
	: Module(moduleName, configName, instanceType, headerName, incDir),
	  Configurable(settings),
	  Prepended(prependModules),
	  RequiresApi(apis),
	  mySupportsIntraComm (supportsIntraComm)
{
	/*nothing to do*/
}

//=============================
// ~CommProtocol
//=============================
CommProtocol::~CommProtocol ( )
{
	/*nothing to do*/
}

//=============================
// print
//=============================
std::ostream& CommProtocol::print (std::ostream& out) const
{
	out << "{";
	Module::print (out);
	out << ", ";
	Configurable::print (out);
	out << ", ";
	Prepended::print (out);
	out << ", ";
	RequiresApi::print (out);

	if (mySupportsIntraComm)
	    out << ", supportsIntraCommunication";

	out << "}";

	return out;
}

//=============================
// supportsIntraComm
//=============================
bool CommProtocol::supportsIntraComm (void)
{
    return mySupportsIntraComm;
}

/*EOF*/
