/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include "mustConfig.h"
#include "gtiConfig.h"

#ifdef HAVE_MPI_TYPE_CREATE_STRUCT
#define MPI_Type_struct MPI_Type_create_struct
#endif

#ifdef HAVE_MPI_TYPE_CREATE_HVECTOR
#define MPI_Type_hvector MPI_Type_create_hvector
#endif

#ifdef HAVE_MPI_TYPE_CREATE_HINDEXED
#define MPI_Type_hindexed MPI_Type_create_hindexed
#endif

#ifdef HAVE_MPI_TYPE_GET_EXTENT
#define MPI_Type_extent(type, extent) {MPI_Aint lb; MPI_Type_get_extent(type, &lb, extent);}
#define MPI_Type_lb(type, lb) {MPI_Aint extent; MPI_Type_get_extent(type, lb, &extent);}
#define MPI_Type_ub(type, ub) {MPI_Aint lb, extent; MPI_Type_get_extent(type, &lb, &extent); *ub=lb+extent;}
#endif

#ifdef HAVE_MPI_GET_ADDRESS
#define MPI_Address MPI_Get_address
#endif


#ifdef HAVE_MPI_COMM_CREATE_KEYVAL
#define MPI_Keyval_free MPI_Comm_free_keyval
#define MPI_Keyval_create MPI_Comm_create_keyval
#undef MPI_NULL_COPY_FN
#define MPI_NULL_COPY_FN MPI_COMM_NULL_COPY_FN
#undef MPI_NULL_DELETE_FN
#define MPI_NULL_DELETE_FN MPI_COMM_NULL_DELETE_FN
#undef MPI_DUP_FN
#define MPI_DUP_FN MPI_COMM_DUP_FN
#endif

#ifdef HAVE_MPI_COMM_CREATE_ERRHANDLER
#define MPI_Errhandler_create MPI_Comm_create_errhandler
#endif

