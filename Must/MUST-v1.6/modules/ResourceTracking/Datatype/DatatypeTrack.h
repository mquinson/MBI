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
 * @file DatatypeTrack.h
 *       @see MUST::DatatypeTrack.
 *
 *  @date 10.02.2011
 *  @author Tobias Hilbrich, Joachim Protze
 */

#include "ModuleBase.h"
#include "I_DatatypeTrack.h"
#include "I_BaseConstants.h"
#include "TrackBase.h"
#include "HandleInfoBase.h"
#include "Datatype.h"

#include <map>
#include <vector>
#include <string.h>

#include "DatatypeTrackDerivedStorage.h"

#ifndef DATATYPETRACK_H
#define DATATYPETRACK_H


using namespace gti;

namespace must
{
    
    /**
     * Implementation for I_DatatypeTrack.
     */
    class DatatypeTrack : public TrackBase<Datatype, I_Datatype, MustDatatypeType, MustMpiDatatypePredefined, DatatypeTrack, I_DatatypeTrack>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DatatypeTrack (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~DatatypeTrack (void);

        /**
         * @see I_DatatypeTrack::commit
         */
        GTI_ANALYSIS_RETURN commit (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType type
        );

        /**
         * @see I_DatatypeTrack::free
         */
        GTI_ANALYSIS_RETURN free (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType type
        );

        /**
         * @see I_DatatypeTrack::typeContiguous
         */
        GTI_ANALYSIS_RETURN typeContiguous (
                MustParallelId pId,
                MustLocationId lId,
                int count,
                MustDatatypeType oldType,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeHindexed
         */
        GTI_ANALYSIS_RETURN typeHindexed (
                MustParallelId pId,
                MustLocationId lId,
                int count,
                const int* arrayOfBlocklengths,
                const MustAddressType* arrayOfDisplacements,
                MustDatatypeType oldType,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeIndexed
         */
        GTI_ANALYSIS_RETURN typeIndexed (
                MustParallelId pId,
                MustLocationId lId,
                int count,
                const int* arrayOfBlocklengths,
                const int* arrayOfDisplacements,
                MustDatatypeType oldType,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeHvector
         */
        GTI_ANALYSIS_RETURN typeHvector (
                MustParallelId pId,
                MustLocationId lId,
                int count,
                int blocklength,
                MustAddressType stride,
                MustDatatypeType oldType,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeVector
         */
        GTI_ANALYSIS_RETURN typeVector (
                MustParallelId pId,
                MustLocationId lId,
                int count,
                int blocklength,
                int stride,
                MustDatatypeType oldType,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeStruct
         */
        GTI_ANALYSIS_RETURN typeStruct (
                MustParallelId pId,
                MustLocationId lId,
                int count,
                const int* arrayOfBlocklengths,
                const MustAddressType* arrayOfDisplacements,
                const MustDatatypeType* oldTypes,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeIndexedBlock
         */
        GTI_ANALYSIS_RETURN typeIndexedBlock (
                MustParallelId pId,
                MustLocationId lId,
                int count,
                int blocklength,
                const int* arrayOfDisplacements,
                MustDatatypeType oldType,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeResized
         */
        GTI_ANALYSIS_RETURN typeResized (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType lb,
                MustAddressType extent,
                MustDatatypeType oldType,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeSubarray
         */
        GTI_ANALYSIS_RETURN typeSubarray (
                MustParallelId pId,
                MustLocationId lId,
                int ndims,
                const int* arrayOfSizes,
                const int* arrayOfSubsizes,
                const int* arrayOfStarts,
                int order,
                MustDatatypeType oldType,
                MustDatatypeType newType
        );

        /**
         * @see I_DatatypeTrack::typeDarray
         */
        GTI_ANALYSIS_RETURN typeDarray (
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
        );

        /**
         * @see I_DatatypeTrack::addPredefinedTypes
         * Overloaded from TrackBase, triggers extra
         * calculations, pushes extents and alignments
         * into base type infos.
         */
        GTI_ANALYSIS_RETURN addPredefinedTypes (
                MustParallelId pId,
                MustDatatypeType datatypeNull,
                int numPredefs,
                int* predefinedIds,
                MustDatatypeType* predefinedValues,
                MustAddressType *extents,
                int *alignments
        );

        /**
         * @see I_DatatypeTrack::freeRemoteDatatype
         */
        GTI_ANALYSIS_RETURN freeRemoteDatatype (
                int rank,
                MustRemoteIdType remoteId);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypePredefined
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypePredefined (
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
                int alignment);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeContiguous
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeContiguous (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeVector
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeVector (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeHvector
         */
        gti::GTI_ANALYSIS_RETURN addRemoteDatatypeHvector (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeIndexed
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeIndexed (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeHindexed
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeHindexed (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeStruct
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeStruct (
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
                MustRemoteIdType* baseTypes);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeIndexedBlock
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeIndexedBlock (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeResized
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeResized (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeSubarray
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeSubarray (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::addRemoteDatatypeDarray
         */
        GTI_ANALYSIS_RETURN addRemoteDatatypeDarray (
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
                MustRemoteIdType baseType);

        /**
         * @see I_DatatypeTrack::getDatatype
         */
        I_Datatype * getDatatype (
                MustParallelId pId,
                MustDatatypeType type);

        /**
         * @see I_DatatypeTrack::getDatatype
         */
        I_Datatype * getDatatype (
                int rank,
                MustDatatypeType type);

        /**
         * @see I_DatatypeTrack::getPersistentDatatype
         */
        I_DatatypePersistent * getPersistentDatatype (
                MustParallelId pId,
                MustDatatypeType type);

        /**
         * @see I_DatatypeTrack::getPersistentDatatype
         */
        I_DatatypePersistent * getPersistentDatatype (
                int rank,
                MustDatatypeType type);

        /**
         * @see I_DatatypeTrack::getRemoteDatatype
         */
        I_Datatype* getRemoteDatatype (
                MustParallelId pId,
                MustRemoteIdType remoteId);

        /**
         * @see I_DatatypeTrack::getRemoteDatatype
         */
        I_Datatype* getRemoteDatatype (
                int rank,
                MustRemoteIdType remoteId);

        /**
         * @see I_DatatypeTrack::getPersistentRemoteDatatype
         */
        I_DatatypePersistent* getPersistentRemoteDatatype (
                MustParallelId pId,
                MustRemoteIdType remoteId);

        /**
         * @see I_DatatypeTrack::getPersistentRemoteDatatype
         */
        I_DatatypePersistent* getPersistentRemoteDatatype (
                int rank,
                MustRemoteIdType remoteId);

        /**
         * @see I_DatatypeTrack::passDatatypeAcross
         */
        bool passDatatypeAcross (
                MustParallelId pId,
                MustDatatypeType Datatype,
                int toPlaceId);

        /**
         * @see I_DatatypeTrack::passDatatypeAcross
         */
        bool passDatatypeAcross (
                int rank,
                MustDatatypeType comm,
                int toPlaceId);

        /**
         * @see I_DatatypeTrack::passDatatypeAcross
         */
        bool passDatatypeAcross (
                int rank,
                I_Datatype* datatype,
                int toPlaceId,
                MustRemoteIdType *pOutRemoteId);

        /**
         * Returns a pointer to the base constants module.
         * Must not be freed.
         * @return pointer or NULL in case of an error.
         */
        I_BaseConstants* getBCoMod(void);
        int CacheHitCount;
        int CacheMissCount;

        /**
         * Queries the tracker for the information structure for
         * MPI_UB.
         * @return information structure, managed by the tracker.
         */
        FullBaseTypeInfo* getUbInfo (void);

        /**
         * Queries the tracker for the information structure for
         * MPI_LB.
         * @return information structure, managed by the tracker.
         */
        FullBaseTypeInfo* getLbInfo (void);
        
        /**
         * Returns the name of a predefined datatype.
         * @param enum value of the predefined.
         * @return textual name, e.g. "MPI_INT".
         */
        std::string getPredefinedName (MustMpiDatatypePredefined predefined);

    protected:
        I_BaseConstants* myBCoMod; /**< Module used to query base constants.*/

        std::map <MustMpiDatatypePredefined, FullBaseTypeInfo*> myPredefinedInfos;

        FullBaseTypeInfo   *myUbInfo, /**< Pointer to information structure of MPI_UB.*/
                           *myLbInfo; /**< Pointer to information structure of MPI_LB.*/

        passFreeAcrossP myFreeDatatypeAcrossFunc; /**< Function pointer to use for freeing a type on a remote side (previously passed to another node in this level).*/
        passDatatypePredefinedAcrossP myPassPredefinedAcrossFunc;
        passDatatypeContiguousAcrossP myPassContiguousAcrossFunc;
        passDatatypeVectorAcrossP myPassVectorAcrossFunc;
        passDatatypeHvectorAcrossP myPassHvectorAcrossFunc;
        passDatatypeIndexedAcrossP myPassIndexedAcrossFunc;
        passDatatypeHindexedAcrossP myPassHindexedAcrossFunc;
        passDatatypeStructAcrossP myPassStructAcrossFunc;
        passDatatypeIndexedBlockAcrossP myPassIndexedBlockAcrossFunc;
        passDatatypeResizedAcrossP myPassResizedAcrossFunc;
        passDatatypeSubarrayAcrossP myPassSubarrayAcrossFunc;
        passDatatypeDarrayAcrossP myPassDarrayAcrossFunc;

        /**
         * Overrides MUST::TrackBase::fillPredefinedInfos.
         */
        void fillPredefinedInfos (void);

        /**
         * Used to initialize null and predefined infos.
         * @see TrackBase::createPredefinedInfo.
         * (Implementation of hook)
         */
        Datatype* createPredefinedInfo (int predef, MustDatatypeType handle);

        /**
         * Internal implementation for passDatatypeAcross.
         */
        bool passDatatypeAcrossInternal (
                int rank,
                Datatype* datatype,
                int toPlaceId,
                MustRemoteIdType *pOutRemoteId,
                bool hasHandle,
                MustDatatypeType handle);

    }; /*class DatatypeTrack */
} /*namespace MUST*/

#endif /*DATATYPETRACK_H*/
