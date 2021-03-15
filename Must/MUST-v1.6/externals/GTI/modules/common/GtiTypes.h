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
 * @file GtiTypes.h
 *       Header file for global GTI types.
 *
 * @author Tobias Hilbrich
 */

#include <stdint.h>

#ifndef GTI_TYPES_H
#define GTI_TYPES_H

/**
 * This type defines the getInstance function implemented by all
 * modules.
 */
typedef int (*getInstance_t)(void* pOutInstance, const void *pInstanceName);

/**
 * This type defines the freeInstance function implemented by all
 * modules.
 */
typedef int (*freeInstance_t)(void* pInOutInstance);

/**
 * This type defines the freeInstance function implemented by all
 * modules.
 */
typedef int (*addDataHandler_t)(const void* pInstanceName, const void* pKey, const void* pValue);

/**
 * Type for arbitrary function pointer to be returned by GTI.
 */
typedef int (*GTI_Fct_t)(void*);

/**
 * This type defines the getFunction function that needs to be
 * implemented by wrappers.
 */
typedef int (*getFunction_t)(const char* functionName, GTI_Fct_t* pOutFunctionAddr);
typedef int (*getFunctionNoName_t)(GTI_Fct_t* pOutFunctionAddr);

/**
 * This type identifies a TBON node within a layer.
 * Its an Id within a range 0-[layerSize-1].
 * The id is used for connecting layers with each other and must be continuous
 * for correct operation of the connection workflows.
 */
typedef uint64_t GtiTbonNodeInLayerId;

#endif /*GTI_TYPES_H*/
