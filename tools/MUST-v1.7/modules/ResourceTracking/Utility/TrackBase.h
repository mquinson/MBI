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
 * @file TrackBase.h
 *       @see MUST::TrackBase.
 *
 *  @date 10.02.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"
#include "HandleInfoBase.h"
#include "MustTypes.h"
#include "I_TrackBase.h"

#include <map>
#include <list>

#ifndef TRACKBASE_H
#define TRACKBASE_H

using namespace gti;

namespace must
{
	/**
	 * Base class for handle/resource tracking modules.
	 * Already inherits from gti::ModuleBase.
	 *
	 * Example of usage:
	 * @code
	 class RequestTrack : public TrackBase<FullRequestInfo, MustRequestType, MustMpiRequestPredefined, RequestTrack, I_RequestTrack>
    {
    public:

    }
	 @endcode
	 */
	template <
		typename FULL_INFO, //Type of full info, maintained with pointers
		typename I_INFO, //Type of info interface, maintained with pointers
		typename HANDLE_TYPE, //Type used to store the value of a handle
		typename PREDEFINED_ENUM, //Enumeration used to identify predefined handles
		class SUPER, //Class inheriting from TrackBase
		class INTERFACE //Interface to implement by SUPER
		>
	class TrackBase : public gti::ModuleBase<SUPER, INTERFACE>, public virtual must::I_TrackBase<I_INFO>
	{
	public:
		/**
		 * Constructor.
		 * @param instanceName name of this module instance.
		 */
		TrackBase (const char* instanceName);

		/**
		 * Destructor.
		 */
		virtual ~TrackBase (void);

        /**
         * Adds the integer (HANDLE_TYPE) values for all predefined
         * (named) handles for requests.
         *
         * @param pId of the context.
         * @param nullValue value of MPI_<HANDLE_NAME>_NULL.
         * @param numPredefs number of predefined non null datatypes being sent.
         * @param predefinedIds array of value of PREDEFINED_ENUM for each predefined type, array size is numPredefs.
         * @param predefinedValues array of integer handles for the predefined types.
         *
         */
        GTI_ANALYSIS_RETURN addPredefineds (
                MustParallelId pId,
                HANDLE_TYPE nullValue,
                int numPredefs,
                int* predefinedIds,
                HANDLE_TYPE* predefinedValues
        );

        /**
         * Adds the integer (HANDLE_TYPE) values for all predefined
         * (named) handles for requests.
         *
         * @param rank of the context.
         * @param nullValue value of MPI_<HANDLE_NAME>_NULL.
         * @param numPredefs number of predefined non null datatypes being sent.
         * @param predefinedIds array of value of PREDEFINED_ENUM for each predefined type, array size is numPredefs.
         * @param predefinedValues array of integer handles for the predefined types.
         *
         */
        GTI_ANALYSIS_RETURN addPredefineds (
                int rank,
                HANDLE_TYPE nullValue,
                int numPredefs,
                int* predefinedIds,
                HANDLE_TYPE* predefinedValues
        );

		/**
		 * Returns a list of all currently existing user handles.
		 * The returned list contains pairs of the rank that
		 * created the resource and the
		 * handle id of the resource.
		 *
		 * @return list of user handles
		 */
		std::list<std::pair<int, HANDLE_TYPE> > getUserHandles (void);

        /**
         * Searches for the given handle in all handle objects.
         * (User + Predefined + Null)
         * @param rank for handle.
         * @param handle to search for.
         * @return iterator position in myUserHandles if found, myUserHandles() otherwise.
         */
        FULL_INFO* getHandleInfo (MustParallelId pId, HANDLE_TYPE handle);

        /**
         * Searches for the given handle in all handle objects.
         * (User + Predefined + Null)
         * @param rank for handle.
         * @param handle to search for.
         * @return iterator position in myUserHandles if found, myUserHandles() otherwise.
         */
        FULL_INFO* getHandleInfo (int rank, HANDLE_TYPE handle);

        /**
         * Searches for a resource from a remote place that was registered
         * with the given remoteId.
         * @param rank context.
         * @param remoteId to search a resource for.
         * @return Pointer to the resource or NULL.
         */
        FULL_INFO* getRemoteIdInfo (int rank, MustRemoteIdType remoteId);

        /**
         * Searches whether the given info has an associated handle,
         * (User, Predefined, Null). If so it returns true and stores the handle
         * in pOutHandle. Otherwise, it returns false.
         *
         * This function must search through all user handles and is expensive
         * as a result.
         *
         * @param rank context.
         * @param info to find a handle for.
         * @param pOutHandle pointer to storage for the handle if found.
         * @return true if a handle exists, false otherwise.
         */
        bool getHandleForInfo (int rank, FULL_INFO* info, HANDLE_TYPE *pOutHandle);

        /**
         * @see interface.
         */
        virtual void notifyOfShutdown (void);

	protected:
        // HandleMap[rank, handle] = (info)
		typedef std::map <std::pair<int, HANDLE_TYPE>, FULL_INFO* > HandleMap;
        // PredefinedInfos[handle] = (info)
        typedef std::map < HANDLE_TYPE , FULL_INFO* > PredefinedInfos;
        // PredefinedMap[mustPredefHandle] = (handle, info)
        typedef std::map < int , std::pair<HANDLE_TYPE, FULL_INFO*> > PredefinedMap;
        // NullMap[rank] = (handle)
        typedef std::map < int, HANDLE_TYPE > NullMap;
        
		typedef std::pair<int, MustRemoteIdType> RemoteIdentifier;
		typedef std::pair <FULL_INFO*, std::pair<bool, HANDLE_TYPE> > RemoteResourceInfo;
        // RemoteMap[rank, remotehandle] = (info, bool?, handle)
		typedef std::map < RemoteIdentifier , RemoteResourceInfo > RemoteMap; //Maps rank,remoteId to a resource and information on whether it has a handle, and if so its handle value

		NullMap myNullValues; /**< Values of MPI_handle_NULL.*/
		HANDLE_TYPE myNullValue; /**< Value of MPI_handle_NULL.*/
		FULL_INFO * myNullInfo; /**< Info object for MPI_handle_NULL.*/
		PredefinedInfos myPredefineds; /**< Used to identify named constants, initialized by addPredefineds. */
		PredefinedMap myPredefinedMap;
		HandleMap myUserHandles; /**< User-defined types.*/
		typename HandleMap::iterator myLastQuery; /** Used to speed up repeated queries for the same handle. */
		RemoteMap myRemoteRes; /**< Resources from remote places on the same level.*/

		I_ParallelIdAnalysis* myPIdMod; /**< Module used to convert pIds to ranks.*/
		I_LocationAnalysis* myLIdMod; /**< Module needed to pass location ids along.*/

		/**
		 * Vector of modules that remained after setting the parallel id module.
		 * These will be initialized in the constructor and destroyed in the
		 * destructor. If you want to take care about the destruction yourself
		 * you must set the respective element to NULL.
		 */
		std::vector<I_Module*> myFurtherMods;

		/**
		 * Searches for the given handle in the list of user handles (myUserHandles).
		 * @param pId parallel Id.
		 * @param handle to search for.
		 * @return iterator position in myUserHandles if found, myUserHandles.end() otherwise.
		 */
		typename HandleMap::iterator findUserHandle (MustParallelId pId, HANDLE_TYPE handle);

		/**
		 * Searches for the given handle in the list of user handles (myUserHandles).
		 * @param rank for handle.
		 * @param handle to search for.
		 * @return iterator position in myUserHandles if found, myUserHandles.end() otherwise.
		 */
		typename HandleMap::iterator findUserHandle (int rank, HANDLE_TYPE handle);

        /**
         * Checks whether the given handle (and pId) are already
         * known as a user or predefined handle.
         * @param pId parallel id for the request in question
         * @param handle to check for.
         * @return true if already known, false otherwise.
         */
        bool isAlreadyKnown (MustParallelId pId, HANDLE_TYPE handle);

        /**
         * inserts a new handleInfo to UserHandles list
         * @param pId parallel id for the request in question
         * @param handle to check for.
         * @param handleInfo info for that object.
         * @return true if successful, false otherwise.
         */
        bool submitUserHandle (MustParallelId pId, HANDLE_TYPE handle, FULL_INFO* handleInfo);

        /**
         * As the other submitUserHandle but with a rank instead of a pId.
         */
        bool submitUserHandle (int rank, HANDLE_TYPE handle, FULL_INFO* handleInfo);

        /**
         * Decrements the MPI ref count of the handle belonging to the given context and handle id,
         * and unsubscribes it if its MPI reference count becomes 0.
         * @param pId parallel id for the request in question
         * @param handle to check for.
         * @return true if successful, false otherwise.
         */
        bool removeUserHandle (MustParallelId pId, HANDLE_TYPE handle);

        /**
         * As the other removeUserHandle function but with a  rank instead of a pId.
         */
        bool removeUserHandle (int rank, HANDLE_TYPE handle);

        /**
         * Inserts a new handleInfo to that was received from a remote
         * place on the same level.
         *
         * If it has a associated handle besides its remote id it is added
         * to the list of user handles, such that it can be retrieved with
         * getHandleInfo.
         *
         * @param rank from which the resource is
         * @param remoteId of the resource
         * @param hasHandle true if this resource has an associated handle
         * @param handle value if this resource has a handle
         * @param handleInfo info for that object.
         * @return true if successful, false otherwise.
         */
        bool submitRemoteResource (int rank, MustRemoteIdType remoteId, bool hasHandle, HANDLE_TYPE handle, FULL_INFO* handleInfo);

        /**
         * Decrements the MPI ref count of the remote handle belonging to the given context,
         * and unsubscribes it if its MPI reference count becomes 0; which should always hold
         * for remote resource.
         * @param rank from which the resource is
         * @param remoteId of the resource
         * @return true if successful, false otherwise.
         */
        bool removeRemoteResource (int rank, MustRemoteIdType remoteId);

		/**
		 * Converts a pId to its rank,
		 * @param pId to convert.
		 * @return rank.
		 */
		int pId2Rank (MustParallelId pId);

		/**
		 * Hook method being called after addPredefineds is called.
		 * allows the creation of extra information for each predefined
		 * value.
		 *
		 * Will be called for each predefined handle AND for the Null handle.
		 *
		 * "value" is the predefined entry for the predefined to create, for
		 * null handles it is set to 0. "handle" is the handle value of the
		 * predefined or null handle. The null handle can easily be
		 * detected by testing handle against myNullValue, which is set
		 * before calling createPredefinedInfo for the null handle.
		 *
		 * @param value of predefined in enum, or 0 for null handles.
		 * @param handle value of predefined or null handle.
		 * @return information structure for this handle or null if no information is
		 *              associated with this handle.
		 */
		virtual FULL_INFO * createPredefinedInfo (int value, HANDLE_TYPE handle);

		/**
		 * Frees all user and predefined handles as well as the null handle.
		 * Is called in the destructor or by the class inheriting from track base.
		 * Calling this multiple times causes no errors.
		 *
		 * Use case is to delete any handles before the constructor is called. If
		 * destroying the handles causes something in the parent class to be
		 * deleted it may be necessary to free handles manually.
		 */
		void freeHandleMaps (void);
                /**
                 * Test whether a handle is predefined.
                 * @return true if provided info is for a predefined handle.
                 */
//                virtual bool isPredefined (FULL_INFO * info) = 0;
	};

#include "TrackBase.hpp"
} //end namespace MUST

#endif /*TRACKBASE_H*/
