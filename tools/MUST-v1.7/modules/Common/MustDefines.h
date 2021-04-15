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
 * @file MustDefines.h
 *       @see MustDefines.
 *
 *  @date 17.08.2011
 *  @author Mathias Korepkat
 */

#include "mpi.h"
#include <stdint.h>

#ifndef MUSTDEFINES_H
#define MUSTDEFINES_H

#define MUST_BOTTOM ((int64_t)-1L)
#define MUST_IN_PLACE ((int64_t)-2L)

#ifdef __cplusplus
#   define EXTERN extern "C"
#else
#   define EXTERN extern
#endif

/**
 * generate fortran bindings
 */
#define GENERATE_F77_BINDINGS(lower_case, \
                              upper_case, \
                              wrapper_function, \
                              signature, \
                              params) \
  EXTERN void lower_case signature; \
  EXTERN void lower_case signature { wrapper_function params; } \
  EXTERN void lower_case##_ signature; \
  EXTERN void lower_case##_ signature { wrapper_function params; } \
  EXTERN void lower_case##__ signature; \
  EXTERN void lower_case##__ signature { wrapper_function params; } \
  EXTERN void upper_case signature; \
  EXTERN void upper_case signature { wrapper_function params; }

#ifndef MUST_MAX_NUM_STACKLEVELS
#define MUST_MAX_NUM_STACKLEVELS 10
#endif

#ifndef MUST_MAX_TOTAL_INFO_SIZE
#define MUST_MAX_TOTAL_INFO_SIZE 4096
#endif

#define MUST_OUTPUT_DIR "MUST_Output-files/"
#define MUST_OUTPUT_REDIR "../"
#define MUST_OUTPUT_DIR_CHECK             struct stat sd;\
            if(stat(MUST_OUTPUT_DIR, &sd)!=0)\
            {\
                mkdir(MUST_OUTPUT_DIR, 0755);\
            }\


#endif /*MUSTDEFINES_H*/
