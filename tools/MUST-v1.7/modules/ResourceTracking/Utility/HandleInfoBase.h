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
 * @file HandleInfoBase.h
 *       @see HandleInfoBase.
 *
 *  @date 23.06.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Destructable.h"
#include "BaseIds.h"
#include "ResourceApi.h"

#include <map>
#include <string>
#include <list>

#ifndef HANDLEINFOBASE_H
#define HANDLEINFOBASE_H

namespace must
{
    /**
     * Base class for all handles informations (e.g. must::Datatype which implements must::I_Datatype).
     *
     * Provides reference counting and implements an erase (e.g. must::I_Destructable::erase)
     * as well as an additional incRefCount. Uses two reference counts, one for MPI usages, i.e. how
     * often this handle was returned by MPI and needs to be freed accordingly and a "user" reference
     * count that tracks how often a module asked for a persistent information.
     */
    class HandleInfoBase : public virtual I_Destructable
    {
    public:

        /**
         * Constructor.
         * Initializes with MPI reference count 1 and user reference count 0.
         * @param resourceName is the name of the resource in question
         */
        HandleInfoBase (std::string resourceName);

        /**
         * Destructor.
         */
        virtual ~HandleInfoBase ();

        /**
         * Decrements the user reference count of this handle information
         * by one, if both reference counts hit 0 or less, the object is
         * deleted, with a call to the template method HandleInfoBase::destroy.
         * @return true if this was the last user reference, irrespective of whether there are still MPI references.
         */
        bool erase (void);

        /**
         * @see I_Destructable::copy.
         * Implemented by a call to HandleInfoBase::incRefCount.
         */
        bool copy (void);

        /**
         * Decrements the mpi reference count of this handle information
         * by one, if both reference counts hits 0 or less, the object is
         * deleted, with a call to the template method HandleInfoBase::destroy.
         * @return true if this was the last MPI reference, irrespective of whether there are still user references.
         */
        bool mpiErase (void);

        /**
         * Method for destroying this handle information.
         * Sets count of MPI references to 0, if this causes both references to hit 0 or less, the object is destroyed,
         * otherwise it is left.
         * @return true if the object was deallocated, false otherwise.
         */
        bool mpiDestroy (void);

        /**
         * Increments the user reference count.
         */
        void incRefCount (void);

        /**
         * Increments the MPI reference count.
         */
        void mpiIncRefCount (void);

        /**
         * Template that provides the name of this resource type, e.g. "Request", "Comm".
         * @return name of resource
         */
        virtual std::string getResourceName (void) = 0;

        /**
         * Template to print information on this resource.
         * Style is in createLogEvent friendly format.
         */
        virtual bool printInfo (
                std::stringstream &out,
                std::list<std::pair<MustParallelId,MustLocationId> > *pReferences) = 0;

        /**
         * Notification of a new Tracker that started.
         * Used to determine when all trackers are
         * freed.
         */
        static void subscribeTracker (void);

        /**
         * Notification of a Tracker that terminated.
         * Used to determine when all trackers are
         * freed.
         */
        static void unsubscribeTracker (void);

        /**
         * Returns the remote id to use for this resource.
         * @return remote id.
         */
        uint64_t getRemoteId (void);

        /**
         * Marks this resource as forwarded to the
         * given place in this TBON level.
         *
         * This is needed for correct functionality of
         * HandleInfoBase::wasForwardedToPlace.
         *
         * Further, when this resource is destroyed,
         * all places to which we forwarded will
         * be notified of the destruction.
         *
         * IMPORTANT: the passed function pointer must be
         * the same for all calls to this function, a resource handle
         * only manages a single free function and not one per
         * forward. Also, this is a pointer to a function pointer!
         * I.e. to call the function HandleInfoBase needs to
         * dereference the pointer twice. The rational on this is that
         * with that design any tracker can unset the function pointer.
         * This is needed to avoid unnecessary calls to the function
         * when handles are forcefully removed at shutdown time.
         *
         * @param placeId to which we where forwarded
         * @param rank context used for the forward
         * @param freeFunction pointer to storage for a function pointer to a call that invoke the remote
         *  free of this resource.
         */
        void setForwardedToPlace (int placeId, int rank, passFreeAcrossP freeFunction);
        bool wasForwardedToPlace (int placeId, int rank);

        /**
         * Notifies HandleInfoBase that a shutdown of this place is going on, and that any
         * forwarding of frees to other places on the layer is not possible anymore.
         */
        static void disableFreeForwardingAcross (void);

    protected:
        int userRefCount; /**< The reference count used to determine the live span of this handle information, count of external (no MPI) uses of the handle.*/
        int mpiRefCount; /**< The reference count used to determine the live span of this handle information, count of MPI use of this handle (Rational: MPI may return the same handle multiple times, i.e. MPI_Comm_group for OpenMPI).*/

        std::set<std::pair<int, int> > myForwardedToPlaces; /**< Stores to which places in the same level this resource was forwarded (set of (placeId, rank) pairs).*/
        passFreeAcrossP myPassFreeAcross;

        static bool ourAllowFreeForwarding; /**< Global flag that specifies whether we should call the myPassFreeAcross function, reasons for it to be set to false include a starting shutdown of the place.*/

#ifdef MUST_DEBUG
        static std::map<std::string, std::map<HandleInfoBase*,bool> > ourHandles;
        static int ourNumTrackers;
#endif

        static void printLostHandles (void);

        /**
         * Called if the reference counting detects that this object can be freed.
         */
        virtual void deleteThis (void);

    }; /*class HandleInfoBase*/

} /*namespace must*/

#endif /*I_HANDLEINFOBASE_H*/
