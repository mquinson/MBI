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
 * @file CommStrategyIntra.cpp
 *      @see gti::weaver::CommStrategyIntra
 *
 * @author Tobias Hilbrich
 * @date 18.01.2012
 */

#include "CommStrategyIntra.h"

using namespace gti::weaver::modules;

//=============================
// CommStrategyIntra
//=============================
CommStrategyIntra::CommStrategyIntra ( )
    : Configurable ()
{
    /*Nothing to do*/
}

//=============================
// CommStrategyIntra
//=============================
CommStrategyIntra::CommStrategyIntra (
        std::string moduleName,
        std::string configName,
        std::list<SettingsDescription*> settings)
    : Configurable (settings), Module (moduleName,configName)
{
    /*Nothing to do*/
}

//=============================
// CommStrategyIntra
//=============================
CommStrategyIntra::CommStrategyIntra (
        std::string moduleName,
        std::string configName,
        std::string instanceType,
        std::string headerName,
        std::string incDir,
        std::list<SettingsDescription*> settings)
    : Configurable (settings),
      Module (moduleName,configName,instanceType,headerName,incDir)
{
    /*Nothing to do*/
}


//=============================
// CommStrategyIntra
//=============================
CommStrategyIntra::~CommStrategyIntra ( )
{
    /*Nothing to do*/
}

//=============================
// print
//=============================
std::ostream& CommStrategyIntra::print (std::ostream& out) const
{
    out << "module={";
    Module::print (out);
    out << "}, ";
    Configurable::print (out);

    return out;
}

/*EOF*/
