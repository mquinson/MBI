# This file is part of GTI (Generic Tool Infrastructure)
#
# Copyright (C)
#   2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2008-2019 Lawrence Livermore National Laboratories, United States of America
#   2013-2019 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

##
# @file CMakeLists.cmake
#       CMake file for the modules/Common directory.
#
# @author Tobias Hilbrich
# @date 04.02.2011

#MPI-2 functions
SET (MUST_SUPPORTED_MPI2_FUNCS
    MPI_Comm_create_errhandler
    MPI_Comm_set_errhandler
    MPI_Comm_spawn
    CACHE INTERNAL "Mpi-2 specific functions"
)

FOREACH (func ${MUST_SUPPORTED_MPI2_FUNCS})
    STRING (TOLOWER ${func} testName)
    STRING (TOUPPER ${func} haveName)
    SET (testName "ft_${testName}.cpp")
    SET (haveName "HAVE_${haveName}")

    featureTestMpi ("${testName}" CXX ${haveName})
ENDFOREACH (func)

##MPI Const correctness
#Basic need for const correctness
featureTestMpi ("ft_mpi_const_correctness.cpp" CXX HAVE_MPI_CONST_CORRECTNESS)

IF(NOT HAVE_MPI_CONST_CORRECTNESS)
    set(HAVE_MPI_NO_CONST_CORRECTNESS "True" CACHE INTERNAL "Result of testing for const correctness." )
ELSE ()
    set(HAVE_MPI_NO_CONST_CORRECTNESS "False" CACHE INTERNAL "Result of testing for const correctness." )   
ENDIF()

#Specialized adaption for MPI_Address
featureTestMpi ("ft_mpi_const_correctness_address.cpp" CXX HAVE_MPI_ADDRESS_CONST_CORRECT)
#Specialized adaption for MPI_Type_hindexed
featureTestMpi ("ft_mpi_const_correctness_type_hindexed.cpp" CXX HAVE_MPI_TYPE_HINDEXED_CONST_CORRECT)
#Specialized adaption for MPI_Type_struct
featureTestMpi ("ft_mpi_const_correctness_type_struct.cpp" CXX HAVE_MPI_TYPE_STRUCT_CONST_CORRECT)
