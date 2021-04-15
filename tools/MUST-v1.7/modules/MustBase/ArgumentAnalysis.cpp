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
 * @file ArgumentAnalysis.cpp
 *       @see MUST::ArgumentAnalysis.
 *
 *  @date 28.02.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"

#include "ArgumentAnalysis.h"

using namespace must;

mGET_INSTANCE_FUNCTION(ArgumentAnalysis)
mFREE_INSTANCE_FUNCTION(ArgumentAnalysis)
mPNMPI_REGISTRATIONPOINT_FUNCTION(ArgumentAnalysis)

//=============================
// Constructor
//=============================
ArgumentAnalysis::ArgumentAnalysis (const char* instanceName)
    : gti::ModuleBase<ArgumentAnalysis, I_ArgumentAnalysis> (instanceName),
      myArgNames (NULL)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    //nothing to do

    //Initialize data
    myArgNames = new std::string[MUST_ARGUMENT_LAST_ARGUMENT];

    myArgNames[MUST_ARGUMENT_COMM] = "comm";
    myArgNames[MUST_ARGUMENT_BUF] = "buf";
    myArgNames[MUST_ARGUMENT_COUNT] = "count";
    myArgNames[MUST_ARGUMENT_DATATYPE] = "datatype";
    myArgNames[MUST_ARGUMENT_DEST] = "dest";
    myArgNames[MUST_ARGUMENT_TAG] = "tag";
    myArgNames[MUST_ARGUMENT_SOURCE] = "source";
    myArgNames[MUST_ARGUMENT_STATUS] = "status";
    myArgNames[MUST_ARGUMENT_BUFFER] = "buffer";
    myArgNames[MUST_ARGUMENT_SIZE] = "size";
    myArgNames[MUST_ARGUMENT_REQUEST] = "request";
    myArgNames[MUST_ARGUMENT_FLAG] = "flag";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_REQUESTS] = "array_of_requests";
    myArgNames[MUST_ARGUMENT_INDEX] = "index";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_STATUSES] = "array_of_statuses";
    myArgNames[MUST_ARGUMENT_INCOUNT] = "incount";
    myArgNames[MUST_ARGUMENT_OUTCOUNT] = "outcount";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_INDICES] = "array_of_indices";
    myArgNames[MUST_ARGUMENT_SENDBUF] = "sendbuf";
    myArgNames[MUST_ARGUMENT_SENDCOUNT] = "sendcount";
    myArgNames[MUST_ARGUMENT_SENDTYPE] = "sendtype";
    myArgNames[MUST_ARGUMENT_SENDTAG] = "sendtag";
    myArgNames[MUST_ARGUMENT_RECVBUF] = "recvbuf";
    myArgNames[MUST_ARGUMENT_RECVCOUNT] = "recvcount";
    myArgNames[MUST_ARGUMENT_RECVTYPE] = "recvtype";
    myArgNames[MUST_ARGUMENT_RECVTAG] = "recvtag";
    myArgNames[MUST_ARGUMENT_OLDTYPE] = "oldtype";
    myArgNames[MUST_ARGUMENT_NEWTYPE] = "newtype";
    myArgNames[MUST_ARGUMENT_BLOCKLENGTH] = "blocklength";
    myArgNames[MUST_ARGUMENT_STRIDE] = "stride";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_BLOCKLENGTHS] = "array_of_blocklengths";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_DISPLACEMENTS] = "array_of_displacements";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_TYPES] = "array_of_types";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_SIZES] = "array_of_sizes";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_SUBSIZES] = "array_of_subsizes";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_STARTS] = "array_of_starts";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_GSIZES] = "array_of_gsizes";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_DISTRIBS] = "array_of_distribs";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_DARGS] = "array_of_dargs";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_PSIZES] = "array_of_psizes";
    myArgNames[MUST_ARGUMENT_LOCATION] = "location";
    myArgNames[MUST_ARGUMENT_ADDRESS] = "address";
    myArgNames[MUST_ARGUMENT_EXTENT] = "extent";
    myArgNames[MUST_ARGUMENT_DISPLACEMENT] = "displacement";
    myArgNames[MUST_ARGUMENT_INBUF] = "inbuf";
    myArgNames[MUST_ARGUMENT_OUTBUF] = "outbuf";
    myArgNames[MUST_ARGUMENT_OUTSIZE] = "outsize";
    myArgNames[MUST_ARGUMENT_POSITION] = "position";
    myArgNames[MUST_ARGUMENT_INSIZE] = "insize";
    myArgNames[MUST_ARGUMENT_ERRORCODE] = "errorcode";
    myArgNames[MUST_ARGUMENT_ORIGIN_ADDR] = "origin_addr";
    myArgNames[MUST_ARGUMENT_ORIGIN_COUNT] = "origin_count";
    myArgNames[MUST_ARGUMENT_ORIGIN_DATATYPE] = "origin_datatype";
    myArgNames[MUST_ARGUMENT_TARGET_RANK] = "target_rank";
    myArgNames[MUST_ARGUMENT_TARGET_DISP] = "target_disp";
    myArgNames[MUST_ARGUMENT_TARGET_COUNT] = "target_count";
    myArgNames[MUST_ARGUMENT_TARGET_DATATYPE] = "target_datatype";
    myArgNames[MUST_ARGUMENT_OP] = "op";
    myArgNames[MUST_ARGUMENT_WIN] = "win";
    myArgNames[MUST_ARGUMENT_ERRORCLASS] = "errorclass";
    myArgNames[MUST_ARGUMENT_STRING] = "string";
    myArgNames[MUST_ARGUMENT_RECVCOUNTS] = "recvcounts";
    myArgNames[MUST_ARGUMENT_DISPLS] = "displs";
    myArgNames[MUST_ARGUMENT_INFO] = "info";
    myArgNames[MUST_ARGUMENT_BASEPTR] = "baseptr";
    myArgNames[MUST_ARGUMENT_SENDCOUNTS] = "sendcounts";
    myArgNames[MUST_ARGUMENT_SDISPLS] = "sdispls";
    myArgNames[MUST_ARGUMENT_RDISPLS] = "rdispls";
    myArgNames[MUST_ARGUMENT_SENDTYPES] = "sendtypes";
    myArgNames[MUST_ARGUMENT_RECVTYPES] = "recvtypes";
    myArgNames[MUST_ARGUMENT_KEYVAL] = "keyval";
    myArgNames[MUST_ARGUMENT_ATTRIBUTE_VAL] = "attribute_val";
    myArgNames[MUST_ARGUMENT_ROOT] = "root";
    myArgNames[MUST_ARGUMENT_RANK] = "rank";
    myArgNames[MUST_ARGUMENT_MAXDIMS] = "maxdims";
    myArgNames[MUST_ARGUMENT_COORDS] = "coords";
    myArgNames[MUST_ARGUMENT_OLD_COMM] = "old_comm";
    myArgNames[MUST_ARGUMENT_NDIMS] = "ndims";
    myArgNames[MUST_ARGUMENT_DIMS] = "dims";
    myArgNames[MUST_ARGUMENT_PERIODS] = "periods";
    myArgNames[MUST_ARGUMENT_REORDER] = "reorder";
    myArgNames[MUST_ARGUMENT_COMM_CART] = "comm_cart";
    myArgNames[MUST_ARGUMENT_NEWRANK] = "newrank";
    myArgNames[MUST_ARGUMENT_DIRECTION] = "direction";
    myArgNames[MUST_ARGUMENT_DISP] = "disp";
    myArgNames[MUST_ARGUMENT_RANK_SOURCE] = "rank_source";
    myArgNames[MUST_ARGUMENT_RANK_DEST] = "rank_dest";
    myArgNames[MUST_ARGUMENT_REMAIN_DIMS] = "remain_dims";
    myArgNames[MUST_ARGUMENT_NEW_COMM] = "new_comm";
    myArgNames[MUST_ARGUMENT_PORT_NAME] = "port_name";
    myArgNames[MUST_ARGUMENT_NEWCOMM] = "newcomm";
    myArgNames[MUST_ARGUMENT_COMM1] = "comm1";
    myArgNames[MUST_ARGUMENT_COMM2] = "comm2";
    myArgNames[MUST_ARGUMENT_RESULT] = "result";
    myArgNames[MUST_ARGUMENT_FUNCTION] = "function";
    myArgNames[MUST_ARGUMENT_ERRHANDLER] = "errhandler";
    myArgNames[MUST_ARGUMENT_COMM_COPY_ATTR_FN] = "comm_copy_attr_fn";
    myArgNames[MUST_ARGUMENT_COMM_DELETE_ATTR_FN] = "comm_delete_attr_fn";
    myArgNames[MUST_ARGUMENT_COMM_KEYVAL] = "comm_keyval";
    myArgNames[MUST_ARGUMENT_EXTRA_STATE] = "extra_state";
    myArgNames[MUST_ARGUMENT_GROUP] = "group";
    myArgNames[MUST_ARGUMENT_ERHANDLER] = "erhandler";
    myArgNames[MUST_ARGUMENT_COMM_NAME] = "comm_name";
    myArgNames[MUST_ARGUMENT_RESULTLEN] = "resultlen";
    myArgNames[MUST_ARGUMENT_PARENT] = "parent";
    myArgNames[MUST_ARGUMENT_FD] = "fd";
    myArgNames[MUST_ARGUMENT_INTERCOMM] = "intercomm";
    myArgNames[MUST_ARGUMENT_COMMAND] = "command";
    myArgNames[MUST_ARGUMENT_ARGV] = "argv";
    myArgNames[MUST_ARGUMENT_MAXPROCS] = "maxprocs";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_ERRCODES] = "array_of_errcodes";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_COMMANDS] = "array_of_commands";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_ARGV] = "array_of_argv";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_MAXPROCS] = "array_of_maxprocs";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_INFO] = "array_of_info";
    myArgNames[MUST_ARGUMENT_COLOR] = "color";
    myArgNames[MUST_ARGUMENT_KEY] = "key";
    myArgNames[MUST_ARGUMENT_NNODES] = "nnodes";
    myArgNames[MUST_ARGUMENT_FILE] = "file";
    myArgNames[MUST_ARGUMENT_FH] = "fh";
    myArgNames[MUST_ARGUMENT_FILENAME] = "filename";
    myArgNames[MUST_ARGUMENT_AMODE] = "amode";
    myArgNames[MUST_ARGUMENT_INFO_USED] = "info_used";
    myArgNames[MUST_ARGUMENT_ETYPE] = "etype";
    myArgNames[MUST_ARGUMENT_FILETYPE] = "filetype";
    myArgNames[MUST_ARGUMENT_DATAREP] = "datarep";
    myArgNames[MUST_ARGUMENT_OFFSET] = "offset";
    myArgNames[MUST_ARGUMENT_WHENCE] = "whence";
    myArgNames[MUST_ARGUMENT_BASE] = "base";
    myArgNames[MUST_ARGUMENT_NAME] = "name";
    myArgNames[MUST_ARGUMENT_VERSION] = "version";
    myArgNames[MUST_ARGUMENT_SUBVERSION] = "subversion";
    myArgNames[MUST_ARGUMENT_COMM_OLD] = "comm_old";
    myArgNames[MUST_ARGUMENT_EDGES] = "edges";
    myArgNames[MUST_ARGUMENT_COMM_GRAPH] = "comm_graph";
    myArgNames[MUST_ARGUMENT_MAXINDICES] = "maxindices";
    myArgNames[MUST_ARGUMENT_MAXEDGES] = "maxedges";
    myArgNames[MUST_ARGUMENT_NNEIGHBORS] = "nneighbors";
    myArgNames[MUST_ARGUMENT_MAXNEIGHBORS] = "maxneighbors";
    myArgNames[MUST_ARGUMENT_NEIGHBORS] = "neighbors";
    myArgNames[MUST_ARGUMENT_NEDGES] = "nedges";
    myArgNames[MUST_ARGUMENT_QUERY_FN] = "query_fn";
    myArgNames[MUST_ARGUMENT_FREE_FN] = "free_fn";
    myArgNames[MUST_ARGUMENT_CANCEL_FN] = "cancel_fn";
    myArgNames[MUST_ARGUMENT_GROUP1] = "group1";
    myArgNames[MUST_ARGUMENT_GROUP2] = "group2";
    myArgNames[MUST_ARGUMENT_NEWGROUP] = "newgroup";
    myArgNames[MUST_ARGUMENT_N] = "n";
    myArgNames[MUST_ARGUMENT_RANKS] = "ranks";
    myArgNames[MUST_ARGUMENT_RANGES3] = "ranges3";
    myArgNames[MUST_ARGUMENT_RANKS1] = "ranks1";
    myArgNames[MUST_ARGUMENT_RANKS2] = "ranks2";
    myArgNames[MUST_ARGUMENT_NEWINFO] = "newinfo";
    myArgNames[MUST_ARGUMENT_VALUELEN] = "valuelen";
    myArgNames[MUST_ARGUMENT_VALUE] = "value";
    myArgNames[MUST_ARGUMENT_NKEYS] = "nkeys";
    myArgNames[MUST_ARGUMENT_ARGC] = "argc";
    myArgNames[MUST_ARGUMENT_REQUIRED] = "required";
    myArgNames[MUST_ARGUMENT_PROVIDED] = "provided";
    myArgNames[MUST_ARGUMENT_LOCAL_COMM] = "local_comm";
    myArgNames[MUST_ARGUMENT_LOCAL_LEADER] = "local_leader";
    myArgNames[MUST_ARGUMENT_BRIDGE_COMM] = "bridge_comm";
    myArgNames[MUST_ARGUMENT_REMOTE_LEADER] = "remote_leader";
    myArgNames[MUST_ARGUMENT_NEWINTERCOMM] = "newintercomm";
    myArgNames[MUST_ARGUMENT_NEWINTRACOMM] = "newintracomm";
    myArgNames[MUST_ARGUMENT_HIGH] = "high";
    myArgNames[MUST_ARGUMENT_COPY_FN] = "copy_fn";
    myArgNames[MUST_ARGUMENT_DELETE_FN] = "delete_fn";
    myArgNames[MUST_ARGUMENT_SERVICE_NAME] = "service_name";
    myArgNames[MUST_ARGUMENT_COMMUTE] = "commute";
    myArgNames[MUST_ARGUMENT_LEVEL] = "level";
    myArgNames[MUST_ARGUMENT_INOUTBUF] = "inoutbuf";
    myArgNames[MUST_ARGUMENT_READ_CONVERSION_FN] = "read_conversion_fn";
    myArgNames[MUST_ARGUMENT_WRITE_CONVERSION_FN] = "write_conversion_fn";
    myArgNames[MUST_ARGUMENT_DTYPE_FILE_EXTENT_FN] = "dtype_file_extent_fn";
    myArgNames[MUST_ARGUMENT_IBUF] = "ibuf";
    myArgNames[MUST_ARGUMENT_C_STATUS] = "c_status";
    myArgNames[MUST_ARGUMENT_F_STATUS] = "f_status";
    myArgNames[MUST_ARGUMENT_TYPE] = "type";
    myArgNames[MUST_ARGUMENT_GSIZE_ARRAY] = "gsize_array";
    myArgNames[MUST_ARGUMENT_DISTRIB_ARRAY] = "distrib_array";
    myArgNames[MUST_ARGUMENT_DARG_ARRAY] = "darg_array";
    myArgNames[MUST_ARGUMENT_PSIZE_ARRAY] = "psize_array";
    myArgNames[MUST_ARGUMENT_ORDER] = "order";
    myArgNames[MUST_ARGUMENT_P] = "p";
    myArgNames[MUST_ARGUMENT_R] = "r";
    myArgNames[MUST_ARGUMENT_TYPE_COPY_ATTR_FN] = "type_copy_attr_fn";
    myArgNames[MUST_ARGUMENT_TYPE_DELETE_ATTR_FN] = "type_delete_attr_fn";
    myArgNames[MUST_ARGUMENT_TYPE_KEYVAL] = "type_keyval";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_BLOCK_LENGTHS] = "array_of_block_lengths";
    myArgNames[MUST_ARGUMENT_SIZE_ARRAY] = "size_array";
    myArgNames[MUST_ARGUMENT_SUBSIZE_ARRAY] = "subsize_array";
    myArgNames[MUST_ARGUMENT_START_ARRAY] = "start_array";
    myArgNames[MUST_ARGUMENT_LB] = "lb";
    myArgNames[MUST_ARGUMENT_MTYPE] = "mtype";
    myArgNames[MUST_ARGUMENT_MAX_INTEGERS] = "max_integers";
    myArgNames[MUST_ARGUMENT_MAX_ADDRESSES] = "max_addresses";
    myArgNames[MUST_ARGUMENT_MAX_DATATYPES] = "max_datatypes";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_INTEGERS] = "array_of_integers";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_ADDRESSES] = "array_of_addresses";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_DATATYPES] = "array_of_datatypes";
    myArgNames[MUST_ARGUMENT_NUM_INTEGERS] = "num_integers";
    myArgNames[MUST_ARGUMENT_NUM_ADDRESSES] = "num_addresses";
    myArgNames[MUST_ARGUMENT_NUM_DATATYPES] = "num_datatypes";
    myArgNames[MUST_ARGUMENT_COMBINER] = "combiner";
    myArgNames[MUST_ARGUMENT_TYPE_NAME] = "type_name";
    myArgNames[MUST_ARGUMENT_TRUE_LB] = "true_lb";
    myArgNames[MUST_ARGUMENT_TRUE_EXTENT] = "true_extent";
    myArgNames[MUST_ARGUMENT_TYPECLASS] = "typeclass";
    myArgNames[MUST_ARGUMENT_ATTR_VAL] = "attr_val";
    myArgNames[MUST_ARGUMENT_UB] = "ub";
    myArgNames[MUST_ARGUMENT_DISP_UNIT] = "disp_unit";
    myArgNames[MUST_ARGUMENT_WIN_COPY_ATTR_FN] = "win_copy_attr_fn";
    myArgNames[MUST_ARGUMENT_WIN_DELETE_ATTR_FN] = "win_delete_attr_fn";
    myArgNames[MUST_ARGUMENT_WIN_KEYVAL] = "win_keyval";
    myArgNames[MUST_ARGUMENT_ASSERT] = "assert";
    myArgNames[MUST_ARGUMENT_WIN_NAME] = "win_name";
    myArgNames[MUST_ARGUMENT_LOCK_TYPE] = "lock_type";
    myArgNames[MUST_ARGUMENT_INDICES] = "indices";
    myArgNames[MUST_ARGUMENT_RANGES] = "ranges";
    myArgNames[MUST_ARGUMENT_PEER_COMM] = "peer_comm";
    myArgNames[MUST_ARGUMENT_ARRAY_OF_IDISPLACEMENTS] = "array_of_idisplacements";
}

//=============================
// Destructor
//=============================
ArgumentAnalysis::~ArgumentAnalysis ()
{
    if (myArgNames)
    		delete [] myArgNames;
}

//=============================
// getIndex
//=============================
int ArgumentAnalysis::getIndex (MustArgumentId id)
{
	return id >> 24;
}

//=============================
// getArgName
//=============================
std::string ArgumentAnalysis::getArgName (MustArgumentId id)
{
	int index = id & 0x00FFFFFF;

	if (index < 0 || index >= MUST_ARGUMENT_LAST_ARGUMENT)
		return "";

	return myArgNames[index];
}

/*EOF*/
