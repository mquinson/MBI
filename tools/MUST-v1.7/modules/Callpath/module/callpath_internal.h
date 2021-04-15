/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#ifndef PNMPI_CALLPATH_INTERNAL_H
#define PNMPI_CALLPATH_INTERNAL_H

#include "GtiTLS.h"

///\file callpath_internal.h
/// This is an internal header used for variables that are shared
/// between the callpath module and the wrappers.

/// Callpath variable is defined in the main module cpp file.
extern thread_local Callpath PNMPIMOD_Callpath_callpath;

#endif // PNMPI_CALLPATH_INTERNAL_H
