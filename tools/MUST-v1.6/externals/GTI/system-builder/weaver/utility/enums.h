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
 * @file enums.h
 *  Header file for weaver specific enumerations.
 *
 * @author Tobias Hilbrich
 * @date 13.01.2010
 */

#ifndef ENUM_H
#define ENUM_H

/**
 * Different usage intent of function arguments.
 */
enum ArgumentIntent
{
	INTENT_IN = 0,
	INTENT_OUT,
	INTENT_INOUT
};

/**
 * Different types of a levels size.
 */
enum LevelSizeType
{
	SIZE_TYPE_ABSOLUTE = 0,
	SIZE_TYPE_FRACTIONAL
};

/**
 * Different types of a place.
 */
enum PlaceType
{
	PLACE_TYPE_EXECUTABLE = 0,
	PLACE_TYPE_APP,
	PLACE_TYPE_MODULE
};

/**
 * Enumeration of all setting types.
 */
enum SettingType
{
	BOOL_SETTING = 0,
	FLOAT_SETTING,
	INTEGER_SETTING,
	ENUM_SETTING,
	ENUM_SELECTION_SETTING,
	FILE_PATH_SETTING,
	PATH_SETTING,
	STRING_SETTING,
	UNKNOWN_SETTING
};

/**
 * Enumeration that lists different usage
 * intentions for a file path.
 * This is required to determine whether a
 * file needs to exist or not.
 */
enum FilePathSettingIntention
{
	IN_INTENTION = 0,
	OUT_INTENTION,
	IN_OUT_INTENTION,
	UNKNOWN_INTENTION
};

/**
 * Used to determine whether an operation
 * or analysis is executed before or after
 * executing the actual call for a wrapped
 * call. I.e.:
 * @code
 * MyCall (...) //wrapper for MyCall
 * {
 *    //PRE phase
 *    [P]MyCall (...); //call actual call
 *    //POST phase
 * }
 * @endcode
 */
enum CalculationOrder
{
	ORDER_PRE = 0,
	ORDER_POST
};

#endif /*ENUM_H*/
