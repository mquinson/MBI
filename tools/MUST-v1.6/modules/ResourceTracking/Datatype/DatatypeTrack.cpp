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
 * @file DatatypeTrack.cpp
 *       @see MUST::DatatypeTrack.
 *
 *  @date 10.02.2011
 *  @author Tobias Hilbrich, Joachim Protze
 */

#include "GtiMacros.h"

#include "DatatypeTrack.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(DatatypeTrack)
mFREE_INSTANCE_FUNCTION(DatatypeTrack)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DatatypeTrack)



//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// DatatypeTrack
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Constructor
//=============================
DatatypeTrack::DatatypeTrack (const char* instanceName)
: TrackBase<Datatype, I_Datatype, MustDatatypeType, MustMpiDatatypePredefined, DatatypeTrack, I_DatatypeTrack> (instanceName),
  CacheHitCount (0),
  CacheMissCount (0),
  myPredefinedInfos (),
  myUbInfo (NULL),
  myLbInfo (NULL)
{
    //Get the GroupTrack module
    if (myFurtherMods.size () < 1)
    {
        std::cerr << "Error: the CommTrack module needs the GroupTrack module as a child, but it was not specified." << std::endl;
        assert (0);
    }

    myBCoMod = (I_BaseConstants*) myFurtherMods[0];

    //Retrieve function pointers for passing resources across
    getWrapAcrossFunction("passDatatypePredefinedAcross", (GTI_Fct_t*) &myPassPredefinedAcrossFunc);
    getWrapAcrossFunction("passDatatypeContiguousAcross", (GTI_Fct_t*) &myPassContiguousAcrossFunc);
    getWrapAcrossFunction("passDatatypeVectorAcross", (GTI_Fct_t*) &myPassVectorAcrossFunc);
    getWrapAcrossFunction("passDatatypeHvectorAcross", (GTI_Fct_t*) &myPassHvectorAcrossFunc);
    getWrapAcrossFunction("passDatatypeIndexedAcross", (GTI_Fct_t*) &myPassIndexedAcrossFunc);
    getWrapAcrossFunction("passDatatypeHindexedAcross", (GTI_Fct_t*) &myPassHindexedAcrossFunc);
    getWrapAcrossFunction("passDatatypeStructAcross", (GTI_Fct_t*) &myPassStructAcrossFunc);
    getWrapAcrossFunction("passDatatypeIndexedBlockAcross", (GTI_Fct_t*) &myPassIndexedBlockAcrossFunc);
    getWrapAcrossFunction("passDatatypeResizedAcross", (GTI_Fct_t*) &myPassResizedAcrossFunc);
    getWrapAcrossFunction("passDatatypeSubarrayAcross", (GTI_Fct_t*) &myPassSubarrayAcrossFunc);
    getWrapAcrossFunction("passDatatypeDarrayAcross", (GTI_Fct_t*) &myPassDarrayAcrossFunc);

    getWrapAcrossFunction("passFreeDatatypeAcross", (GTI_Fct_t*) &myFreeDatatypeAcrossFunc);
}

//=============================
// Destructor
//=============================
DatatypeTrack::~DatatypeTrack ()
{
    //Notify HandleInfoBase of ongoing shutdown
    HandleInfoBase::disableFreeForwardingAcross();

    //We don't need to free the BaseConstants module, this will be done by TrackBase

#ifdef MUST_DEBUG
    if (CacheHitCount+CacheMissCount>0)
        std::cout << "Cache matches: " << CacheHitCount << " misses: " << CacheMissCount << std::endl;
#endif
}

//=============================
// commit
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::commit (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType type
)
{
    HandleMap::iterator pos = findUserHandle (pId, type);
    //If not a user handle, nothing to do (we do not detect errrors)
    if (pos == myUserHandles.end())
        return GTI_ANALYSIS_SUCCESS;

    //Apply the commit
    pos->second->commit(pId, lId);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// free
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::free (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType type
)
{
//     HandleMap::iterator pos = findUserHandle (pId, type);
// 
//     //If not a user handle, nothing to do (we do not detect errrors)
//     if (pos == myUserHandles.end())
//         return GTI_ANALYSIS_SUCCESS;
// 
//     //Apply the free
//     pos->second->erase();
//     myUserHandles.erase(pos);
//     myLastQuery = myUserHandles.end();
    removeUserHandle(pId, type);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeContiguous
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeContiguous (
        MustParallelId pId,
        MustLocationId lId,
        int count,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullContiguousTypeInfo *info = new FullContiguousTypeInfo (
            this,
            pId,
            lId,
            count,
            oldInfos,
            myPassContiguousAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeHindexed
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeHindexed (
        MustParallelId pId,
        MustLocationId lId,
        int count,
        const int* arrayOfBlocklengths,
        const MustAddressType* arrayOfDisplacements,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullHIndexedTypeInfo *info = new FullHIndexedTypeInfo (
            this,
            pId,
            lId,
            count,
            arrayOfBlocklengths,
            arrayOfDisplacements,
            oldInfos,
            myPassHindexedAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeIndexed
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeIndexed (
        MustParallelId pId,
        MustLocationId lId,
        int count,
        const int* arrayOfBlocklengths,
        const int* arrayOfDisplacements,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullIndexedTypeInfo *info = new FullIndexedTypeInfo (
            this,
            pId,
            lId,
            count,
            arrayOfBlocklengths,
            arrayOfDisplacements,
            oldInfos,
            myPassIndexedAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeHvector
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeHvector (
        MustParallelId pId,
        MustLocationId lId,
        int count,
        int blocklength,
        MustAddressType stride,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullHVectorTypeInfo *info = new FullHVectorTypeInfo (
            this,
            pId,
            lId,
            count,
            blocklength,
            stride,
            oldInfos,
            myPassHvectorAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeVector
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeVector (
        MustParallelId pId,
        MustLocationId lId,
        int count,
        int blocklength,
        int stride,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullVectorTypeInfo *info = new FullVectorTypeInfo (
            this,
            pId,
            lId,
            count,
            blocklength,
            stride,
            oldInfos,
            myPassVectorAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeStruct
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeStruct (
        MustParallelId pId,
        MustLocationId lId,
        int count,
        const int* arrayOfBlocklengths,
        const MustAddressType* arrayOfDisplacements,
        const MustDatatypeType* oldTypes,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    if (count < 1)
        /**
         * @todo test count < 1 for other types? Yes
         */
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos;
    int i;
    for (i = 0; i < count; i++)
    {
        //old type reasonable ?
        oldInfos.push_back((Datatype*)getPersistentDatatype(pId, oldTypes[i]));
        if (oldInfos.back() == NULL)
            return GTI_ANALYSIS_SUCCESS;
    }

    //Handle the type creation
     FullStructTypeInfo *info = new FullStructTypeInfo (
             this,
             pId,
             lId,
             count,
             arrayOfBlocklengths,
             arrayOfDisplacements,
             oldInfos,
             myPassStructAcrossFunc);


    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeIndexedBlock
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeIndexedBlock (
        MustParallelId pId,
        MustLocationId lId,
        int count,
        int blocklength,
        const int* arrayOfDisplacements,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullIndexedBlockTypeInfo *info = new FullIndexedBlockTypeInfo (
            this,
            pId,
            lId,
            count,
            blocklength,
            arrayOfDisplacements,
            oldInfos,
            myPassIndexedBlockAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeResized
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeResized (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType lb,
        MustAddressType extent,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullResizedTypeInfo *info = new FullResizedTypeInfo (
            this,
            pId,
            lId,
            lb,
            extent,
            oldInfos,
            myPassResizedAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeSubarray
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeSubarray (
        MustParallelId pId,
        MustLocationId lId,
        int ndims,
        const int* arrayOfSizes,
        const int* arrayOfSubsizes,
        const int* arrayOfStarts,
        int order,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullSubarrayTypeInfo *info = new FullSubarrayTypeInfo (
            this,
            pId,
            lId,
            ndims,
            arrayOfSizes,
            arrayOfSubsizes,
            arrayOfStarts,
            order,
            oldInfos,
            myPassSubarrayAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// typeDarray
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::typeDarray (
        MustParallelId pId,
        MustLocationId lId,
        int size,
        int rank,
        int ndims,
        const int* arrayOfGsizes,
        const int* arrayOfDistribs,
        const int* arrayOfDargs,
        const int* arrayOfPsizes,
        int order,
        MustDatatypeType oldType,
        MustDatatypeType newType
)
{
    //new type already known as a handle ?
    //TODO: handle cases where MPI returns the same handle for equal datatypes being constructed by some rank!
    if (isAlreadyKnown (pId, newType))
        return GTI_ANALYSIS_SUCCESS;

    //old type reasonable ?
    std::vector<Datatype *> oldInfos (1,(Datatype*)getPersistentDatatype(pId, oldType));
    if (oldInfos[0] == NULL)
        return GTI_ANALYSIS_SUCCESS; 

    //Handle the type creation
    FullDarrayTypeInfo *info = new FullDarrayTypeInfo (
            this,
            pId,
            lId,
            size,
            rank,
            ndims,
            arrayOfGsizes,
            arrayOfDistribs,
            arrayOfDargs,
            arrayOfPsizes,
            order,
            oldInfos,
            myPassDarrayAcrossFunc);

    submitUserHandle(pId, newType, info);

    return GTI_ANALYSIS_SUCCESS;
}


//=============================
// getPredefinedName
//=============================
std::string DatatypeTrack::getPredefinedName (MustMpiDatatypePredefined predefined)
{
    switch (predefined)
    {
        case MUST_MPI_CHAR: return "MPI_CHAR"; break;
        case MUST_MPI_SHORT: return "MPI_SHORT"; break;
        case MUST_MPI_INT: return "MPI_INT"; break;
        case MUST_MPI_LONG: return "MPI_LONG"; break;
        case MUST_MPI_UNSIGNED_CHAR: return "MPI_UNSIGNED_CHAR"; break;
        case MUST_MPI_UNSIGNED_SHORT: return "MPI_UNSIGNED_SHORT"; break;
        case MUST_MPI_UNSIGNED: return "MPI_UNSIGNED"; break;
        case MUST_MPI_UNSIGNED_LONG: return "MPI_UNSIGNED_LONG"; break;
        case MUST_MPI_FLOAT: return "MPI_FLOAT"; break;
        case MUST_MPI_DOUBLE: return "MPI_DOUBLE"; break;
        case MUST_MPI_LONG_DOUBLE: return "MPI_LONG_DOUBLE"; break;
        case MUST_MPI_BYTE: return "MPI_BYTE"; break;
        case MUST_MPI_PACKED: return "MPI_PACKED"; break;
        case MUST_MPI_INTEGER: return "MPI_INTEGER"; break;
        case MUST_MPI_REAL: return "MPI_REAL"; break;
        case MUST_MPI_DOUBLE_PRECISION: return "MPI_DOUBLE_PRECISION"; break;
        case MUST_MPI_COMPLEX: return "MPI_COMPLEX"; break;
        case MUST_MPI_LOGICAL: return "MPI_LOGICAL"; break;
        case MUST_MPI_CHARACTER: return "MPI_CHARACTER"; break;
        case MUST_MPI_FLOAT_INT: return "MPI_FLOAT_INT"; break;
        case MUST_MPI_DOUBLE_INT: return "MPI_DOUBLE_INT"; break;
        case MUST_MPI_LONG_INT: return "MPI_LONG_INT"; break;
        case MUST_MPI_2INT: return "MPI_2INT"; break;
        case MUST_MPI_SHORT_INT: return "MPI_SHORT_INT"; break;
        case MUST_MPI_LONG_DOUBLE_INT: return "MPI_LONG_DOUBLE_INT"; break;
        case MUST_MPI_2REAL: return "MPI_2REAL"; break;
        case MUST_MPI_2DOUBLE_PRECISION: return "MPI_2DOUBLE_PRECISION"; break;
        case MUST_MPI_2INTEGER: return "MPI_2INTEGER"; break;
        case MUST_MPI_2COMPLEX: return "MPI_2COMPLEX"; break;
        case MUST_MPI_2DOUBLE_COMPLEX: return "MPI_2DOUBLE_COMPLEX"; break;
        case MUST_MPI_LONG_LONG_INT: return "MPI_LONG_LONG_INT"; break;
        case MUST_MPI_LONG_LONG: return "MPI_LONG_LONG"; break;
        case MUST_MPI_UNSIGNED_LONG_LONG: return "MPI_UNSIGNED_LONG_LONG"; break;
        case MUST_MPI_WCHAR: return "MPI_WCHAR"; break;
        case MUST_MPI_SIGNED_CHAR: return "MPI_SIGNED_CHAR"; break;
        case MUST_MPI_CXX_BOOL: return "MPI_CXX_BOOL"; break;
        case MUST_MPI_CXX_FLOAT_COMPLEX: return "MPI_CXX_FLOAT_COMPLEX"; break;
        case MUST_MPI_CXX_DOUBLE_COMPLEX: return "MPI_CXX_DOUBLE_COMPLEX"; break;
        case MUST_MPI_CXX_LONG_DOUBLE_COMPLEX: return "MPI_CXX_LONG_DOUBLE_COMPLEX"; break;
        case MUST_MPI_INTEGER1: return "MPI_INTEGER1"; break;
        case MUST_MPI_INTEGER2: return "MPI_INTEGER2"; break;
        case MUST_MPI_INTEGER4: return "MPI_INTEGER4"; break;
        case MUST_MPI_INTEGER8: return "MPI_INTEGER8"; break;
        case MUST_MPI_INTEGER16: return "MPI_INTEGER16"; break;
        case MUST_MPI_REAL2: return "MPI_REAL2"; break;
        case MUST_MPI_REAL4: return "MPI_REAL4"; break;
        case MUST_MPI_REAL8: return "MPI_REAL8"; break;
        case MUST_MPI_REAL16: return "MPI_REAL16"; break;
        case MUST_MPI_DOUBLE_COMPLEX: return "MPI_DOUBLE_COMPLEX"; break;
        case MUST_MPI_COMPLEX8: return "MPI_COMPLEX8"; break;
        case MUST_MPI_COMPLEX16: return "MPI_COMPLEX16"; break;
        case MUST_MPI_COMPLEX32: return "MPI_COMPLEX32"; break;
        case MUST_MPI_LOGICAL1: return "MPI_LOGICAL1"; break;
        case MUST_MPI_LOGICAL2: return "MPI_LOGICAL2"; break;
        case MUST_MPI_LOGICAL4: return "MPI_LOGICAL4"; break;
        case MUST_MPI_LOGICAL8: return "MPI_LOGICAL8"; break;
        case MUST_MPI_LOGICAL16: return "MPI_LOGICAL16"; break;
        case MUST_MPI_UB: return "MPI_UB"; break;
        case MUST_MPI_LB: return "MPI_LB"; break;
        case MUST_MPI_C_BOOL: return "MPI_C_BOOL"; break;
        case MUST_MPI_INT8_T: return "MPI_INT8_T"; break;
        case MUST_MPI_INT16_T: return "MPI_INT16_T"; break;
        case MUST_MPI_INT32_T: return "MPI_INT32_T"; break;
        case MUST_MPI_INT64_T: return "MPI_INT64_T"; break;
        case MUST_MPI_UINT8_T: return "MPI_UINT8_T"; break;
        case MUST_MPI_UINT16_T: return "MPI_UINT16_T"; break;
        case MUST_MPI_UINT32_T: return "MPI_UINT32_T"; break;
        case MUST_MPI_UINT64_T: return "MPI_UINT64_T"; break;
        case MUST_MPI_C_COMPLEX: return "MPI_C_COMPLEX"; break;
        case MUST_MPI_C_FLOAT_COMPLEX: return "MPI_C_FLOAT_COMPLEX"; break;
        case MUST_MPI_C_DOUBLE_COMPLEX: return "MPI_C_DOUBLE_COMPLEX"; break;
        case MUST_MPI_C_LONG_DOUBLE_COMPLEX: return "MPI_C_LONG_DOUBLE_COMPLEX"; break;
        case MUST_MPI_DATATYPE_UNKNOWN: return "Unknown Datatype"; break;
    }

    return "Unknown Datatype";
}


//=============================
// fillPredefinedInfos
//=============================
// template < typename FULL_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE >
// TrackBase<Datatype, I_Datatype, MustDatatypeType, MustMpiDatatypePredefined, DatatypeTrack, I_DatatypeTrack> (instanceName),
Datatype* DatatypeTrack::createPredefinedInfo (int predef, MustDatatypeType value)
{
    bool explicitLb=false;
    bool explicitUb=false;
    bool null = false;
    bool optional = false;
    bool forReduction = false;
    bool boundMarker = false;
    bool c = false;
    bool fortran = false;

    if (value == myNullValue){
        // MPI_DATATYPE_NULL
        null = true;
        c = true;
        fortran = false;
        optional = false;
        forReduction = false;
        boundMarker = false;
    }
    else
    {
        switch ((MustMpiDatatypePredefined) predef)
        {
            //Elementary C
            case MUST_MPI_CHAR:
            case MUST_MPI_SHORT:
            case MUST_MPI_INT:
            case MUST_MPI_LONG:
            case MUST_MPI_UNSIGNED_CHAR:
            case MUST_MPI_UNSIGNED_SHORT:
            case MUST_MPI_UNSIGNED:
            case MUST_MPI_UNSIGNED_LONG:
            case MUST_MPI_FLOAT:
            case MUST_MPI_DOUBLE:
            case MUST_MPI_LONG_DOUBLE:
                c = true;
                fortran = false;
                optional = false;
                forReduction = false;
                boundMarker = false;
                break;

                //Elementary C & Fortran
            case MUST_MPI_BYTE:
            case MUST_MPI_PACKED:
                c = true;
                fortran = true;
                optional = false;
                forReduction = false;
                boundMarker = false;
                break;

                //Elementary Fortran
            case MUST_MPI_INTEGER:
            case MUST_MPI_REAL:
            case MUST_MPI_DOUBLE_PRECISION:
            case MUST_MPI_COMPLEX:
            case MUST_MPI_LOGICAL:
            case MUST_MPI_CHARACTER:
                c = false;
                fortran = true;
                optional = false;
                forReduction = false;
                boundMarker = false;
                break;

                //Reduction types C
            case MUST_MPI_FLOAT_INT:
            case MUST_MPI_DOUBLE_INT:
            case MUST_MPI_LONG_INT:
            case MUST_MPI_2INT:
            case MUST_MPI_SHORT_INT:
            case MUST_MPI_LONG_DOUBLE_INT:
                c = true;
                fortran = false;
                optional = false;
                forReduction = true;
                boundMarker = false;
                break;

                //Reduction types Fortran (first three non-optional, last two optional)
            case MUST_MPI_2REAL:
            case MUST_MPI_2DOUBLE_PRECISION:
            case MUST_MPI_2INTEGER:
                c = false;
                fortran = true;
                optional = false;
                forReduction = true;
                boundMarker = false;
                break;
            case MUST_MPI_2COMPLEX:
            case MUST_MPI_2DOUBLE_COMPLEX:
                c = false;
                fortran = true;
                optional = true;
                forReduction = true;
                boundMarker = false;
                break;

                //Optional C
            case MUST_MPI_LONG_LONG_INT:
            case MUST_MPI_LONG_LONG:
            case MUST_MPI_UNSIGNED_LONG_LONG:
            case MUST_MPI_WCHAR:
            case MUST_MPI_SIGNED_CHAR:
            case MUST_MPI_CXX_BOOL:
            case MUST_MPI_CXX_FLOAT_COMPLEX:
            case MUST_MPI_CXX_DOUBLE_COMPLEX:
            case MUST_MPI_CXX_LONG_DOUBLE_COMPLEX:
            case MUST_MPI_C_BOOL:
            case MUST_MPI_INT8_T:
            case MUST_MPI_INT16_T:
            case MUST_MPI_INT32_T:
            case MUST_MPI_INT64_T:
            case MUST_MPI_UINT8_T:
            case MUST_MPI_UINT16_T:
            case MUST_MPI_UINT32_T:
            case MUST_MPI_UINT64_T:
            case MUST_MPI_C_COMPLEX:
            case MUST_MPI_C_FLOAT_COMPLEX:
            case MUST_MPI_C_DOUBLE_COMPLEX:
            case MUST_MPI_C_LONG_DOUBLE_COMPLEX:
                c = true;
                fortran = false;
                optional = true;
                forReduction = false;
                boundMarker = false;
                break;

                //Optional Fortran
            case MUST_MPI_INTEGER1:
            case MUST_MPI_INTEGER2:
            case MUST_MPI_INTEGER4:
            case MUST_MPI_INTEGER8:
            case MUST_MPI_INTEGER16:
            case MUST_MPI_REAL2:
            case MUST_MPI_REAL4:
            case MUST_MPI_REAL8:
            case MUST_MPI_REAL16:
            case MUST_MPI_DOUBLE_COMPLEX:
            case MUST_MPI_COMPLEX8:
            case MUST_MPI_COMPLEX16:
            case MUST_MPI_COMPLEX32:
            case MUST_MPI_LOGICAL1:
            case MUST_MPI_LOGICAL2:
            case MUST_MPI_LOGICAL4:
            case MUST_MPI_LOGICAL8:
            case MUST_MPI_LOGICAL16:
                c = false;
                fortran = true;
                optional = true;
                forReduction = false;
                boundMarker = false;
                break;

                //Bound markers
            case MUST_MPI_UB:
                c = true;
                fortran = true;
                optional = false;
                forReduction = false;
                boundMarker = true;
                explicitUb=true;
                break;
            case MUST_MPI_LB:
                c = true;
                fortran = true;
                optional = false;
                forReduction = false;
                boundMarker = true;
                explicitLb=true;
                break;
            case MUST_MPI_DATATYPE_UNKNOWN:
                c = false;
                fortran = false;
                optional = false;
                forReduction = false;
                boundMarker = false;
                break;
/*            default:
                    std::cerr << "Controll completness of base types in " << __FILE__ << "@" << __LINE__ << " missing predef has number " << predef << std::endl;
                    assert (0);*/

        }
    }

    FullBaseTypeInfo* info = new FullBaseTypeInfo (
            this,
            optional,
            forReduction,
            boundMarker,
            null,
            c,
            fortran,
            explicitLb,
            explicitUb,
            (MustMpiDatatypePredefined)predef,
            getPredefinedName((MustMpiDatatypePredefined)predef).c_str(),
            myPassPredefinedAcrossFunc);

    if (explicitLb==true)
            myLbInfo = info;
    if (explicitUb==true)
            myUbInfo = info;
    return info;
    ////DEBUG output
    //PredefinedMap::iterator iter2;
    //for (iter2 = myPredefineds.begin(); iter2 != myPredefineds.end(); iter2++)
    //{
    //    std::cout << "Predef: " << getPredefinedName (iter2->second)  << " value " << iter2->first << std::endl;
    //}
}


//=============================
// addPredefinedTypes
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addPredefinedTypes (
                    MustParallelId pId,
            		MustDatatypeType datatypeNull,
            		int numPredefs,
            		int* predefinedIds,
            		MustDatatypeType* predefinedValues,
            		MustAddressType *extents,
            		int *alignments)
{
	//== 1) Trigger TrackBase to create the handle to enum mapping
	// This will triger fillPredefinedInfos, which will create the full info
	// Structs for all base types.
	TrackBase<Datatype, I_Datatype, MustDatatypeType, MustMpiDatatypePredefined, DatatypeTrack, I_DatatypeTrack>::addPredefineds (
        pId,
		datatypeNull,
    		numPredefs,
    		predefinedIds,
    		predefinedValues
    		);

	//== 2) Update the extents and alignments according to the given values
// 	std::map <MustDatatypeType, Datatype*>::iterator iter;
	PredefinedInfos::iterator iter;
	for (int i = 0; i < numPredefs; i++)
	{
		iter = myPredefineds.find(predefinedValues[i]);

		if (iter == myPredefineds.end())
		{
#ifdef MUST_DEBUG
			std::cerr<<"Internal warning in " << __FILE__ << "@" << __LINE__ << " for mpi type (must enum) " << predefinedIds[i] << ", there is no full info for this type, likely due to two MPI datatypes havin the same handle (e.g. MPI_LONG_LONG and MPI_LONG_LONG_INT in OpenMPI)."<< std::endl;
#endif
			continue;
		}

		((FullBaseTypeInfo*)iter->second)->setSizes(extents[i], alignments[i]);
// 		std::cout << "DT Info, " << iter->second->getPredefinedName() << " extent=" << iter->second->isNull() << " alignment=" << iter->second->getAlignment() << std::endl;
	}

	return GTI_ANALYSIS_SUCCESS;
}


//=============================
// getUbInfo
//=============================
FullBaseTypeInfo* DatatypeTrack::getUbInfo (void)
{
	return myUbInfo;
}

//=============================
// getLbInfo
//=============================
FullBaseTypeInfo* DatatypeTrack::getLbInfo (void)
{
	return myLbInfo;
}

//=============================
// getBCoMod
//=============================
I_BaseConstants* DatatypeTrack::getBCoMod()
{
    return myBCoMod;
}

//=============================
// getDatatype
//=============================
I_Datatype * DatatypeTrack::getDatatype (
        MustParallelId pId,
        MustDatatypeType type)
{
    return getHandleInfo(pId, type);
}

//=============================
// getDatatype
//=============================
I_Datatype * DatatypeTrack::getDatatype (
        int rank,
        MustDatatypeType type)
{
    return getHandleInfo(rank, type);
}

//=============================
// getPersistentDatatype
//=============================
I_DatatypePersistent * DatatypeTrack::getPersistentDatatype (
        MustParallelId pId,
        MustDatatypeType type)
{
    Datatype* ret = getHandleInfo(pId, type);

    if (ret)
        ret->incRefCount();
    return ret;
}

//=============================
// getPersistentDatatype
//=============================
I_DatatypePersistent * DatatypeTrack::getPersistentDatatype (
        int rank,
        MustDatatypeType type)
{
    Datatype* ret = getHandleInfo(rank, type);
    ret->incRefCount();
    return ret;
}

//=============================
// getRemoteDatatype
//=============================
I_Datatype* DatatypeTrack::getRemoteDatatype (
        MustParallelId pId,
        MustRemoteIdType remoteId)
{
    return getRemoteDatatype (pId2Rank(pId), remoteId);
}

//=============================
// getRemoteDatatype
//=============================
I_Datatype* DatatypeTrack::getRemoteDatatype (
        int rank,
        MustRemoteIdType remoteId)
{
    Datatype* ret = getRemoteIdInfo(rank,remoteId);
    return ret;
}

//=============================
// getPersistentRemoteDatatype
//=============================
I_DatatypePersistent* DatatypeTrack::getPersistentRemoteDatatype (
        MustParallelId pId,
        MustRemoteIdType remoteId)
{
    return getPersistentRemoteDatatype (pId2Rank(pId), remoteId);
}

//=============================
// getPersistentRemoteDatatype
//=============================
I_DatatypePersistent* DatatypeTrack::getPersistentRemoteDatatype (
        int rank,
        MustRemoteIdType remoteId)
{
    Datatype* ret = getRemoteIdInfo(rank,remoteId);
    if (ret) ret->incRefCount();
    return ret;
}

//=============================
// getPersistentRemoteDatatype
//=============================
bool DatatypeTrack::passDatatypeAcross (
        MustParallelId pId,
        MustDatatypeType datatype,
        int toPlaceId)
{
    return passDatatypeAcross (pId2Rank(pId), datatype, toPlaceId);
}

//=============================
// getPersistentRemoteDatatype
//=============================
bool DatatypeTrack::passDatatypeAcross (
        int rank,
        MustDatatypeType datatype,
        int toPlaceId)
{
    Datatype* info = getHandleInfo(rank, datatype);

    return passDatatypeAcrossInternal (rank, info, toPlaceId, NULL, true, datatype);
}

//=============================
// getPersistentRemoteDatatype
//=============================
bool DatatypeTrack::passDatatypeAcross (
        int rank,
        I_Datatype* datatype,
        int toPlaceId,
        MustRemoteIdType *pOutRemoteId)
{
    //Valid info?
    if (!datatype)
        return false;

    //Cast to internal representation
    Datatype* info = (Datatype*) datatype;

    //Do we have a handle for this still?
    MustDatatypeType handle = 0;
    bool hasHandle = getHandleForInfo (rank, info, &handle);

    return passDatatypeAcrossInternal (rank, info, toPlaceId, pOutRemoteId, hasHandle, handle);
}

//=============================
// passDatatypeAcrossInternal
//=============================
bool DatatypeTrack::passDatatypeAcrossInternal (
        int rank,
        Datatype* datatype,
        int toPlaceId,
        MustRemoteIdType *pOutRemoteId,
        bool hasHandle,
        MustDatatypeType handle)
{
    //Do we have wrap-across at all?
    if (!myPassPredefinedAcrossFunc ||
            !myPassContiguousAcrossFunc ||
            !myPassVectorAcrossFunc ||
            !myPassHvectorAcrossFunc ||
            !myPassIndexedAcrossFunc ||
            !myPassHindexedAcrossFunc ||
            !myPassStructAcrossFunc ||
            !myPassIndexedBlockAcrossFunc ||
            !myPassResizedAcrossFunc ||
            !myPassSubarrayAcrossFunc ||
            !myPassDarrayAcrossFunc)
        return false;

    //Valid info?
    if (!datatype)
        return false;

    //Store the remote id
    if (pOutRemoteId)
        *pOutRemoteId = datatype->getRemoteId();

    //Did we already pass this type?
    if (datatype->wasForwardedToPlace(toPlaceId, rank))
        return true;

    //Pass base resources of the datatype
    if (!datatype->isNull() && !datatype->isPredefined())
    {
        myLIdMod->passLocationToPlace(datatype->getCreationPId(), datatype->getCreationLId(), toPlaceId);

        if (datatype->isCommited())
            myLIdMod->passLocationToPlace(datatype->getCommitPId(), datatype->getCommitLId(), toPlaceId);
    }

    std::list<I_Datatype*> bases = datatype->getReferencedTypes();
    std::list<I_Datatype*>::iterator baseIter;

    for (baseIter = bases.begin(); baseIter != bases.end(); baseIter++)
    {
        I_Datatype* baseType = *baseIter;
        if (baseType && !baseType->isNull())
            passDatatypeAcross (rank, baseType, toPlaceId, NULL);
    }

    //Pass the actuall datatype across
    if (!datatype->passAcross (rank, hasHandle, handle, toPlaceId))
        return false;

    //Tell the comm that we passed it across
    datatype->setForwardedToPlace(toPlaceId, rank, myFreeDatatypeAcrossFunc);

    return true;
}

//=============================
// freeRemoteDatatype
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::freeRemoteDatatype (
        int rank,
        MustRemoteIdType remoteId)
{
    removeRemoteResource(rank, remoteId);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypePredefined
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypePredefined (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        int isOptional, /*bool*/
        int isForReduction, /*bool*/
        int isBoundMarker, /*bool*/
        int isNull, /*bool*/
        int isC, /*bool*/
        int isFortran, /*bool*/
        int hasExplicitLb, /*bool*/
        int hasExplicitUb, /*bool*/
        int predefValue, /*MustMpiDatatypePredefined*/
        MustAddressType extent,
        int alignment)
{
    //Create the resource
    FullBaseTypeInfo* resource = new FullBaseTypeInfo (
            this,
            (bool) isOptional,
            (bool) isForReduction,
            (bool) isBoundMarker,
            (bool) isNull,
            (bool) isC,
            (bool) isFortran,
            (bool) hasExplicitLb,
            (bool) hasExplicitUb,
            (MustMpiDatatypePredefined) predefValue,
            getPredefinedName((MustMpiDatatypePredefined) predefValue).c_str(),
            myPassPredefinedAcrossFunc);

    //Set alignment and extent
    resource->setSizes(extent, alignment);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeContiguous
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeContiguous (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int count,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullContiguousTypeInfo* resource = new FullContiguousTypeInfo (
            this,
            creationPId,
            creationLId,
            count,
            baseTypeInfo,
            myPassContiguousAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeVector
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeVector (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int count,
        int blocklength,
        int stride,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullVectorTypeInfo* resource = new FullVectorTypeInfo (
            this,
            creationPId,
            creationLId,
            count,
            blocklength,
            stride,
            baseTypeInfo,
            myPassVectorAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeHvector
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeHvector (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int count,
        int blocklength,
        MustAddressType stride,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullHVectorTypeInfo* resource = new FullHVectorTypeInfo (
            this,
            creationPId,
            creationLId,
            count,
            blocklength,
            stride,
            baseTypeInfo,
            myPassHvectorAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeIndexed
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeIndexed (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int count,
        int* blocklengths,
        int* displacements,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullIndexedTypeInfo* resource = new FullIndexedTypeInfo (
            this,
            creationPId,
            creationLId,
            count,
            blocklengths,
            displacements,
            baseTypeInfo,
            myPassIndexedAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeHindexed
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeHindexed (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int count,
        int* blocklengths,
        MustAddressType* displacements,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullHIndexedTypeInfo* resource = new FullHIndexedTypeInfo (
            this,
            creationPId,
            creationLId,
            count,
            blocklengths,
            displacements,
            baseTypeInfo,
            myPassHindexedAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeStruct
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeStruct (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int count,
        int* blocklengths,
        MustAddressType* displacements,
        MustRemoteIdType* baseTypes)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;

    if (count > 0)
    {
        baseTypeInfo.resize(count);
    }

    for (int i = 0; i < count; i++)
    {
        baseTypeInfo[i] = getRemoteIdInfo (rank,baseTypes[i]);

        if (!baseTypeInfo[i])
        {
            std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << " for count=" << i << std::endl;
            assert (0);
            return GTI_ANALYSIS_FAILURE;
        }
        else
        {
            (baseTypeInfo[i])->incRefCount();
        }
    }

    //Create the resource
    FullStructTypeInfo* resource = new FullStructTypeInfo (
            this,
            creationPId,
            creationLId,
            count,
            blocklengths,
            displacements,
            baseTypeInfo,
            myPassStructAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeIndexedBlock
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeIndexedBlock (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int count,
        int blocklength,
        int* displacements,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullIndexedBlockTypeInfo* resource = new FullIndexedBlockTypeInfo (
            this,
            creationPId,
            creationLId,
            count,
            blocklength,
            displacements,
            baseTypeInfo,
            myPassIndexedBlockAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeResized
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeResized (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        MustAddressType lb,
        MustAddressType extent,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullResizedTypeInfo* resource = new FullResizedTypeInfo (
            this,
            creationPId,
            creationLId,
            lb,
            extent,
            baseTypeInfo,
            myPassResizedAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeSubarray
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeSubarray (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int ndims,
        int* sizes,
        int* subsizes,
        int* starts,
        int order,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullSubarrayTypeInfo* resource = new FullSubarrayTypeInfo (
            this,
            creationPId,
            creationLId,
            ndims,
            sizes,
            subsizes,
            starts,
            order,
            baseTypeInfo,
            myPassSubarrayAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addRemoteDatatypeDarray
//=============================
GTI_ANALYSIS_RETURN DatatypeTrack::addRemoteDatatypeDarray (
        int rank,
        int hasHandle,
        MustDatatypeType typeHandle,
        MustRemoteIdType remoteId,
        //
        MustParallelId creationPId,
        MustLocationId creationLId,
        int isCommited,
        MustParallelId commitPId,
        MustLocationId commitLId,
        int commSize,
        int commRank,
        int ndims,
        int* gsizes,
        int* distribs,
        int* dargs,
        int* psizes,
        int order,
        MustRemoteIdType baseType)
{
    //Get base type
    std::vector<Datatype*> baseTypeInfo;
    baseTypeInfo.resize(1);
    baseTypeInfo[0] = getRemoteIdInfo (rank,baseType);

    if (!baseTypeInfo[0])
    {
        std::cerr << "Internal error in: " << __FILE__ << ":" << __LINE__ << std::endl;
        assert (0);
        return GTI_ANALYSIS_FAILURE;
    }
    else
    {
        (baseTypeInfo[0])->incRefCount();
    }

    //Create the resource
    FullDarrayTypeInfo* resource = new FullDarrayTypeInfo (
            this,
            creationPId,
            creationLId,
            commSize,
            commRank,
            ndims,
            gsizes,
            distribs,
            dargs,
            psizes,
            order,
            baseTypeInfo,
            myPassDarrayAcrossFunc);

    //Set commit state
    if  (isCommited)
        resource->commit(commitPId, commitLId);

    //Register the new remote datatype
    submitRemoteResource(rank, remoteId, hasHandle, typeHandle, resource);

    return GTI_ANALYSIS_SUCCESS;
}


/*EOF*/
