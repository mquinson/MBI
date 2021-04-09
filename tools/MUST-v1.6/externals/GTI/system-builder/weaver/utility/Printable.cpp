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
 * @file Printable.cpp
 * 		@see gti::weaver::Printable
 *
 * @author Tobias Hilbrich
 * @date 13.07.2010
 */

#include "Printable.h"

using namespace gti;
using namespace gti::weaver;

//=============================
// operator <<
//=============================
std::ostream& gti::weaver::operator << (std::ostream& out, const Printable& p)
{
	return p.print (out);
}
