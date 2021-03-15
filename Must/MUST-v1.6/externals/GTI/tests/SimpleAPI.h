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
 * @file SimpleAPI.h
 *       prototypes for simple API calls of the simple weaver test.
 *
 *  @date 25.08.2010
 *  @author Tobias Hilbrich
 */

#ifndef GTI_TEST_SIMPLE_API_H
#define GTI_TEST_SIMPLE_API_H

extern "C" int test1 (int count, int* sizes, float f);

extern "C" int Ptest1 (int count, int* sizes, float f);

extern "C" int newSize (int size);

extern "C" int PnewSize (int size);

extern "C" int shutdown (void);

extern "C" int Pshutdown (void);

int PreducedFloatSum (float fSum) {return 0;}

#endif /*GTI_TEST_SIMPLE_API_H*/
