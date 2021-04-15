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
 * @file mustFeaturetested.h
 *       Header with configuration defines.
 *
 *  @date 21.06.2011
 *  @author Joachim Protze
 */

#include "mustConfig.h"
#include "MustTypes.h"

#include <stdint.h>

#ifndef MUSTFEATURETESTED_H
#define MUSTFEATURETESTED_H

#ifdef HAVE_MPI_DISTRIBUTE_BLOCK
#define MUST_DISTRIBUTE_BLOCK MPI_DISTRIBUTE_BLOCK
#define MUST_DISTRIBUTE_CYCLIC MPI_DISTRIBUTE_CYCLIC
#define MUST_DISTRIBUTE_NONE MPI_DISTRIBUTE_NONE
#define MUST_DISTRIBUTE_DFLT_DARG MPI_DISTRIBUTE_DFLT_DARG
#else
#define MUST_DISTRIBUTE_BLOCK 0
#define MUST_DISTRIBUTE_CYCLIC 0
#define MUST_DISTRIBUTE_NONE 0
#define MUST_DISTRIBUTE_DFLT_DARG 0
#endif

#ifdef HAVE_MPI_ORDER_C
#define MUST_ORDER_C MPI_ORDER_C
#define MUST_ORDER_FORTRAN MPI_ORDER_FORTRAN
#else
#define MUST_ORDER_C 0
#define MUST_ORDER_FORTRAN 0
#endif

// bash "oneliner" to generate the following lines:
// for i in comm errhandler file group info op request status type win; do I=$(echo $i | tr '[a-z]' '[A-Z]'); F=$(echo -n $i | cut -c1 | tr '[a-z]' '[A-Z]')$(echo -n $i | cut -c2-);echo "/* whether mpi.h has MPI_${F}_c2f() and MPI_${F}_f2c() or not */";echo "#ifdef HAVE_MPI_${I}_C2F"; echo "#define MUST_${F}_c2f(h) MPI_${F}_c2f(h)"; echo "#define MUST_${F}_f2c(h) MPI_${F}_f2c(h)"; echo "#else"; echo "#define MUST_${F}_c2f(h) ((Must${F}Type)(h))"; echo "#define MUST_${F}_f2c(h) ((MPI_$F)(h))"; echo "#endif /* HAVE_MPI_${I}_C2F */" ;echo; done

/* whether mpi.h has MPI_Comm_c2f() and MPI_Comm_f2c() or not */
#ifdef HAVE_MPI_COMM_C2F
#define MUST_Comm_c2f(h) MPI_Comm_c2f(h)
#define MUST_Comm_f2c(h) MPI_Comm_f2c(h)
#else
#define MUST_Comm_c2f(h) ((MustCommType)(h))
#define MUST_Comm_f2c(h) ((MPI_Comm)(h))
#endif /* HAVE_MPI_COMM_C2F */

/* whether mpi.h has MPI_Errhandler_c2f() and MPI_Errhandler_f2c() or not */
#ifdef HAVE_MPI_ERRHANDLER_C2F
#define MUST_Errhandler_c2f(h) MPI_Errhandler_c2f(h)
#define MUST_Errhandler_f2c(h) MPI_Errhandler_f2c(h)
#else
#define MUST_Errhandler_c2f(h) ((MustErrType)(h))
#define MUST_Errhandler_f2c(h) ((MPI_Errhandler)(h))
#endif /* HAVE_MPI_ERRHANDLER_C2F */

/* whether mpi.h has MPI_File_c2f() and MPI_File_f2c() or not */
#ifdef HAVE_MPI_FILE_C2F
#define MUST_File_c2f(h) MPI_File_c2f(h)
#define MUST_File_f2c(h) MPI_File_f2c(h)
#else
#define MUST_File_c2f(h) ((MustFileType)(h))
#define MUST_File_f2c(h) ((MPI_File)(h))
#endif /* HAVE_MPI_FILE_C2F */

/* whether mpi.h has MPI_Group_c2f() and MPI_Group_f2c() or not */
#ifdef HAVE_MPI_GROUP_C2F
#define MUST_Group_c2f(h) MPI_Group_c2f(h)
#define MUST_Group_f2c(h) MPI_Group_f2c(h)
#else
#define MUST_Group_c2f(h) ((MustGroupType)(h))
#define MUST_Group_f2c(h) ((MPI_Group)(h))
#endif /* HAVE_MPI_GROUP_C2F */

/* whether mpi.h has MPI_Info_c2f() and MPI_Info_f2c() or not */
#ifdef HAVE_MPI_INFO_C2F
#define MUST_Info_c2f(h) MPI_Info_c2f(h)
#define MUST_Info_f2c(h) MPI_Info_f2c(h)
#else
#define MUST_Info_c2f(h) ((MustInfoType)(h))
#define MUST_Info_f2c(h) ((MPI_Info)(h))
#endif /* HAVE_MPI_INFO_C2F */

/* whether mpi.h has MPI_Op_c2f() and MPI_Op_f2c() or not */
#ifdef HAVE_MPI_OP_C2F
#define MUST_Op_c2f(h) MPI_Op_c2f(h)
#define MUST_Op_f2c(h) MPI_Op_f2c(h)
#else
#define MUST_Op_c2f(h) ((MustOpType)(h))
#define MUST_Op_f2c(h) ((MPI_Op)(h))
#endif /* HAVE_MPI_OP_C2F */

/* whether mpi.h has MPI_Request_c2f() and MPI_Request_f2c() or not */
#ifdef HAVE_MPI_REQUEST_C2F
#define MUST_Request_c2f(h) MPI_Request_c2f(h)
#define MUST_Request_f2c(h) MPI_Request_f2c(h)
#else
#define MUST_Request_c2f(h) ((MustRequestType)(h))
#define MUST_Request_f2c(h) ((MPI_Request)(h))
#endif /* HAVE_MPI_REQUEST_C2F */

/* whether mpi.h has MPI_Status_c2f() and MPI_Status_f2c() or not */
#ifdef HAVE_MPI_STATUS_C2F
#define MUST_Status_c2f(h) MPI_Status_c2f(h)
#define MUST_Status_f2c(h) MPI_Status_f2c(h)
#else
#define MUST_Status_c2f(h) ((MustStatusType)(h))
#define MUST_Status_f2c(h) ((MPI_Status)(h))
#endif /* HAVE_MPI_STATUS_C2F */

/* whether mpi.h has MPI_Type_c2f() and MPI_Type_f2c() or not */
#ifdef HAVE_MPI_TYPE_C2F
#define MUST_Type_c2f(h) MPI_Type_c2f(h)
#define MUST_Type_f2c(h) MPI_Type_f2c(h)
#else
#define MUST_Type_c2f(h) ((MustDatatypeType)(h))
#define MUST_Type_f2c(h) ((MPI_Datatype)(h))
#endif /* HAVE_MPI_TYPE_C2F */

/* whether mpi.h has MPI_Win_c2f() and MPI_Win_f2c() or not */
#ifdef HAVE_MPI_WIN_C2F
#define MUST_Win_c2f(h) MPI_Win_c2f(h)
#define MUST_Win_f2c(h) MPI_Win_f2c(h)
#else
#define MUST_Win_c2f(h) ((MustWinType)(h))
#define MUST_Win_f2c(h) ((MPI_Win)(h))
#endif /* HAVE_MPI_WIN_C2F */

// bash "oneliner" to generate the following lines:
// for i in comm errhandler file group info op request type win; do I=$(echo $i | tr '[a-z]' '[A-Z]'); F=$(echo -n $i | cut -c1 | tr '[a-z]' '[A-Z]')$(echo -n $i | cut -c2-);echo "#define MUST_${F}_m2i(h) ((Must${F}Type)(h))"; echo "#define MUST_${F}_i2m(h) ((MPI_$F)(h))"; echo; done

#define MUST_Comm_m2i(h) ((MustCommType)(h))
#define MUST_Comm_i2m(h) ((MPI_Comm)(h))

#define MUST_Errhandler_m2i(h) ((MustErrType)(h))
#define MUST_Errhandler_i2m(h) ((MPI_Errhandler)(h))

#define MUST_File_m2i(h) ((MustFileType)(h))
#define MUST_File_i2m(h) ((MPI_File)(h))

#define MUST_Group_m2i(h) ((MustGroupType)(h))
#define MUST_Group_i2m(h) ((MPI_Group)(h))

#define MUST_Info_m2i(h) ((MustInfoType)(h))
#define MUST_Info_i2m(h) ((MPI_Info)(h))

#define MUST_Op_m2i(h) ((MustOpType)(h))
#define MUST_Op_i2m(h) ((MPI_Op)(h))

#define MUST_Request_m2i(h) ((MustRequestType)(h))
#define MUST_Request_i2m(h) ((MPI_Request)(h))

#define MUST_Type_m2i(h) ((MustDatatypeType)(h))
#define MUST_Type_i2m(h) ((MPI_Datatype)(h))

#define MUST_Win_m2i(h) ((MustWinType)(h))
#define MUST_Win_i2m(h) ((MPI_Win)(h))



#endif /* MUSTFEATURETESTED_H */
