# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

include(CheckFortranMPISymbolExists)
include(CheckMPIFunctionExists)
include(CheckMPISymbolExists)
include(CheckMPISymbolIsRvalue)


#
# helper scripts
#

# Call check_mpi_function_exists and set _PREFIX and _POSTFIX variables.
macro (check_mpi_function_exists_pp function)
    STRING(TOUPPER "HAVE_${function}" havevariable)

    # check for symbol
#    check_mpi_function_exists(${function} ${havevariable})
    check_mpi_symbol_is_rvalue(${function} ${havevariable})

    # Set empty prefix and postfix if symbol was found or set an XML prefix if
    # symbol was not found.
    if (${havevariable})
        set(${havevariable}_PREFIX "")
        set(${havevariable}_POSTFIX "")
    else (${havevariable})
        set(${havevariable}_PREFIX "<!--")
        set(${havevariable}_POSTFIX "-->")
    endif(${havevariable})
endmacro (check_mpi_function_exists_pp)


# call check_mpi_symbol_exists and check for <function>_c2f and <function>_f2c
macro (check_mpi_function_exists_c2f2c function)
    STRING(TOUPPER "HAVE_${function}" variable)

    # check for symbols
    check_mpi_function_exists(${function}_f2c "${variable}_F2C")
    if (${${variable}_F2C})
        check_mpi_function_exists(${function}_c2f "${variable}_C2F")
    endif ()
endmacro (check_mpi_function_exists_c2f2c)



# optional Fortran datatypes
if (GTI_ENABLE_FORTRAN)
    set(OptionalFortranTypes
            MPI_DOUBLE_COMPLEX
            MPI_REAL2 MPI_REAL4 MPI_REAL8 MPI_REAL16
            MPI_INTEGER1 MPI_INTEGER2 MPI_INTEGER4 MPI_INTEGER8 MPI_INTEGER16
            MPI_COMPLEX8 MPI_COMPLEX16 MPI_COMPLEX32
            MPI_LOGICAL1 MPI_LOGICAL2 MPI_LOGICAL4 MPI_LOGICAL8 MPI_LOGICAL16
            MPI_2COMPLEX
            MPI_2DOUBLE_COMPLEX
        CACHE INTERNAL "All the optional Fortran types"
    )

    foreach (type ${OptionalFortranTypes})
        check_fortran_mpi_symbol_exists(${type} HAVE_${type})

        # Set extra variable used in Fortran source for datatype handling
        if (HAVE_${type})
            set(F_${type} "${type}" CACHE INTERNAL "")
        else (HAVE_${type})
            set(F_${type} "MPI_DATATYPE_NULL" CACHE INTERNAL "")
        endif (HAVE_${type})
    endforeach (type)
endif (GTI_ENABLE_FORTRAN)

# optional datatypes C
check_mpi_symbol_is_rvalue(MPI_LONG_LONG_INT HAVE_MPI_LONG_LONG_INT)
check_mpi_symbol_is_rvalue(MPI_LONG_LONG HAVE_MPI_LONG_LONG)
check_mpi_symbol_is_rvalue(MPI_UNSIGNED_LONG_LONG HAVE_MPI_UNSIGNED_LONG_LONG)
check_mpi_symbol_is_rvalue(MPI_WCHAR HAVE_MPI_WCHAR)
check_mpi_symbol_is_rvalue(MPI_SIGNED_CHAR HAVE_MPI_SIGNED_CHAR)

# optional datatypes C++
check_mpi_symbol_is_rvalue(MPI_CXX_BOOL HAVE_MPI_CXX_BOOL)
check_mpi_symbol_is_rvalue(MPI_CXX_FLOAT_COMPLEX HAVE_MPI_CXX_FLOAT_COMPLEX)
check_mpi_symbol_is_rvalue(MPI_CXX_DOUBLE_COMPLEX HAVE_MPI_CXX_DOUBLE_COMPLEX)
check_mpi_symbol_is_rvalue(MPI_CXX_LONG_DOUBLE_COMPLEX HAVE_MPI_CXX_LONG_DOUBLE_COMPLEX)

# removed names and functions
set(Removed30Symbols 
                     MPI_LB 
                     MPI_UB 
                     MPI_Handler_function 
                     MPI_COMBINER_HVECTOR_INTEGER 
                     MPI_COMBINER_HINDEXED_INTEGER 
                     MPI_COMBINER_STRUCT_INTEGER)

foreach (symbol ${Removed30Symbols})
    check_mpi_symbol_is_rvalue(${symbol} HAVE_${symbol})
endforeach()

set(Removed30Functions 
    MPI_Keyval_free
    MPI_Keyval_create
    MPI_Attr_put
    MPI_Attr_get
    MPI_Attr_delete
    MPI_Type_ub
    MPI_Type_struct
    MPI_Type_lb
    MPI_Type_hvector
    MPI_Type_hindexed
    MPI_Type_extent
    MPI_Errhandler_get
    MPI_Errhandler_create
    MPI_Address
)

foreach (function ${Removed30Functions})
    check_mpi_function_exists_pp(${function} HAVE_${function})
endforeach()

# get basic types of MPI datatypes
set(COMM_F_TYPE "uint64_t" CACHE INTERNAL "Return type of MPI_Comm_c2f.")
set(DATATYPE_F_TYPE "uint64_t" CACHE INTERNAL "Return type of MPI_Datatype_c2f.")
set(ERRHANDLER_F_TYPE "uint64_t" CACHE INTERNAL "Return type of MPI_Errhandler_c2f.")
set(GROUP_F_TYPE "uint64_t" CACHE INTERNAL "Return type of MPI_Group_c2f.")
set(OP_F_TYPE "uint64_t" CACHE INTERNAL "Return type of MPI_Op_c2f.")
set(REQUEST_F_TYPE "uint64_t" CACHE INTERNAL "Return type of MPI_Request_c2f.")
set(KEYVAL_TYPE "int" CACHE INTERNAL "Type to use for keyvals.")
set(AINT_TYPE "int64_t" CACHE INTERNAL "Type that is equal to MPI_Aint.")


# handle convert macros
check_mpi_function_exists_c2f2c(MPI_Comm)
check_mpi_function_exists_c2f2c(MPI_Errhandler)
check_mpi_function_exists_c2f2c(MPI_File)
check_mpi_function_exists_c2f2c(MPI_Group)
check_mpi_function_exists_c2f2c(MPI_Info)
check_mpi_function_exists_c2f2c(MPI_Op)
check_mpi_function_exists_c2f2c(MPI_Request)
check_mpi_function_exists_c2f2c(MPI_Status)
check_mpi_function_exists_c2f2c(MPI_Type)
check_mpi_function_exists_c2f2c(MPI_Win)





#
# MPI 2
#

# MPI-2 constants
check_mpi_symbol_is_rvalue(MPI_DISTRIBUTE_BLOCK HAVE_MPI_DISTRIBUTE_BLOCK)
check_mpi_symbol_is_rvalue(MPI_ORDER_C HAVE_MPI_ORDER_C)
check_mpi_symbol_is_rvalue(MPI_STATUS_IGNORE HAVE_MPI_STATUS_IGNORE)
check_mpi_symbol_is_rvalue(MPI_STATUSES_IGNORE HAVE_MPI_STATUSES_IGNORE)

check_mpi_function_exists_pp(MPI_Alltoallw)
check_mpi_function_exists_pp(MPI_Exscan)
check_mpi_function_exists_pp(MPI_Get_address)
check_mpi_function_exists_pp(MPI_Reduce_scatter)
check_mpi_function_exists_pp(MPI_Reduce_scatter_block)
check_mpi_function_exists_pp(MPI_Type_create_darray)
check_mpi_function_exists_pp(MPI_Type_create_hindexed)
check_mpi_function_exists_pp(MPI_Type_create_hvector)
check_mpi_function_exists_pp(MPI_Type_create_indexed_block)
check_mpi_function_exists_pp(MPI_Type_create_resized)
check_mpi_function_exists_pp(MPI_Type_create_struct)
check_mpi_function_exists_pp(MPI_Type_create_subarray)
check_mpi_function_exists_pp(MPI_Type_get_extent)
check_mpi_function_exists_pp(MPI_Type_get_true_extent)
check_mpi_function_exists_pp(MPI_Comm_create_keyval)

# MPI 2.2 constants
foreach (type
    MPI_C_BOOL
    MPI_INT8_T MPI_INT16_T MPI_INT32_T MPI_INT64_T
    MPI_UINT8_T MPI_UINT16_T MPI_UINT32_T MPI_UINT64_T
    MPI_C_COMPLEX MPI_C_FLOAT_COMPLEX MPI_C_DOUBLE_COMPLEX
        MPI_C_LONG_DOUBLE_COMPLEX
)
    check_mpi_symbol_is_rvalue(${type} HAVE_${type})
endforeach (type)


#
# MPI 3
#

check_mpi_symbol_is_rvalue(MPI_DIST_GRAPH HAVE_MPI_DIST_GRAPH)

check_mpi_function_exists_pp(MPI_Dist_graph_neighbors_count)
check_mpi_function_exists_pp(MPI_Iallgather)
check_mpi_function_exists_pp(MPI_Iallgatherv)
check_mpi_function_exists_pp(MPI_Iallreduce)
check_mpi_function_exists_pp(MPI_Ialltoall)
check_mpi_function_exists_pp(MPI_Ialltoallv)
check_mpi_function_exists_pp(MPI_Ialltoallw)
check_mpi_function_exists_pp(MPI_Ibarrier)
check_mpi_function_exists_pp(MPI_Ibcast)
check_mpi_function_exists_pp(MPI_Iexscan)
check_mpi_function_exists_pp(MPI_Igather)
check_mpi_function_exists_pp(MPI_Igatherv)
check_mpi_function_exists_pp(MPI_Ineighbor_allgather)
check_mpi_function_exists_pp(MPI_Ineighbor_allgatherv)
check_mpi_function_exists_pp(MPI_Ineighbor_alltoall)
check_mpi_function_exists_pp(MPI_Ineighbor_alltoallv)
check_mpi_function_exists_pp(MPI_Ineighbor_alltoallw)
check_mpi_function_exists_pp(MPI_Ireduce)
check_mpi_function_exists_pp(MPI_Ireduce_scatter)
check_mpi_function_exists_pp(MPI_Ireduce_scatter_block)
check_mpi_function_exists_pp(MPI_Iscan)
check_mpi_function_exists_pp(MPI_Iscatter)
check_mpi_function_exists_pp(MPI_Iscatterv)
check_mpi_function_exists_pp(MPI_Neighbor_allgather)
check_mpi_function_exists_pp(MPI_Neighbor_allgatherv)
check_mpi_function_exists_pp(MPI_Neighbor_alltoall)
check_mpi_function_exists_pp(MPI_Neighbor_alltoallv)
check_mpi_function_exists_pp(MPI_Neighbor_alltoallw)
check_mpi_function_exists_pp(MPI_Reduce_local)


##Extra types and additions for const correctness (MPI-3)
IF (HAVE_MPI_NO_CONST_CORRECTNESS)
    SET (CONSTABLE_VOIDP_TYPE "void*" CACHE INTERNAL "Type to use for void* that can be const void* in newer MPIs.")
    SET (CONSTABLE_INTP_TYPE "int*" CACHE INTERNAL "Type to use for int* that can be const int [] in newer MPIs.")
    SET (CONSTABLE_INTP_ADDITION "" CACHE INTERNAL "Addition to CONSTABLE_INTP_TYPE.")
    SET (CONSTABLE_DATATYPEP_TYPE "MPI_Datatype*" CACHE INTERNAL "Type to use for MPI_Datatype* that can be const MPI_Datatype [] in newer MPIs.")
    SET (CONSTABLE_DATATYPEP_ADDITION "" CACHE INTERNAL "Addition to CONSTABLE_DATATYPEP_TYPE.")
    SET (CONSTABLE_SINGLE_STATUSP "MPI_Status*" CACHE INTERNAL "Type to use for MPI_Status* that can be const MPI_Status* in newer MPIs.")
    SET (CONSTABLE_AINTP_TYPE "MPI_Aint*" CACHE INTERNAL "Type to use for MPI_Aint* that can be const MPI_Aint [] in newer MPIs.")
    SET (CONSTABLE_AINTP_ADDITION "" CACHE INTERNAL "Addition to CONSTABLE_AINTP_TYPE.")
ELSE ()
    SET (CONSTABLE_VOIDP_TYPE "const void*" CACHE INTERNAL "Type to use void* that can be const void* in newer MPIs.")
    SET (CONSTABLE_INTP_TYPE "const int" CACHE INTERNAL "Type to use for int* that can be const int [] in newer MPIs.")
    SET (CONSTABLE_INTP_ADDITION "typeAfterArg=\"[]\"" CACHE INTERNAL "Addition to CONSTABLE_INTP_TYPE.")
    SET (CONSTABLE_DATATYPEP_TYPE "const MPI_Datatype" CACHE INTERNAL "Type to use for MPI_Datatype* that can be const MPI_Datatype [] in newer MPIs.")
    SET (CONSTABLE_DATATYPEP_ADDITION "typeAfterArg=\"[]\"" CACHE INTERNAL "Addition to CONSTABLE_DATATYPEP_TYPE.")
    SET (CONSTABLE_SINGLE_STATUSP "const MPI_Status*" CACHE INTERNAL "Type to use for MPI_Status* that can be const MPI_Status* in newer MPIs.")
    SET (CONSTABLE_AINTP_TYPE "const MPI_Aint" CACHE INTERNAL "Type to use for MPI_Aint* that can be const MPI_Aint [] in newer MPIs.")
    SET (CONSTABLE_AINTP_ADDITION "typeAfterArg=\"[]\"" CACHE INTERNAL "Addition to CONSTABLE_AINTP_TYPE.")
ENDIF ()

#Special handling for MPI_Address
IF (HAVE_MPI_ADDRESS_CONST_CORRECT)
    SET (MPI_ADDRESS_CONSTABLE_VOIDP_TYPE "const void*" CACHE INTERNAL "Type to use void* that can be const void* in newer MPIs.")
ELSE ()
    SET (MPI_ADDRESS_CONSTABLE_VOIDP_TYPE "void*" CACHE INTERNAL "Type to use for void* that can be const void* in newer MPIs.")
ENDIF ()

#Special handling for MPI_Type_hindexed
IF (HAVE_MPI_TYPE_HINDEXED_CONST_CORRECT)
    SET (MPI_TYPE_HINDEXED_CONSTABLE_INTP_TYPE "const int" CACHE INTERNAL "Type to use for int* that can be const int [] in newer MPIs.")
    SET (MPI_TYPE_HINDEXED_CONSTABLE_INTP_ADDITION "typeAfterArg=\"[]\"" CACHE INTERNAL "Addition to CONSTABLE_INTP_TYPE.")
    SET (MPI_TYPE_HINDEXED_CONSTABLE_AINTP_TYPE "const MPI_Aint" CACHE INTERNAL "Type to use for MPI_Aint* that can be const MPI_Aint [] in newer MPIs.")
    SET (MPI_TYPE_HINDEXED_CONSTABLE_AINTP_ADDITION "typeAfterArg=\"[]\"" CACHE INTERNAL "Addition to CONSTABLE_AINTP_TYPE.")
ELSE ()
    SET (MPI_TYPE_HINDEXED_CONSTABLE_INTP_TYPE "int*" CACHE INTERNAL "Type to use for int* that can be const int [] in newer MPIs.")
    SET (MPI_TYPE_HINDEXED_CONSTABLE_INTP_ADDITION "" CACHE INTERNAL "Addition to CONSTABLE_INTP_TYPE.")
    SET (MPI_TYPE_HINDEXED_CONSTABLE_AINTP_TYPE "MPI_Aint*" CACHE INTERNAL "Type to use for MPI_Aint* that can be const MPI_Aint [] in newer MPIs.")
    SET (MPI_TYPE_HINDEXED_CONSTABLE_AINTP_ADDITION "" CACHE INTERNAL "Addition to CONSTABLE_AINTP_TYPE.")
ENDIF ()

#Special handling for MPI_Type_struct
IF (HAVE_MPI_TYPE_STRUCT_CONST_CORRECT)
    SET (MPI_TYPE_STRUCT_CONSTABLE_INTP_TYPE "const int" CACHE INTERNAL "Type to use for int* that can be const int [] in newer MPIs.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_INTP_ADDITION "typeAfterArg=\"[]\"" CACHE INTERNAL "Addition to CONSTABLE_INTP_TYPE.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_AINTP_TYPE "const MPI_Aint" CACHE INTERNAL "Type to use for MPI_Aint* that can be const MPI_Aint [] in newer MPIs.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_AINTP_ADDITION "typeAfterArg=\"[]\"" CACHE INTERNAL "Addition to CONSTABLE_AINTP_TYPE.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_DATATYPEP_TYPE "const MPI_Datatype" CACHE INTERNAL "Type to use for MPI_Datatype* that can be const MPI_Datatype [] in newer MPIs.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_DATATYPEP_ADDITION "typeAfterArg=\"[]\"" CACHE INTERNAL "Addition to CONSTABLE_DATATYPEP_TYPE.")
ELSE ()
    SET (MPI_TYPE_STRUCT_CONSTABLE_INTP_TYPE "int*" CACHE INTERNAL "Type to use for int* that can be const int [] in newer MPIs.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_INTP_ADDITION "" CACHE INTERNAL "Addition to CONSTABLE_INTP_TYPE.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_AINTP_TYPE "MPI_Aint*" CACHE INTERNAL "Type to use for MPI_Aint* that can be const MPI_Aint [] in newer MPIs.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_AINTP_ADDITION "" CACHE INTERNAL "Addition to CONSTABLE_AINTP_TYPE.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_DATATYPEP_TYPE "MPI_Datatype*" CACHE INTERNAL "Type to use for MPI_Datatype* that can be const MPI_Datatype [] in newer MPIs.")
    SET (MPI_TYPE_STRUCT_CONSTABLE_DATATYPEP_ADDITION "" CACHE INTERNAL "Addition to CONSTABLE_DATATYPEP_TYPE.")
ENDIF ()


SET (LOGICAL_TS_TYPE "unsigned long" CACHE INTERNAL "Type to use for MUST's logical time stamps.")
