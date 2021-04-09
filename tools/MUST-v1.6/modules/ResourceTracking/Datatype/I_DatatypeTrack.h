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
 * @file I_DatatypeTrack.h
 *       @see I_DatatypeTrack.
 *
 *  @date 24.01.2011
 *  @author Tobias Hilbrich, Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"

#include <list>
#include <iostream>

#include "I_Datatype.h"
#include "I_TrackBase.h"

#ifndef I_DATATYPETRACK_H
#define I_DATATYPETRACK_H


/**
 * Interface for tracking datatypes and information on their state.
 *
 * Important: This analysis module only tracks datatypes,
 * it provides no correctness checking. However, it tries
 * to handle incorrect actions as good as possible.
 *
 * Dependencies (in listed order):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 * - BaseConstants
 */
class I_DatatypeTrack : public gti::I_Module, public virtual must::I_TrackBase<must::I_Datatype>
{
public:
    /**
     * Commits the given datatype.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param type to commit, must be valid, not null, not predefined.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN commit (
            MustParallelId pId,
            MustLocationId lId,
            MustDatatypeType type
            ) = 0;

    /**
      * Frees the given datatype.
      *
      * @todo this needs to be extended at a later point.
      *            A freed datatype must not be freed immediately if
      *            it is still in use by a non-blocking communication.
      *            e.g.: MPI_Type_*
      *                    MPI_Type_commit
      *                    MPI_Isend
      *                    MPI_Type_free
      *                    MPI_Wait
      *
      * @param pId parallel id of the callsite.
      * @param lId location id of the callsite.
      * @param type to free, must be valid, not null, not predefined.
      * @return @see gti::GTI_ANALYSIS_RETURN.
      */
    virtual gti::GTI_ANALYSIS_RETURN free (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType type
                ) = 0;

    /**
     * Creates a MPI_Type_contiguous.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param count number of oldType's.
     * @param oldType that is repeated.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeContiguous (
            MustParallelId pId,
            MustLocationId lId,
            int count,
            MustDatatypeType oldType,
            MustDatatypeType newType
            ) = 0;

    /**
     * Creates a MPI_Type_hindexed.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param count number of blocks.
     * @param arrayOfBlocklengths block sizes.
     * @param arrayOfDisplacements byte displacement between blocks.
     * @param oldType that is repeated.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeHindexed (
            MustParallelId pId,
            MustLocationId lId,
            int count,
            const int* arrayOfBlocklengths,
            const MustAddressType* arrayOfDisplacements,
            MustDatatypeType oldType,
            MustDatatypeType newType
    ) = 0;

    /**
     * Creates a MPI_Type_indexed.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param count number of blocks.
     * @param arrayOfBlocklengths block sizes.
     * @param arrayOfDisplacements displacement between blocks.
     * @param oldType that is repeated.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeIndexed (
            MustParallelId pId,
            MustLocationId lId,
            int count,
            const int* arrayOfBlocklengths,
            const int* arrayOfDisplacements,
            MustDatatypeType oldType,
            MustDatatypeType newType
    ) = 0;

    /**
     * Creates a MPI_Type_hvector.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param count number of blocks.
     * @param blocklength length of each block.
     * @param stride byte between beginning of two blocks.
     * @param oldType that is repeated.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeHvector (
            MustParallelId pId,
            MustLocationId lId,
            int count,
            int blocklength,
            MustAddressType stride,
            MustDatatypeType oldType,
            MustDatatypeType newType
            ) = 0;

    /**
     * Creates a MPI_Type_vector.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param count number of blocks.
     * @param blocklength length of each block.
     * @param stride between beginning of two blocks.
     * @param oldType that is repeated.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeVector (
            MustParallelId pId,
            MustLocationId lId,
            int count,
            int blocklength,
            int stride,
            MustDatatypeType oldType,
            MustDatatypeType newType
    ) = 0;

    /**
      * Creates a MPI_Type_struct.
      *
      * @param pId parallel id of the callsite.
      * @param lId location id of the callsite.
      * @param count number of blocks.
      * @param arrayOfBlocklengths block sizes.
      * @param arrayOfDisplacements byte displacement between blocks.
      * @param oldTypes that are repeated.
      * @param newType created.
      * @return @see gti::GTI_ANALYSIS_RETURN.
      */
    virtual gti::GTI_ANALYSIS_RETURN typeStruct (
            MustParallelId pId,
            MustLocationId lId,
            int count,
            const int* arrayOfBlocklengths,
            const MustAddressType* arrayOfDisplacements,
            const MustDatatypeType* oldTypes,
            MustDatatypeType newType
            ) = 0;

    /**
     * Creates a MPI_Type_create_indexed_block.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param count number of blocks.
     * @param blocklength length of each block.
     * @param arrayOfDisplacements displacement between blocks.
     * @param oldType that is repeated.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeIndexedBlock (
            MustParallelId pId,
            MustLocationId lId,
            int count,
            int blocklength,
            const int* arrayOfDisplacements,
            MustDatatypeType oldType,
            MustDatatypeType newType
    ) = 0;

    /**
     * Creates a MPI_Type_create_resized.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param lb new lower bound of the datatype.
     * @param extent new extent of the datatype.
     * @param oldType that is resized.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeResized (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType lb,
            MustAddressType extent,
            MustDatatypeType oldType,
            MustDatatypeType newType
    ) = 0;

    /**
     * Creates a MPI_Type_create_subarray.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param ndims number of dimensions.
     * @param arrayOfSizes length of array for each dimension.
     * @param arrayOfSubsizes length of subarray for each dimension.
     * @param arrayOfStarts begin of subarray for each dimension.
     * @param order array storage order.
     * @param oldType that is repeated.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeSubarray (
            MustParallelId pId,
            MustLocationId lId,
            int ndims,
            const int* arrayOfSizes,
            const int* arrayOfSubsizes,
            const int* arrayOfStarts,
            int order,
            MustDatatypeType oldType,
            MustDatatypeType newType
    ) = 0;

    /**
     * Creates a MPI_Type_create_darray.
     *
     * @param pId parallel id of the callsite.
     * @param lId location id of the callsite.
     * @param size number of ranks.
     * @param rank choosen rank.
     * @param ndims number of dimensions.
     * @param arrayOfGsizes length of array for each dimension.
     * @param arrayOfDistribs distribution of array for each dimension.
     * @param arrayOfDargs distribution argument for each dimension.
     * @param arrayOfPsizes size of processgrid for each dimension.
     * @param order array storage order.
     * @param oldType that is repeated.
     * @param newType created.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN typeDarray (
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
    ) = 0;

    /**
     * Adds the integer (MustDatatypeType) values for all predefined
     * (named) handles for requests.
     *
     * @param datatypeNull value of MPI_DATATYPE_NULL.
     * @param numPredefs number of predefined non null datatypes being sent.
     * @param predefinedIds array of value of MustMpiDatatypePredefined for each predefined type, array size is numPredefs.
     * @param predefinedValues array of handles for the predefined types.
     *
     */
    virtual gti::GTI_ANALYSIS_RETURN addPredefinedTypes (
            MustParallelId pId,
            MustDatatypeType datatypeNull,
            int numPredefs,
            int* predefinedIds,
            MustDatatypeType* predefinedValues,
            MustAddressType *extents,
            int *alignments) = 0;

    /**
     * Returns a list of all currently known user handles.
     * Usage scenarios involve logging lost handles at finalize.
     * @return a list of pairs of the form (rank, handle id).
     */
    virtual std::list<std::pair<int, MustDatatypeType> > getUserHandles (void) = 0;

    /**
     * Returns a pointer to datatype information.
     * Is NULL if this is an unknown handle, note that
     * an MPI_TYPE_NULL handle returns a valid pointer though.
     *
     * Memory must not be freed and is valid until I_DatatypeTrack
     * receives the next event, if you need the information longer
     * query getPersistentDatatype instead.
     *
     * @param pId associated with the type.
     * @param type to query full information for.
     * @return NULL if there was no information for the given type.
     */
    virtual must::I_Datatype * getDatatype (
            MustParallelId pId,
            MustDatatypeType type)=0;

    /** As I_DatatypeTrack::getDatatype with rank instead of pid.*/
    virtual must::I_Datatype * getDatatype (
            int rank,
            MustDatatypeType type)=0;

    /**
     * Like I_DatatypeTrack::getDatatype, though returns a persistent information
     * that is valid until you erase it, i.e.:
     *@code
     I_DatatypePersistent * typeInfo = myDatatypeTrack->getPersistentDatatype (pId, handle);
     if (typeInfo == NULL) return;
     .... //Do something with typeInfo
     typeInfo->erase(); //Mark as not needed any longer
     *@endcode
     *
     * A reference count mechanism is used to implement this.
     *
     * @param pId associated with the type.
     * @param type to query full information for.
     * @return NULL if there was no information for the given type.
     */
    virtual must::I_DatatypePersistent * getPersistentDatatype (
            MustParallelId pId,
            MustDatatypeType type)=0;

    /** As I_DatatypeTrack::getPersistentDatatype with rank instead of pid.*/
    virtual must::I_DatatypePersistent * getPersistentDatatype (
            int rank,
            MustDatatypeType type)=0;

    /**
     * Like I_DatatypeTrack::getDatatype, but
     * with a remote id instead of a handle.
     */
    virtual must::I_Datatype* getRemoteDatatype (
            MustParallelId pId,
            MustRemoteIdType remoteId) = 0;

    /**
     * Like I_DatatypeTrack::getDatatype, but
     * with a remote id instead of a handle.
     */
    virtual must::I_Datatype* getRemoteDatatype (
            int rank,
            MustRemoteIdType remoteId) = 0;

    /**
     * Like I_DatatypeTrack::getPersistentDatatype, but
     * with a remote id instead of a handle.
     */
    virtual must::I_DatatypePersistent* getPersistentRemoteDatatype (
            MustParallelId pId,
            MustRemoteIdType remoteId) = 0;

    /**
     * Like I_DatatypeTrack::getPersistentDatatype, but
     * with a remote id instead of a handle.
     */
    virtual must::I_DatatypePersistent* getPersistentRemoteDatatype (
            int rank,
            MustRemoteIdType remoteId) = 0;

    /**
     * Passes the given datatype to the given place on this tool level.
     * @param pId context of the Datatype to pass
     * @param Datatype to pass
     * @param toPlaceId place to send to
     * @return true iff successful.
     *
     * Reasons for this to fail include the unavailability of intra layer
     * communication.
     */
    virtual bool passDatatypeAcross (
            MustParallelId pId,
            MustDatatypeType Datatype,
            int toPlaceId) = 0;

    /**
     * Like the other passDatatypeAcross version but with rank instead of a pId.
     */
    virtual bool passDatatypeAcross (
            int rank,
            MustDatatypeType comm,
            int toPlaceId) = 0;

    /**
         * Passes the given datatype to the given place on this tool level.
         *
         * * This is usually more expensive than the other passDatatypeAcross
         * versions as this requires that the tracker checks whether there
         * also exists a handle for this resource. To do that it has to
         * search though all its handles which may be expensive.
         *
         * @param pId context of the datatype to pass
         * @param datatype to pass (as information)
         * @param toPlaceId place to send to
         * @param pOutRemoteId pointer to storage for a remote id, is set to
         *               the remote id that is used to identify the resource on the
         *               remote side.
         * @return true iff successful.
         *
         * Reasons for this to fail include the unavailability of intra layer
         * communication.
         */
    virtual bool passDatatypeAcross (
            int rank,
            must::I_Datatype* datatype,
            int toPlaceId,
            MustRemoteIdType *pOutRemoteId) = 0;

    /**
     * Notification when a datatype from remote was freed on the remote side.
     * @param rank of the resource.
     * @param remoteId of the datatype.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN freeRemoteDatatype (
            int rank,
            MustRemoteIdType remoteId) = 0;

    /**
     * Adds a predefined datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypePredefined (
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
            int alignment) = 0;

    /**
     * Adds a contiguous datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeContiguous (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Adds a vector datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeVector (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Adds a hvector datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeHvector (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Adds an indexed datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeIndexed (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Adds an hindexed datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeHindexed (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Adds a struct datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeStruct (
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
            MustRemoteIdType* baseTypes) = 0;

    /**
     * Adds an indexed block datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeIndexedBlock (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Adds a  resized datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeResized (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Adds a subarray datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeSubarray (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Adds a subarray datatype from a remote place.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN addRemoteDatatypeDarray (
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
            MustRemoteIdType baseType) = 0;

    /**
     * Allows other modules to notify this module of an ongoing shutdown.
     * This influcences the behavior of passing free calls across to other layers.
     */
    virtual void notifyOfShutdown (void) = 0;
    
    virtual bool isPredefined(must::I_Datatype * info) {return info->isPredefined();}

}; /*class I_DatatypeTrack*/

#endif /*I_DATATYPETRACK_H*/
