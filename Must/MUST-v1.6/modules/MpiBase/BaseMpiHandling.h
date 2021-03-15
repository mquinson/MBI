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
 * @file BaseMpiHandling.h
 * 		Header for mpi_base_specification.xml.in.
 *
 * @author Tobias Hilbrich
 * @date 03.01.2011
 */

#ifndef BASE_MPI_HANDLING_H
#define BASE_MPI_HANDLING_H

#include <mpi.h>
#include <cstdio>
#include "mustFeaturetested.h"
#include "mustConfig.h"
#include "MustDefines.h"

/**
 * @todo MPI_Fint is not necessarily equal to int!
 */

/**
 * Returns the address of a location in memory.
 * @param location to return address for.
 * @return address of location.
 */
inline int64_t BaseMpi_GetAddress (const void *location)
{
    if (location == MPI_BOTTOM && MPI_BOTTOM != NULL)
        return MUST_BOTTOM;
#ifdef HAVE_MPI_IN_PLACE
    if (location == MPI_IN_PLACE)
        return MUST_IN_PLACE;
#endif
    MPI_Aint ret;
#ifdef HAVE_MPI_GET_ADDRESS
    PMPI_Get_address ((void*)location, &ret);
#else
    PMPI_Address ((void*)location, &ret);
#endif
    return (int64_t)ret;
}

/**
 * Returns the size of a communicator.
 * @param comm to return size for.
 * @return size of comm.
 */
inline int BaseMpi_CommSize (MPI_Comm comm)
{
    int ret;
    PMPI_Comm_size (comm, &ret);
    return ret;
}

/**
 * Returns the size of the "edges" array for MPI_Graph_create
 * and other calls with a similar argument.
 * @param index the array of indices for this call.
 * @param count number of nodes in the graph.
 * @return size of edges array.
 */
inline int BaseMpi_GraphEdgeCount (const int* index, int count)
{
	return index[count-1];
}

/**
 * Converts a communicator handle to an integer.
 * @param comm to convert.
 * @return integer.
 */
inline MustCommType BaseMpi_Comm2int (MPI_Comm comm)
{
	return MUST_Comm_m2i (comm);
}

/**
 * Converts a datatype handle to an integer.
 * @param type to convert.
 * @return integer.
 */
inline MustDatatypeType BaseMpi_Datatype2int (MPI_Datatype type)
{
	return MUST_Type_m2i (type);
}

/**
 * Converts a datatype handle (given with a pointer to it) to an integer.
 * @param type to convert.
 * @return integer.
 */
inline MustDatatypeType BaseMpi_DatatypeP2int (MPI_Datatype* type)
{
    MustDatatypeType ret = 0;
    if (type) ret = MUST_Type_m2i (*type);
    return ret;
}

/**
 * Converts an operation handle to an integer.
 * @param op to convert.
 * @return integer.
 */
inline MustOpType BaseMpi_Op2int (MPI_Op op)
{
	return MUST_Op_m2i (op);
}

/**
 * Converts a pointer to an operation handle to an integer.
 * @param op to convert.
 * @return integer.
 */
inline MustOpType BaseMpi_OpP2int (MPI_Op* op)
{
    MustOpType ret = 0;
    if (op) ret = MUST_Op_m2i (*op);
    return ret;
}

/**
 * Converts a pointer to a request handle to an integer.
 * @param request to convert.
 * @return integer.
 */
inline MustRequestType BaseMpi_RequestP2int (MPI_Request* request)
{
	MustRequestType ret = 0;
	if (request) ret = MUST_Request_m2i (*request);
	return ret;
}

/**
 * Converts a pointer to an communicator to an integer.
 * @param comm to convert.
 * @return integer.
 */
inline MustCommType BaseMpi_CommP2int (MPI_Comm* comm)
{
	MustCommType ret = 0;
	if (comm) ret = MUST_Comm_m2i (*comm);
	return ret;
}

/**
 * Converts a group to an integer.
 * @param group to convert.
 * @return integer.
 */
inline MustGroupType BaseMpi_Group2int (MPI_Group group)
{
	return MUST_Group_m2i (group);
}

/**
 * Converts a pointer to a group to an integer.
 * @param group to convert.
 * @return integer.
 */
inline MustGroupType BaseMpi_GroupP2int (MPI_Group* group)
{
	MustGroupType ret = 0;
	if (group) ret = MUST_Group_m2i (*group);
	return ret;
}

/**
 * Converts an errorhandler to an integer.
 * @param errorhandler to convert.
 * @return integer.
 */
inline MustErrType BaseMpi_Errhandler2int (MPI_Errhandler err)
{
	return MUST_Errhandler_m2i (err);
}

/**
 * Converts a pointer to an errorhandler to an integer.
 * @param errorhandler to convert.
 * @return integer.
 */
inline MustErrType BaseMpi_ErrhandlerP2int (MPI_Errhandler* err)
{
	MustErrType ret = 0;
	if (err) ret = MUST_Errhandler_m2i (*err);
	return ret;
}

/**
 * Returns the source of a pointer to a status.
 * @param status to convert.
 * @return integer, 0-N for a rank, -1 if not available (STATUS_IGNORE).
 */
inline int BaseMpi_StatusP2Source (MPI_Status* status)
{
#ifdef HAVE_MPI_STATUS_IGNORE
    if (status == MPI_STATUS_IGNORE)
        return -1;
#endif
    return status->MPI_SOURCE;
}

/**
 * Returns the tag of a pointer to a status.
 * @param status to convert.
 * @return integer, >0 tag, -1 if not available (STATUS_IGNORE).
 */
inline int BaseMpi_StatusP2Tag (MPI_Status* status)
{
#ifdef HAVE_MPI_STATUS_IGNORE
    if (status == MPI_STATUS_IGNORE)
        return -1;
#endif
	return status->MPI_TAG;
}

/**
 * Returns the error code of a status for the given pointer.
 * @param status to convert.
 * @return integer.
 */
inline int BaseMpi_StatusP2Error (MPI_Status* status)
{
#ifdef HAVE_MPI_STATUS_IGNORE
    if (status == MPI_STATUS_IGNORE)
        return -1;
#endif
	return status->MPI_ERROR;
}

/**
 * Deallocates an integer array that was allocated by the MPI handling.
 * @param array pointer to array that will be freed.
 */
inline void BaseMpi_FreeArray (int* array)
{
	if (array) delete [] array;
}

/**
 * Deallocates an uint64_t array that was allocated by the MPI handling.
 * @param array pointer to array that will be freed.
 */
inline void BaseMpi_FreeInt64Array (int64_t* array)
{
    if (array) delete [] array;
}
inline void BaseMpi_FreeUint64Array (uint64_t* array)
{
    if (array) delete [] array;
}

/**
 * Converts an array of requests into an array of integers, also allocates this array.
 * @param pOut pointer to integer pointer, the memory pointed to will be used
 *               to store the address of the allocated array used to store the result.
 * @param reqs array of requests to convert.
 * @param length number of entries in array to convert.
 */
inline void BaseMpi_Requests2int (MustRequestType** pOut, MPI_Request* reqs, int length)
{
	if (length > 0)
	{
		*pOut = new MustRequestType[length];
		for (int i = 0; i < length; i++)
		{
			(*pOut)[i] = MUST_Request_m2i (reqs[i]);
		}
	}
	else
	{
		*pOut = NULL;
	}
}

/**
 * Converts an array of datatypes into an array of integers, also allocates this array.
 * @param pOut pointer to integer pointer, the memory pointed to will be used
 *               to store the address of the allocated array used to store the result.
 * @param types array of datatypes to convert.
 * @param length number of entries in array to convert.
 */
inline void BaseMpi_Datatypes2int (MustDatatypeType** pOut, const MPI_Datatype* types, int length)
{
	if (length > 0 && types!=NULL)
	{
		*pOut = new MustDatatypeType[length];
		for (int i = 0; i < length; i++)
		{
			(*pOut)[i] = MUST_Type_m2i ((MPI_Datatype*)((void*)types[i])); //We vast const away here so we don't have to discern between const correct and non const correct MPI implementations
		}
	}
	else
	{
		*pOut = NULL;
	}
}

/**
 * Converts an array of statuses into an array of integers that contains their
 * error codes, also allocates this array.
 * @param pOut pointer to integer pointer, the memory pointed to will be used
 *               to store the address of the allocated array used to store the result.
 * @param statuses array of statuses to convert.
 * @param length number of entries in array to convert.
 */
inline void BaseMpi_Statuses2Error (int** pOut, MPI_Status* statuses, int length)
{
	if (length > 0
#ifdef HAVE_MPI_STATUSES_IGNORE
      && statuses!=MPI_STATUSES_IGNORE
#endif
  )
	{
		*pOut = new int[length];
		for (int i = 0; i < length; i++)
		{
			(*pOut)[i] = statuses[i].MPI_ERROR;
		}
	}
	else
	{
		*pOut = NULL;
	}
}

/**
 * Converts an array of statuses into an array of integers that contains their
 * tags, also allocates this array.
 * @param pOut pointer to integer pointer, the memory pointed to will be used
 *               to store the address of the allocated array used to store the result.
 * @param statuses array of statuses to convert.
 * @param length number of entries in array to convert.
 */
inline void BaseMpi_Statuses2Tag (int** pOut, MPI_Status* statuses, int length)
{
	if (length > 0
#ifdef HAVE_MPI_STATUSES_IGNORE
      && statuses!=MPI_STATUSES_IGNORE
#endif
  )
	{
		*pOut = new int[length];
		for (int i = 0; i < length; i++)
		{
			(*pOut)[i] = statuses[i].MPI_TAG;
		}
	}
	else
	{
		*pOut = NULL;
	}
}

/**
 * Converts an array of statuses into an array of integers that contains their
 * sources, also allocates this array.
 * @param pOut pointer to integer pointer, the memory pointed to will be used
 *               to store the address of the allocated array used to store the result.
 * @param statuses array of statuses to convert.
 * @param length number of entries in array to convert.
 */
inline void BaseMpi_Statuses2Source (int** pOut, MPI_Status* statuses, int length)
{
	if (length > 0
#ifdef HAVE_MPI_STATUSES_IGNORE
      && statuses!=MPI_STATUSES_IGNORE
#endif
  )
	{
		*pOut = new int[length];
		for (int i = 0; i < length; i++)
		{
			(*pOut)[i] = statuses[i].MPI_SOURCE;
		}
	}
	else
	{
		*pOut = NULL;
	}
}

/**
 * Converts one request in an array to an integer.
 * The given index is used to select the request to convert.
 * If the index is invalid (out of bounds) the function returns 0 and does not
 * attempts a conversion.
 * @param requests to convert one of.
 * @param count number of requests in the array.
 * @param index of the request to convert.
 * @return converted value if index was valid, 0 otherwise.
 */
inline MustRequestType BaseMpi_Request2intIndexed (MPI_Request* requests, int count, int index)
{
	if (index < 0 || index >= count)
		return 0;

	return MUST_Request_m2i (requests[index]);
}

/**
 * Converts an MPI_Aint (which may be something) into
 * something we can transfer.
 * @param a value to convert.
 * @return address casted to uint64_t.
 */
inline int64_t BaseMpi_Aint2uint64 (MPI_Aint a)
{
	return (int64_t) a;
}

/**
 * Converts an array of MPI_Aint's (which may be something) into
 * something we can transfer.
 * @param pOut pointer to pointer that will be used to allocated storage for the returned values.
 * @param values to convert.
 * @param count number of values to convert.
 * @return address casted to uint64_t.
 */
inline int64_t BaseMpi_Aints2uint64 (int64_t **pOut, const MPI_Aint* values, int count)
{
	if (count > 0)
	{
		*pOut = new int64_t[count];
		for (int i = 0; i < count; i++)
		{
			(*pOut)[i] = (int64_t) values[i];
		}
	}
	else
	{
		*pOut = NULL;
	}
	//TODO: void function?
	return 0;
}

/**
 * Returns the number of dimensions specified for the given communicator.
 * @param cartComm communicator that should be cartesian, but we have to take a lot of care not to cause a crash if it is no such comm.
 * @return number of dimensions that the cart comm has, 0 if there is some error.
 */
inline int BaseMPI_CommCartNumDims (MPI_Comm cartComm)
{
	int status;
	int ret;

	if (cartComm == MPI_COMM_NULL)
		return 0;

    if (PMPI_Topo_test (cartComm, &status) != MPI_SUCCESS)
    		return 0;

    if (status != MPI_CART)
    		return 0;

    if (PMPI_Cartdim_get (cartComm, &ret) != MPI_SUCCESS)
    		return 0;

    return ret;
}

/**
 * Returns the in-degree of a communicator.
 * @param comm to return in-degree for.
 * @return in-degree of comm.
 */
inline int BaseMpi_CommIndegree (MPI_Comm comm)
{
    int status, indegree, outdegree, weighted, ndims, rank, maxneighbors;
    int ret = 0;

    if (comm == MPI_COMM_NULL)
        return ret;

    if (PMPI_Topo_test (comm, &status) != MPI_SUCCESS)
        return ret;

    switch (status)
    {
        case MPI_CART:
            if (PMPI_Cartdim_get (comm, &ndims) != MPI_SUCCESS)
                return ret;
            ret = 2*ndims; //MPI-3, p315 says so
            break;
        case MPI_GRAPH:
            if (PMPI_Comm_rank(comm, &rank) != MPI_SUCCESS)
                return ret;
            if (PMPI_Graph_neighbors_count(comm, rank, &maxneighbors) != MPI_SUCCESS)
                return ret;
            ret = maxneighbors; //@todo this follows MPI-3, p315, but I am not yet sure whether this is correct
            break;
#if defined(HAVE_MPI_DIST_GRAPH_NEIGHBORS_COUNT) && defined (HAVE_MPI_DIST_GRAPH)
        case MPI_DIST_GRAPH:
            if (PMPI_Dist_graph_neighbors_count(
                    comm,
                    &indegree,
                    &outdegree,
                    &weighted) != MPI_SUCCESS)
                return ret;
            ret = indegree; //MPI-3 p316 bottom
            break;
#endif
        default:
            return ret;
    }

    return ret;
}

/**
 * Returns the out-degree of a communicator.
 * @param comm to return out-degree for.
 * @return out-degree of comm.
 */
inline int BaseMpi_CommOutdegree (MPI_Comm comm)
{
    int status, indegree, outdegree, weighted, ndims, rank, maxneighbors;
    int ret = 0;

    if (comm == MPI_COMM_NULL)
        return ret;

    if (PMPI_Topo_test (comm, &status) != MPI_SUCCESS)
        return ret;

    switch (status)
    {
        case MPI_CART:
            if (PMPI_Cartdim_get (comm, &ndims) != MPI_SUCCESS)
                return ret;
            ret = 2*ndims; //MPI-3, p315 says so
            break;
        case MPI_GRAPH:
            if (PMPI_Comm_rank(comm, &rank) != MPI_SUCCESS)
                return ret;
            if (PMPI_Graph_neighbors_count(comm, rank, &maxneighbors) != MPI_SUCCESS)
                return ret;
            ret = maxneighbors; //@todo this follows MPI-3, p315, but I am not yet sure whether this is correct
            break;
#if defined(HAVE_MPI_DIST_GRAPH_NEIGHBORS_COUNT) && defined (HAVE_MPI_DIST_GRAPH)
        case MPI_DIST_GRAPH:
            if (PMPI_Dist_graph_neighbors_count(
                    comm,
                    &indegree,
                    &outdegree,
                    &weighted) != MPI_SUCCESS)
                return ret;
            ret = outdegree; //MPI-3 p316 bottom
            break;
#endif
        default:
            return ret;
    }

    return ret;
}


#endif /*BASE_MPI_HANDLING_H*/
