/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file ELPSubModule.cpp
 *       @see MUST::ELPSubModule.
 *
 *  @date 22.07.2014
 *  @author Felix Muenchhalfen
 */

#include "ModuleBase.h"
#include "I_ELPSubModule.h"

#ifndef ELPSUBMODULE_H
#define	ELPSUBMODULE_H

using namespace gti;

namespace must
{

class ELPSubModule : public gti::ModuleBase<ELPSubModule, I_ELPSubModule> {
public:
    ELPSubModule(const char* instanceName);
    virtual ~ELPSubModule();
private:

};

}

#endif	/* ELPSUBMODULE_H */

