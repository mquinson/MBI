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
 * @file ResourceApi.h
 * 		P call definition for MUST resource track API calls.
 *
 * @author Tobias Hilbrich
 * @date 22.02.2011
 */

#ifndef RESSOURCE_API_H
#define RESSOURCE_API_H

#include "I_DatatypeTrack.h"
#include "I_CommTrack.h"

/*---------------------------------------------
 *                        PREDEFINEDS
 *--------------------------------------------*/

//==Function used for propagateing predefined datatypes
inline int PpropagatePredefinedDatatypes (
        MustParallelId pId,
		MustDatatypeType datatypeNull,
		int numTypes,
		int* enumValues,
		MustDatatypeType* handleValues,
		MustAddressType *extents,
		int *alignments) {return 0;}

typedef int (*propagatePredefinedDatatypesP) (
        MustParallelId pId,
		MustDatatypeType datatypeNull,
		int numTypes,
		int* enumValues,
		MustDatatypeType* handleValues,
		MustAddressType *extents,
		int *alignments);

//==Function used for propagating predefined communicators
inline int PpropagateComms (
        MustParallelId pId,
		int reachableBegin,
		int reachableEnd,
		int worldSize,
		MustCommType commNull,
		MustCommType commSelf,
		MustCommType commWorld,
		int numWorlds,
		MustCommType* worlds) {return 0;}

typedef int (*propagateCommsP) (
        MustParallelId pId,
		int reachableBegin,
		int reachableEnd,
		int worldSize,
		MustCommType commNull,
		MustCommType commSelf,
		MustCommType commWorld,
		int numWorlds,
		MustCommType* worlds);

/*---------------------------------------------
 *                        FREE ACROSS
 *--------------------------------------------*/

//==Functions to free ressources that where passed across
inline int PpassFreeCommAcross (
        int rank,
        MustRemoteIdType remoteId
        ) {return 0;}

inline int PpassFreeDatatypeAcross (
        int rank,
        MustRemoteIdType remoteId
        ) {return 0;}

inline int PpassFreeErrAcross (
        int rank,
        MustRemoteIdType remoteId
        ) {return 0;}

inline int PpassFreeGroupAcross (
        int rank,
        MustRemoteIdType remoteId
        ) {return 0;}

inline int PpassFreeGroupTableAcross (
        int rank,
        MustRemoteIdType remoteId
        ) {return 0;}

inline int PpassFreeKeyvalAcross (
        int rank,
        MustRemoteIdType remoteId
        ) {return 0;}

inline int PpassFreeOpAcross (
        int rank,
        MustRemoteIdType remoteId
        ) {return 0;}

inline int PpassFreeRequestAcross (
        int rank,
        MustRemoteIdType remoteId
        ) {return 0;}

//Function pointer to any of the above
typedef int (*passFreeAcrossP) (
        int rank,
        MustRemoteIdType remoteId,
        int toPlace
        );

/*---------------------------------------------
 *                        COMM ACROSS
 *--------------------------------------------*/

//==Dummy P-Function
inline int PpassCommAcross (
        int rank,
        int hasHandle,
        MustCommType commHandle,
        MustRemoteIdType remoteId,

        int isNull,
        int isPredefined,
        int predefinedEnum, /*MustMpiCommPredefined*/

        //Only for user communicators (isKnown && !isNull && !isPredefined)
        int isCartesian,
        int isGraph,
        int isIntercomm,

        //Only for user and predefined comms (isKnown && !isNull)
        unsigned long long contextId,
        /*unsigned long long myNextContextId;*/ //Not needed, remote side can't derive from this

        MustRemoteIdType groupTableId, //Table information is sent seperately

        //Only for inter-communicators
        MustRemoteIdType groupTableIdRemote, /**< The remote group of the intercommunicator.*/

        //Only for user defined comms
        MustParallelId creationPId,
        MustLocationId creationLId,

        //For cartesian or graph comms
        int reorder,

        //Only for cartesian comms
        int ndims,
        int *dims,
        int *periods,

        //Only for graph comms
        int nnodes,
        int nedges,
        int *indices,
        int *edges) {return 0;}

//==Function pointer used to pass a communicator to a different place on the same level
typedef int (*passCommAcrossP) (
        int rank,
        int hasHandle,
        MustCommType commHandle,
        MustRemoteIdType remoteId,
        int isNull,
        int isPredefined,
        int predefinedEnum,
        int isCartesian,
        int isGraph,
        int isIntercomm,
        unsigned long long contextId,
        MustRemoteIdType groupTableId,
        MustRemoteIdType groupTableIdRemte,
        MustParallelId creationPId,
        MustLocationId creationLId,
        int reorder,
        int ndims,
        int *dims,
        int *periods,
        int nnodes,
        int nedges,
        int *indices,
        int *edges,
        int toPlaceId
        );

/*---------------------------------------------
 *                        GROUP ACROSS
 *--------------------------------------------*/

//== Dummy P-Functions for passing group tables across
inline int PpassGroupTableAcrossRep2 (
        int rank,
        MustRemoteIdType remoteId,
        int size,
        int* translation
        ) {return 0;}

inline int PpassGroupTableAcrossRep1 (
        int rank,
        MustRemoteIdType remoteId,
        int beginRank,
        int endRank
        ) {return 0;}

//==Function pointers used to pass a group table to a different place on the same level
typedef int (*passGroupTableAcrossRep2P) (
        int rank,
        MustRemoteIdType remoteId,
        int size,
        int* translation,
        int toPlaceId
        );

typedef int (*passGroupTableAcrossRep1P) (
        int rank,
        MustRemoteIdType remoteId,
        int beginRank,
        int endRank,
        int toPlaceId
        );

/*---------------------------------------------
 *                        REQUEST ACROSS
 *--------------------------------------------*/

//== Dummy P-Functions for passing a request across
inline int PpassRequestAcross (
        int rank,
        int hasHandle,
        MustRequestType requestHandle,
        MustRemoteIdType remoteId,
        int isActive,
        int isPersistent,
        int isSend,
        int isNull,
        int isCanceled,
        int isProcNull,
        int count,
        MustRemoteIdType datatype,
        int tag,
        MustRemoteIdType comm,
        int destSource,
        int sendMode,
        MustParallelId creationPId,
        MustParallelId activationPId,
        MustParallelId cancelPId,
        MustLocationId creationLId,
        MustLocationId activationLId,
        MustLocationId cancelLId
        ) {return 0;}

//==Function pointer used to pass a request to a different place on the same level
typedef int (*passRequestAcrossP) (
        int rank,
        int hasHandle,
        MustRequestType requestHandle,
        MustRemoteIdType remoteId,
        int isActive,
        int isPersistent,
        int isSend,
        int isNull,
        int isCanceled,
        int isProcNull,
        int count,
        MustRemoteIdType datatype,
        int tag,
        MustRemoteIdType comm,
        int destSource,
        int sendMode,
        MustParallelId creationPId,
        MustParallelId activationPId,
        MustParallelId cancelPId,
        MustLocationId creationLId,
        MustLocationId activationLId,
        MustLocationId cancelLId,
        int toPlaceId
        );

/*---------------------------------------------
 *                        DATATYPE ACROSS
 *--------------------------------------------*/

//-------------------------
//==Predefined datatype
inline int PpassDatatypePredefinedAcross (
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
        int alignment
        ) {return 0;}

//Pointer
typedef int (*passDatatypePredefinedAcrossP) (
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
        int alignment,
        int toPlaceId
        );

//-------------------------
//==Contiguous datatype
inline int PpassDatatypeContiguousAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeContiguousAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

//-------------------------
//==Vector datatype
inline int PpassDatatypeVectorAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeVectorAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

//-------------------------
//==Hvector datatype
inline int PpassDatatypeHvectorAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeHvectorAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

//-------------------------
//==Indexed datatype
inline int PpassDatatypeIndexedAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeIndexedAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

//-------------------------
//==Hindexed datatype
inline int PpassDatatypeHindexedAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeHindexedAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

//-------------------------
//==Struct datatype
inline int PpassDatatypeStructAcross (
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
        MustRemoteIdType* baseTypes
        ) {return 0;}

//Pointer
typedef int (*passDatatypeStructAcrossP) (
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
        MustRemoteIdType* baseTypes,
        int toPlaceId
        );

//-------------------------
//==IndexedBlock datatype
inline int PpassDatatypeIndexedBlockAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeIndexedBlockAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

//-------------------------
//==Resized datatype
inline int PpassDatatypeResizedAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeResizedAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

//-------------------------
//==Subarray datatype
inline int PpassDatatypeSubarrayAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeSubarrayAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

//-------------------------
//==Darray datatype
inline int PpassDatatypeDarrayAcross (
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
        MustRemoteIdType baseType
        ) {return 0;}

//Pointer
typedef int (*passDatatypeDarrayAcrossP) (
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
        MustRemoteIdType baseType,
        int toPlaceId
        );

#endif /*RESSOURCE_API_H*/

