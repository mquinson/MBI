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
 * @file DatatypePredefs.cpp
 *       @see MUST::DatatypePredefs.
 *
 *  @date 18.02.2011
 *  @author Tobias Hilbrich
 */

#include "mustFeaturetested.h"
#include "mustConfig.h"

#include "GtiMacros.h"

#include "DatatypePredefs.h"
#include "I_DatatypeTrack.h"
#include "ResourceApi.h"

#include <mpi.h>

using namespace must;

mGET_INSTANCE_FUNCTION(DatatypePredefs)
mFREE_INSTANCE_FUNCTION(DatatypePredefs)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DatatypePredefs)

/*
 * Prototypes for Fortran helper functions that are
 * used to return the handle values for these types.
 */
extern "C" void get_mpi_integer_code___ (int* code);
extern "C" void get_mpi_real_code___ (int* code);
extern "C" void get_mpi_double_precision_code___ (int* code);
extern "C" void get_mpi_complex_code___ (int* code);
extern "C" void get_mpi_logical_code___ (int* code);
extern "C" void get_mpi_character_code___ (int* code);

extern "C" void get_mpi_2real_code___ (int* code);
extern "C" void get_mpi_2double_precision_code___ (int* code);
extern "C" void get_mpi_2integer_code___ (int* code);
extern "C" void get_mpi_2complex_code___ (int* code);
extern "C" void get_mpi_2double_complex_code___ (int* code);

extern "C" void get_mpi_integer1_code___ (int* code);
extern "C" void get_mpi_integer2_code___ (int* code);
extern "C" void get_mpi_integer4_code___ (int* code);
extern "C" void get_mpi_integer8_code___ (int* code);
extern "C" void get_mpi_integer16_code___ (int* code);
extern "C" void get_mpi_real2_code___ (int* code);
extern "C" void get_mpi_real4_code___ (int* code);
extern "C" void get_mpi_real8_code___ (int* code);
extern "C" void get_mpi_real16_code___ (int* code);
extern "C" void get_mpi_double_complex_code___ (int* code);
extern "C" void get_mpi_complex8_code___ (int* code);
extern "C" void get_mpi_complex16_code___ (int* code);
extern "C" void get_mpi_complex32_code___ (int* code);
extern "C" void get_mpi_logical1_code___ (int* code);
extern "C" void get_mpi_logical2_code___ (int* code);
extern "C" void get_mpi_logical4_code___ (int* code);
extern "C" void get_mpi_logical8_code___ (int* code);
extern "C" void get_mpi_logical16_code___ (int* code);

//=============================
// Constructor
//=============================
DatatypePredefs::DatatypePredefs (const char* instanceName)
    : gti::ModuleBase<DatatypePredefs, I_DatatypePredefs> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    ////no sub modules needed
}

//=============================
// Destructor
//=============================
DatatypePredefs::~DatatypePredefs ()
{
    //Nothing to do
}

void InsertCPredef(
        MustMpiDatatypePredefined mustId,
        MPI_Datatype mpiId,
        std::map<MustMpiDatatypePredefined, MustDatatypeType> &predefs,
        std::map<MustMpiDatatypePredefined, int> &alignments,
        std::map<MustMpiDatatypePredefined, MPI_Aint> &extents
        )
    {
	//For geting extents
	MPI_Aint extent;
        int retValue;

	//For getting alignments
	MPI_Datatype types[2] = {MPI_DATATYPE_NULL, MPI_CHAR}; //first value needs to be adapted
	int blocklens[2] = {1,1};
	MPI_Aint displs[2] = {0,0}; //second value needs to be adapted
	MPI_Datatype newType;
	if (mpiId != MPI_DATATYPE_NULL){predefs.insert(std::make_pair(mustId, MUST_Type_m2i(mpiId)));
#if HAVE_MPI_TYPE_GET_EXTENT
        MPI_Aint lb;
	retValue = PMPI_Type_get_extent(mpiId, &lb, &extent);
#elif HAVE_MPI_TYPE_EXTENT
	retValue = PMPI_Type_extent(mpiId, &extent);
#else
	#error "Neither MPI_Type_get_extent nor MPI_Type_extent seems to be available"
#endif
	if (retValue != MPI_SUCCESS) std::cerr << "WARNING: MPI_Type_extent(" << mpiId << ") failed" << std::endl;
	extents.insert(std::make_pair(mustId, extent));
	types[0] = mpiId;
	displs[1] = extents[mustId];
#if HAVE_MPI_TYPE_CREATE_STRUCT
	PMPI_Type_create_struct (2, blocklens, displs, types, &newType);
#else
	PMPI_Type_struct (2, blocklens, displs, types, &newType);
#endif
#if HAVE_MPI_TYPE_GET_EXTENT
	retValue = PMPI_Type_get_extent(newType, &lb, &extent);
#elif HAVE_MPI_TYPE_EXTENT
	retValue = PMPI_Type_extent(newType, &extent);
#else
	#error "Neither MPI_Type_get_extent nor MPI_Type_extent seems to be available"
#endif
    if (retValue != MPI_SUCCESS) std::cerr << "WARNING: MPI_Type_extent(" << mpiId << ") failed" << std::endl;
	alignments.insert(std::make_pair(mustId, extent - extents[mustId]));
    PMPI_Type_free(&newType);}
    }

void InsertFortranPredef(
        MustMpiDatatypePredefined mustId,
        int mpiId,
        std::map<MustMpiDatatypePredefined, MustDatatypeType> &predefs,
        std::map<MustMpiDatatypePredefined, int> &alignments,
        std::map<MustMpiDatatypePredefined, MPI_Aint> &extents
        )
{
	//For geting extents
	MPI_Aint extent;
        int retValue;

	//For getting alignments
	MPI_Datatype types[2] = {MPI_DATATYPE_NULL, MPI_CHAR}; //first value needs to be adapted
	int blocklens[2] = {1,1};
	MPI_Aint displs[2] = {0,0}; //second value needs to be adapted
	MPI_Datatype newType;
	types[0] = MUST_Type_f2c(mpiId);
	if (mpiId != MUST_Type_c2f(MPI_DATATYPE_NULL)){predefs.insert(std::make_pair(mustId, MUST_Type_m2i(types[0])));
#if HAVE_MPI_TYPE_GET_EXTENT
        MPI_Aint lb;
	retValue = PMPI_Type_get_extent(types[0], &lb, &extent);
#else
	retValue = PMPI_Type_extent(types[0], &extent);
#endif
        if (retValue != MPI_SUCCESS) std::cerr << "WARNING: MPI_Type_extent(" << mpiId << ") failed" << std::endl;
	extents.insert(std::make_pair(mustId, extent));
	displs[1] = extents[mustId];
#if HAVE_MPI_TYPE_CREATE_STRUCT
	PMPI_Type_create_struct (2, blocklens, displs, types, &newType);
#else
	PMPI_Type_struct (2, blocklens, displs, types, &newType);
#endif
#if HAVE_MPI_TYPE_GET_EXTENT
	retValue = PMPI_Type_get_extent(newType, &lb, &extent);
#else
	retValue = PMPI_Type_extent(newType, &extent);
#endif
	if (retValue != MPI_SUCCESS) std::cerr << "WARNING: MPI_Type_extent(" << mpiId << ") failed" << std::endl;
	alignments.insert(std::make_pair(mustId, extent - extents[mustId]));
    PMPI_Type_free(&newType);}
}

//=============================
// propagate
//=============================
GTI_ANALYSIS_RETURN DatatypePredefs::propagate (
                MustParallelId pId
)
{
	std::map<MustMpiDatatypePredefined, MustDatatypeType> predefs;
	std::map<MustMpiDatatypePredefined, int> alignments;
	std::map<MustMpiDatatypePredefined, MPI_Aint> extents;
	int handle;
	int fortranNull = MUST_Type_c2f (MPI_DATATYPE_NULL);

	//For geting extents
	MPI_Aint extent;

	//For getting alignments
	MPI_Datatype types[2] = {MPI_DATATYPE_NULL, MPI_CHAR}; //first value needs to be adapted
	int blocklens[2] = {1,1};
	MPI_Aint displs[2] = {0,0}; //second value needs to be adapted
	MPI_Datatype newType;

	//==Add all the predefined values to the map
	//=1) Elementary C
	InsertCPredef(MUST_MPI_CHAR, MPI_CHAR, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_SHORT, MPI_SHORT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_INT, MPI_INT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_LONG, MPI_LONG, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_UNSIGNED_SHORT, MPI_UNSIGNED_SHORT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_UNSIGNED, MPI_UNSIGNED, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_FLOAT, MPI_FLOAT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_DOUBLE, MPI_DOUBLE, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_LONG_DOUBLE, MPI_LONG_DOUBLE, predefs, alignments, extents);

	//=2) Elementary C & Fortran
	InsertCPredef(MUST_MPI_BYTE, MPI_BYTE, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_PACKED, MPI_PACKED, predefs, alignments, extents);

	//=3) Elementary Fortran
#ifdef GTI_ENABLE_FORTRAN
	get_mpi_integer_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_INTEGER, handle, predefs, alignments, extents);
	}
	get_mpi_real_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_REAL, handle, predefs, alignments, extents);
	}
	get_mpi_double_precision_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_DOUBLE_PRECISION, handle, predefs, alignments, extents);
	}
	get_mpi_complex_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_COMPLEX, handle, predefs, alignments, extents);
	}
	get_mpi_logical_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_LOGICAL, handle, predefs, alignments, extents);
	}
	get_mpi_character_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_CHARACTER, handle, predefs, alignments, extents);
	}
#endif

	//=4) Reduction types C
	InsertCPredef(MUST_MPI_FLOAT_INT, MPI_FLOAT_INT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_DOUBLE_INT, MPI_DOUBLE_INT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_LONG_INT, MPI_LONG_INT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_2INT, MPI_2INT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_SHORT_INT, MPI_SHORT_INT, predefs, alignments, extents);
	InsertCPredef(MUST_MPI_LONG_DOUBLE_INT, MPI_LONG_DOUBLE_INT, predefs, alignments, extents);

	//=5) Reduction types Fortran
#ifdef GTI_ENABLE_FORTRAN
	get_mpi_2real_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_2REAL, handle, predefs, alignments, extents);
	}
	get_mpi_2double_precision_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_2DOUBLE_PRECISION, handle, predefs, alignments, extents);
	}
	get_mpi_2integer_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_2INTEGER, handle, predefs, alignments, extents);
	}
	get_mpi_2complex_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_2COMPLEX, handle, predefs, alignments, extents);
	}
	get_mpi_2double_complex_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_2DOUBLE_COMPLEX, handle, predefs, alignments, extents);
	}
#endif

	//=6) Optional C
#ifdef HAVE_MPI_LONG_LONG_INT
	InsertCPredef(MUST_MPI_LONG_LONG_INT, MPI_LONG_LONG_INT, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_LONG_LONG
	InsertCPredef(MUST_MPI_LONG_LONG, MPI_LONG_LONG, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_UNSIGNED_LONG_LONG
	InsertCPredef(MUST_MPI_UNSIGNED_LONG_LONG, MPI_UNSIGNED_LONG_LONG, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_WCHAR
	InsertCPredef(MUST_MPI_WCHAR, MPI_WCHAR, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_SIGNED_CHAR
	InsertCPredef(MUST_MPI_SIGNED_CHAR, MPI_SIGNED_CHAR, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_C_BOOL
	InsertCPredef(MUST_MPI_C_BOOL, MPI_C_BOOL, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_INT8_T
	InsertCPredef(MUST_MPI_INT8_T, MPI_INT8_T, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_INT16_T
	InsertCPredef(MUST_MPI_INT16_T, MPI_INT16_T, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_INT32_T
	InsertCPredef(MUST_MPI_INT32_T, MPI_INT32_T, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_INT64_T
	InsertCPredef(MUST_MPI_INT64_T, MPI_INT64_T, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_UINT8_T
	InsertCPredef(MUST_MPI_UINT8_T, MPI_UINT8_T, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_UINT16_T
	InsertCPredef(MUST_MPI_UINT16_T, MPI_UINT16_T, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_UINT32_T
	InsertCPredef(MUST_MPI_UINT32_T, MPI_UINT32_T, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_UINT64_T
	InsertCPredef(MUST_MPI_UINT64_T, MPI_UINT64_T, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_C_COMPLEX
	InsertCPredef(MUST_MPI_C_COMPLEX, MPI_C_COMPLEX, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_C_FLOAT_COMPLEX
	InsertCPredef(MUST_MPI_C_FLOAT_COMPLEX, MPI_C_FLOAT_COMPLEX, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_C_DOUBLE_COMPLEX
	InsertCPredef(MUST_MPI_C_DOUBLE_COMPLEX, MPI_C_DOUBLE_COMPLEX, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_C_LONG_DOUBLE_COMPLEX
	InsertCPredef(MUST_MPI_C_LONG_DOUBLE_COMPLEX, MPI_C_LONG_DOUBLE_COMPLEX, predefs, alignments, extents);
#endif


	//=6a) Optional C++
#ifdef HAVE_MPI_CXX_BOOL
	InsertCPredef(MUST_MPI_CXX_BOOL, MPI_CXX_BOOL, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_CXX_FLOAT_COMPLEX
	InsertCPredef(MUST_MPI_CXX_FLOAT_COMPLEX, MPI_CXX_FLOAT_COMPLEX, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_CXX_DOUBLE_COMPLEX
	InsertCPredef(MUST_MPI_CXX_DOUBLE_COMPLEX, MPI_CXX_DOUBLE_COMPLEX, predefs, alignments, extents);
#endif
#ifdef HAVE_MPI_CXX_LONG_DOUBLE_COMPLEX
	InsertCPredef(MUST_MPI_CXX_LONG_DOUBLE_COMPLEX, MPI_CXX_LONG_DOUBLE_COMPLEX, predefs, alignments, extents);
#endif

	//=7) Optional Fortran
#ifdef GTI_ENABLE_FORTRAN
	get_mpi_integer1_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_INTEGER1, handle, predefs, alignments, extents);
	}
	get_mpi_integer2_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_INTEGER2, handle, predefs, alignments, extents);
	}
	get_mpi_integer4_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_INTEGER4, handle, predefs, alignments, extents);
	}
	get_mpi_integer8_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_INTEGER8, handle, predefs, alignments, extents);
	}
	get_mpi_integer16_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_INTEGER16, handle, predefs, alignments, extents);
	}
	get_mpi_real2_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_REAL2, handle, predefs, alignments, extents);
	}
	get_mpi_real4_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_REAL4, handle, predefs, alignments, extents);
	}
	get_mpi_real8_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_REAL8, handle, predefs, alignments, extents);
	}
	get_mpi_real16_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_REAL16, handle, predefs, alignments, extents);
	}
	get_mpi_double_complex_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_DOUBLE_COMPLEX, handle, predefs, alignments, extents);
	}
	get_mpi_complex8_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_COMPLEX8, handle, predefs, alignments, extents);
	}
	get_mpi_complex16_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_COMPLEX16, handle, predefs, alignments, extents);
	}
	get_mpi_complex32_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_COMPLEX32, handle, predefs, alignments, extents);
	}
	get_mpi_logical1_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_LOGICAL1, handle, predefs, alignments, extents);
	}
	get_mpi_logical2_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_LOGICAL2, handle, predefs, alignments, extents);
	}
	get_mpi_logical4_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_LOGICAL4, handle, predefs, alignments, extents);
	}
	get_mpi_logical8_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_LOGICAL8, handle, predefs, alignments, extents);
	}
	get_mpi_logical16_code___ (&handle);
	if (handle != fortranNull)
	{
		InsertFortranPredef(MUST_MPI_LOGICAL16, handle, predefs, alignments, extents);
	}
#endif

	//=8) Bound markers
WRAP_MPI_CALL_PREFIX
#ifdef HAVE_MPI_UB
	InsertCPredef(MUST_MPI_UB, MPI_UB, predefs, alignments, extents);
    alignments[MUST_MPI_UB]=1;
#endif
#ifdef HAVE_MPI_LB
	InsertCPredef(MUST_MPI_LB, MPI_LB, predefs, alignments, extents);
    alignments[MUST_MPI_LB]=1;
#endif
WRAP_MPI_CALL_POSTFIX

	//==Call the API call to propagate the message to the I_DatatypeTrack
	propagatePredefinedDatatypesP fP;
	if (getWrapperFunction ("propagatePredefinedDatatypes", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
	{
		int* enumValues = new int [predefs.size()];
		MustDatatypeType* handleValues = new MustDatatypeType [predefs.size()];
		int* alignmentValues = new int [predefs.size()];
		MustAddressType* extentValues = new MustAddressType [predefs.size()];

		std::map<MustMpiDatatypePredefined, MustDatatypeType>::iterator iter;
		int i = 0;
		for (iter = predefs.begin(); iter != predefs.end(); iter++, i++)
		{
			enumValues[i] = (int)iter->first;
			handleValues[i] = iter->second;

			//Extent and Alignment
			assert (alignments.find (iter->first) != alignments.end()); //Internal error, did not calculate an alignment
			assert (extents.find (iter->first) != extents.end()); //Internal error, did not calculate an extent
			alignmentValues[i] = alignments[iter->first];
			extentValues[i] = (MustAddressType)extents[iter->first];
		}

		(*fP) (
            pId,
			MUST_Type_m2i (MPI_DATATYPE_NULL),
			predefs.size(),
			enumValues,
			handleValues,
			extentValues,
			alignmentValues
			);

		if (enumValues)
		    delete[] enumValues;
		if (handleValues)
		    delete[] handleValues;
		if (alignmentValues)
		    delete[] alignmentValues;
		if (extentValues)
		    delete[] extentValues;
	}
	else
	{
		////std::cout << "ERROR: failed to get \"propagatePredefinedDatatypes\" function pointer from wrapper. Did you forgot to load the API group MUST_Resource_API ? Aborting now." << std::endl;
		////return GTI_ANALYSIS_FAILURE;
		//We fail silently here, the function won't exist if no datatype tracker is mapped ....
	}

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
