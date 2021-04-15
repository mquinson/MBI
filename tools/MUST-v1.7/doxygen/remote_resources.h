/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2012-2015 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2012-2015 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @page RemoteResources MUST handling of resources that are passed between places of the same tool layer
 *
 * GTI allows intra layer communication in order to communicate between tool places within the same
 * TBON layer. As a result, MUSTs checks and trackers can pass information within a layer. Thus,
 * we require a mechanism to also pass information on MPI resources between places of the same TBON
 * layer. This page describes the mechanisms used to pass any resource handled by any resource tracker
 * to another place on the same layer.
 *
 * @see ResourceApi.h
 * @todo we need to precise what the rules are for forwarding remote resources upwards in the TBON,
 *            right now we do that, but its expensive and doesn't adds anything I fear
 *
 * @section RemoteResourcesExternalUse Passing Resources Across
 * Each resource tracker that provides forwarding of resources within the layer provides an interface
 * function. E.g., for I_CommTrack this is I_CommTrack::passCommAcross. These functions
 * need a handle and a rank to identify the resource in question, the "toPlaceId" specifies the rank to
 * send to.
 *
 * So users of the mechanism can directly invoke these interface functions if they want a local resource
 * to be available on a remote place on the same layer. This will only succeed if intra communication is
 * available in the layer, it fails otherwise. These functions will also pass all other resource description
 * on which they depend to the given place.
 *
 * @section RemoteResourcesInternal Internal Mechanisms
 *
 * The following describes the internal techniques and interfaces used to implement this functionality.
 *
 * @subsection RemoteResourcesInternalRemoteId Remote Ids
 *
 * Users should only forward resources for which an MPI handle exist, however, these resources may
 * depend on other resources for which non MPI handle exists anymore. E.g., a datatype to be passed
 * across may depend on another datatype that was already freed. As a result we need to be able
 * to pass resources that don't have a valid MPI handle across.
 *
 * In order to identify these resources that don't have a handle we use a so called remote ID. A remote
 * ID along with a rank can be used to refer to a resource on the remote side. All resources that
 * are passed across must provide such a remote id in order to be identified.
 *
 * We use the address of the resource instance as this remote ID. It is provided by
 * must::HandleBase::getRemoteId.
 *
 * All wrap-across functions that we use to forward a resource to a remote place on the same level
 * use the following signature:
 * @code
 * int pass<TYPE>Across (int rank, bool hasHandle, <MUST_MPI_HANDLE_TYPE> handle, MustRemoteIdType remoteId, <RESOURCE_INFOS>, toPlace);
 * @endcode
 *
 * @subsection RemoteResourcesInternalCaching Caching
 *
 * Passing resources across is a expensive operations due to the communication need and the extra
 * memory demand on the remote side. It should only be issues if really necessary.
 *
 * In order to reduce the communication demand we cache whether a certain resource was already
 * passed to a remote place. This basic functionality is implemented in must:HandleInfoBase
 * from which all resources inherit. It provides two functions:
 * - must::HandleInfoBase::setForwardedToPlace
 * - must::HandleInfoBase::wasForwardedToPlace
 *
 * The first function notifies the handle that it was forwarded to a given place. The functions also need
 * a rank argument in order to know their rank value on the remote side. must::HandleInfoBase
 * stores the resulting (toPlaceId, rank) pairs to recall which places already know about this resource.
 * In addition this function takes a function pointer that will be called by must::HandleInfoBase
 * when the resource is destroyed, i.e. both its MPI and its user reference count becomes 0. The second function
 * (must::HandleInfoBase::wasForwardedToPlace) returns whether this resource was already forwarded to
 * the given place.
 *
 * @subsection RemoteResourcesInternalTrackBase Support in TrackBase
 * When a tracker receives wrap across call for a resource that was passed from a remote place on the
 * same layer, it not only receives details about the resource, but also about its handle -- if
 * present -- and remote ID. must::TrackBase --from which all resource trackers inherit -- provides
 * functions and data structures to handle the additional information about the remote IDs.
 * As a data structure TrackBase uses a map that maps a (Rank, Remote ID) pair as key to the
 * resource and information on whether (and which) it has a handle. This is used to identify
 * requests to a resource that use a (Rank, Remote ID) pair instead of a (Rank, Handle) pair.
 * The two functions provided by TrackBase to manage remote resources are:
 * - must::TrackBase::submitRemoteResource
 * - must::TrackBase::removeRemoteResource
 *
 * The first adds a new resource that was received from remote, it automatically adds the resource
 * to its remote data structure and to the regular map of user data structures (if a handle is
 * associated). The later resources notifies must::TrackBase that the remote side from which
 * the resource was received has freed the resource. In that case must::TrackBase can delete
 * the resource from its map of remote resources and adapt the resources reference count
 * accordingly (More on freeing remote resources in the next section).
 *
 * @subsection RemoteResourcesInternalFree Freeing Remote Resources
 * On the sender side when a resources reference count becomes 0, must::HandleInfoBase
 * uses its internal information to notify all remote places to which the resource was sent
 * that this resource is not needed anymore. I.e., the local place will not forward any requests
 * with the remote ID of the resource to destroy in the future.
 *
 * Any remote places that receives this notification will remove the resource from the
 * remote ID mapping (must::TrackBase::removeRemoteResource) and decrement the
 * resources MPI reference count by 1 (It is 1 per default, i.e., the MPI referenz count
 * becomes 0). If no user of the resource exists on the remote side, the resource is
 * destroyed, if any user exists its user reference count will be >1, and thus it
 * will not be destroyed and remain valid on the remote side until it is not used anymore.
 *
 * The notification of a local resource destruction used the following wrap across functions:
 * @code
 * int pass<TYPE>FreeAcross (int rank, MustRemoteIdType remoteId, int toPlace);
 * @endcode
 *
 *
 */
