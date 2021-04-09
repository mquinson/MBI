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
 * @file error_generator.c
 *
 * Generates (semi-random) errors in MPI communication
 * 
 *  @date 26.06.2012
 *  @author Joachim Protze
 */

/*
 * Give the MPI-function to be instrumented:
 * -DERRORGEN_MPI_(B|R|S|I|IB|IR|IS)?SEND 
 *          will instrument this given Send-function
 * -DERRORGEN_MPI_I?RECV 
 *          will instrument this given Recv-function
 * -DERRORGEN_MPI_BARRIER 
 *          will instrument the MPI_Barrier
 * -DERRORGEN_MPI_BCAST 
 *          will instrument the MPI_Bcast
 * -DERRORGEN_MPI_(ALL)REDUCE 
 *          will instrument the MPI_(All)reduce
 *
 *
 * 
 * Behaviour:
 * -DERRORGEN_SKIP
 *          will skip the instrumented function on trigger
 * -DERRORGEN_ADD_BARRIER
 *          if -DERRORGEN_SKIP is used this adds a Barrier on the given communicator in addition to skiping the call
 * -DERRORGEN_TAG_INC
 *          will increment the functions tag argument on trigger
 * -DERRORGEN_TAG_DEC
 *          will decrement the functions tag argument on trigger
 * -DERRORGEN_DEST_INC
 *          will increment the functions remote-rank argument on trigger
 * -DERRORGEN_DEST_DEC
 *          will increment the functions remote-rank argument on trigger
 * -DERRORGEN_DATATYPE_NULL
 *          will set the functions datatype argument to MPI_DATATYPE_NULL on trigger
 * -DERRORGEN_COMM_NULL
 *          will set the functions communicator argument to MPI_COMM_NULL on trigger
 * -DERRORGEN_BUFFER_NULL
 *          will set the functions buffer argument to NULL on trigger
 * -DERRORGEN_BUFFER_BOTTOM
 *          will set the functions buffer argument to MPI_BOTTOM on trigger
 * -DERRORGEN_COUNT_ZERO_BUFFER_NULL
 *          will set the functions buffer argument to NULL and the count to 0 on trigger
 * -DERRORGEN_COUNT_ONE_BUFFER_BOTTOM
 *          will set the functions buffer argument to MPI_BOTTOM and the count to 1 on trigger
 * -DERRORGEN_COUNT_ONE
 *          will set the count being used to 1
 * -DERRORGEN_COUNT_INC
 *          will increment the count being used by one
 * -DERRORGEN_COUNT_DOUBLE
 *          will double the count being used
 * -DERRORGEN_COUNT_ZERO
 *          will set the count being used to 0
 * -DERRORGEN_OP_MAXLOC
 *          will set the op being used to MPI_MAXLOC
 * -DERRORGEN_OP_MAX
 *          will set the op being used to MPI_MAX
 * 
 *
 *
 * Trigger (what event generates the error?)
 * -DERRORGEN_TRIGGER_TIME
 *          after a time delay given by -DERRORGEN_DELAY=<time in seconds [double]>
 * -DERRORGEN_TRIGGER_CALLCOUNT
 *          after a callcount delay given by -DERRORGEN_DELAY=<count>
 * -DERRORGEN_TRIGGER_STARTING_AT_CALLCOUNT
 *          like the previous one, but for all applicable calls starting at the given count
 * -DERRORGEN_TRIGGER_TAG
 *          errorinduction for special TAG given by -DERRORGEN_TAG=<tag>
 * 
 * 
 * 
 * -DERRORGEN_TRIGGER_RANK=<NUMBER>
 *          limit errorinduction to one rank
 * 
 * 
 * -DERRORGEN_DELAY=<NUMBER> 
 *          specifies the delay for the chosen trigger
 * -DERRORGEN_TAG=<NUMBER>
 *          specifies the tag for the chosen trigger
 * 
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>



#ifdef ERRORGEN_COUNT_ONE
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) count = 1
#elif defined ERRORGEN_COUNT_ZERO
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) count = 0
#elif defined ERRORGEN_COUNT_ZERO_BUFFER_NULL
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) count = 0; buf=NULL
#elif defined ERRORGEN_COUNT_ONE_BUFFER_BOTTOM
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) count = 1; buf=MPI_BOTTOM
#elif defined ERRORGEN_COUNT_INC
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) count++
#elif defined ERRORGEN_COUNT_DOUBLE
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) count*=2
#elif defined ERRORGEN_TAG_INC
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) tag++
#elif defined ERRORGEN_TAG_DEC
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) tag--
#elif defined ERRORGEN_COMM_NULL
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) comm=MPI_COMM_NULL
#elif defined ERRORGEN_DATATYPE_NULL
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) datatype=MPI_DATATYPE_NULL
#elif defined ERRORGEN_BUFFER_NULL
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) buf=NULL
#elif defined ERRORGEN_BUFFER_BOTTOM
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) buf=MPI_BOTTOM
#elif defined ERRORGEN_OP_MAXLOC
    #define ERRORGEN_TYPE(buf, datatype, dest, op, comm, count) op=MPI_MAXLOC
#elif defined ERRORGEN_OP_MAX
    #define ERRORGEN_TYPE(buf, datatype, dest, op, comm, count) op=MPI_MAX
#elif defined ERRORGEN_DEST_INC
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) \
    int size;\
    PMPI_Comm_size(comm, &size);\
    dest=(dest+1)%size
#elif defined ERRORGEN_DEST_DEC
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) \
    int size;\
    PMPI_Comm_size(comm, &size);\
    dest=(dest-1+size)%size
#else
    #define ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count) buf=buf
#endif

#ifndef ERRORGEN_DELAY
    #define ERRORGEN_DELAY 60
#endif

#ifdef ERRORGEN_TRIGGER_RANK
    #define ERRORGEN_TRIGGER_ON_RANK && is_this_rank(ERRORGEN_TRIGGER_RANK)
#else
    #define ERRORGEN_TRIGGER_ON_RANK
#endif

#ifdef ERRORGEN_TRIGGER_TIME
    #define ERRORGEN_TRIGGER(buf, datatype, dest, tag, comm) error_on_timer() ERRORGEN_TRIGGER_ON_RANK
#elif defined ERRORGEN_TRIGGER_CALLCOUNT
    #define ERRORGEN_TRIGGER(buf, datatype, dest, tag, comm) callcount==ERRORGEN_DELAY ERRORGEN_TRIGGER_ON_RANK
#elif defined ERRORGEN_TRIGGER_STARTING_AT_CALLCOUNT
    #define ERRORGEN_TRIGGER(buf, datatype, dest, tag, comm) callcount>=ERRORGEN_DELAY ERRORGEN_TRIGGER_ON_RANK
#elif defined ERRORGEN_TRIGGER_TAG
    #define ERRORGEN_TRIGGER(buf, datatype, dest, tag, comm) tag==ERRORGEN_TAG ERRORGEN_TRIGGER_ON_RANK
#else
    #define ERRORGEN_TRIGGER(buf, datatype, dest, tag, comm) 0
#endif

#define ERROR_STRING(s) ERROR_STRINGIFY(s)
#define ERROR_STRINGIFY(s) #s

#ifdef ERRORGEN_MPI_SEND
    #define ERRORGEN_SEND MPI_Send
    #define ERRORGEN_PSEND PMPI_Send
#elif defined ERRORGEN_MPI_BSEND
    #define ERRORGEN_SEND MPI_Bsend
    #define ERRORGEN_PSEND PMPI_Bsend
#elif defined ERRORGEN_MPI_RSEND
    #define ERRORGEN_SEND MPI_Rsend
    #define ERRORGEN_PSEND PMPI_Rsend
#elif defined ERRORGEN_MPI_SSEND
    #define ERRORGEN_SEND MPI_Ssend
    #define ERRORGEN_PSEND PMPI_Ssend
#elif defined ERRORGEN_MPI_ISEND
    #define ERRORGEN_SEND MPI_Isend
    #define ERRORGEN_PSEND PMPI_Isend
    #define ERRORGEN_ISEND 1
#elif defined ERRORGEN_MPI_IBSEND
    #define ERRORGEN_SEND MPI_Ibsend
    #define ERRORGEN_PSEND PMPI_Ibsend
    #define ERRORGEN_ISEND 1
#elif defined ERRORGEN_MPI_IRSEND
    #define ERRORGEN_SEND MPI_Irsend
    #define ERRORGEN_PSEND PMPI_Irsend
    #define ERRORGEN_ISEND 1
#elif defined ERRORGEN_MPI_ISSEND
    #define ERRORGEN_SEND MPI_Issend
    #define ERRORGEN_PSEND PMPI_Issend
    #define ERRORGEN_ISEND 1
#endif /* some MPI_Send instrumentation */

#ifdef ERRORGEN_MPI_RECV
    #define ERRORGEN_RECV MPI_Recv
    #define ERRORGEN_PRECV PMPI_Recv
#elif defined ERRORGEN_MPI_IRECV
    #define ERRORGEN_RECV MPI_Irecv
    #define ERRORGEN_PRECV PMPI_Irecv
    #define ERRORGEN_IRECV 1
#endif /* some MPI_Recv instrumentation */

int error_on_timer()
{
    static double starttime=0;
    static int initialized=0;
    if (!initialized)
    {
        starttime = PMPI_Wtime();
        initialized = 1;
    }
    if(PMPI_Wtime()-starttime > ERRORGEN_DELAY)
    {
        starttime = PMPI_Wtime();
        return 1;
    }
    else
        return 0;
}

int is_this_rank(int rank){
    static int myrank, initialized=0;
    if (!initialized)
    {
        PMPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    }
    return rank == myrank;
}

#ifdef ERRORGEN_SEND
  #if defined ERRORGEN_OP_MAX || defined ERRORGEN_OP_MAXLOC
    #warning "no effect for using ERRORGEN_SEND with ERRORGEN_OP_MAX(LOC)"
  #endif
int ERRORGEN_SEND(
  void *buf,
  int count,
  MPI_Datatype datatype,
  int dest,
  int tag,
  MPI_Comm comm
  #ifdef ERRORGEN_ISEND
  ,MPI_Request *request
  #endif /* ERRORGEN_ISEND */
)
{
    int ret;
    static int callcount = 0;
    callcount++;
//     printf("%s seen\n",ERROR_STRING(ERRORGEN_SEND));
    if(ERRORGEN_TRIGGER(buf, datatype, dest, tag, comm))
    {
        printf("ERRORGEN: Trigger on callcount %i\n", callcount);
        printf("ERRORGEN: Original %s(buf=%lli, datatype=%lli, count=%i, dest=%i, tag=%i, comm=%lli)\n", ERROR_STRING(ERRORGEN_SEND), (long long int)buf, (long long int)datatype, (int)count, (int)dest, (int)tag, (long long int)comm);
        ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count);
  #ifdef ERRORGEN_SKIP
        printf("ERRORGEN: Skip Call\n");
#ifdef ERRORGEN_ADD_BARRIER
        ret = MPI_Barrier (comm);
#endif /*DERRORGEN_ADD_BARRIER*/
  #else
        printf("ERRORGEN: Modified %s(buf=%lli, datatype=%lli, count=%i, dest=%i, tag=%i, comm=%lli)\n", ERROR_STRING(ERRORGEN_SEND), (long long int)buf, (long long int)datatype, (int)count, (int)dest, (int)tag, (long long int)comm);
  #endif
    }
  #ifdef ERRORGEN_SKIP
    else
    {
  #endif /* ERRORGEN_SKIP */
    ret = ERRORGEN_PSEND(buf, count, datatype, dest, tag, comm
  #ifdef ERRORGEN_ISEND
        , request
  #endif /* ERRORGEN_ISEND */
    );
  #ifdef ERRORGEN_SKIP
    }
  #endif /* ERRORGEN_SKIP */
  return ret;
}
#endif /* ERRORGEN_SEND */

#ifdef ERRORGEN_RECV
  #if defined ERRORGEN_OP_MAX || defined ERRORGEN_OP_MAXLOC
    #warning "no effect for using ERRORGEN_RECV with ERRORGEN_OP_MAX(LOC)"
  #endif
int ERRORGEN_RECV(
  void *buf,
  int count,
  MPI_Datatype datatype,
  int dest,
  int tag,
  MPI_Comm comm,
  #ifdef ERRORGEN_IRECV
  MPI_Request *request
  #else
  MPI_Status *status
  #endif /* ERRORGEN_IRECV */
)
{
    int ret;
    static int callcount = 0;
    callcount++;
    if(ERRORGEN_TRIGGER(buf, datatype, dest, tag, comm))
    {
        printf("ERRORGEN: Trigger on callcount %i\n", callcount);
        printf("ERRORGEN: Original %s(buf=%lli, datatype=%lli, count=%i, dest=%i, tag=%i, comm=%lli)\n", ERROR_STRING(ERRORGEN_RECV),(long long int)buf, (long long int)datatype, (int)count, (int)dest, (int)tag, (long long int)comm);
        ERRORGEN_TYPE(buf, datatype, dest, tag, comm, count);
  #ifdef ERRORGEN_SKIP
        printf("ERRORGEN: Skip Call\n");
#ifdef ERRORGEN_ADD_BARRIER
        ret=MPI_Barrier (comm);
#endif /*DERRORGEN_ADD_BARRIER*/
  #else
        printf("ERRORGEN: Modified %s(buf=%lli, datatype=%lli, count=%i, dest=%i, tag=%i, comm=%lli)\n", ERROR_STRING(ERRORGEN_RECV),(long long int)buf, (long long int)datatype, (int)count, (int)dest, (int)tag, (long long int)comm);
  #endif
    }
  #ifdef ERRORGEN_SKIP
    else
    {
  #endif /* ERRORGEN_SKIP */
    ret=ERRORGEN_PRECV(buf, count, datatype, dest, tag, comm,
  #ifdef ERRORGEN_IRECV
        request
  #else
        status
  #endif /* ERRORGEN_IRECV */
    );
  #ifdef ERRORGEN_SKIP
    }
  #endif /* ERRORGEN_SKIP */
  return ret;
}
#endif /* ERRORGEN_RECV */

#ifdef ERRORGEN_MPI_BCAST
  #if defined ERRORGEN_TAG_INC || defined ERRORGEN_TAG_DEC || defined ERRORGEN_TRIGGER_TAG
    #warning "no effect for using ERRORGEN_MPI_BCAST with ERRORGEN_TAG_(INC|DEC) or ERRORGEN_TRIGGER_TAG"
  #endif
int MPI_Bcast(
  void *buf,
  int count,
  MPI_Datatype datatype,
  int root,
  MPI_Comm comm
)
{
    int ret;
    static int callcount = 0;
    callcount++;
    int tag=0;
    if(ERRORGEN_TRIGGER(buf, datatype, root, tag, comm))
    {
        printf("ERRORGEN: Trigger on callcount %i\n", callcount);
        printf("ERRORGEN: Original MPI_Bcast(buf=%lli, datatype=%lli, dest=%i, tag=%i, comm=%lli)\n", (long long int)buf, (long long int)datatype, (int)root, (int)tag, (long long int)comm);
        ERRORGEN_TYPE(buf, datatype, root, tag, comm, count);
  #ifdef ERRORGEN_SKIP
        printf("ERRORGEN: Skip Call\n");
#ifdef ERRORGEN_ADD_BARRIER
        ret=MPI_Barrier (comm);
#endif /*DERRORGEN_ADD_BARRIER*/
  #else
        printf("ERRORGEN: Modified MPI_Bcast(buf=%lli, datatype=%lli, dest=%i, tag=%i, comm=%lli)\n", (long long int)buf, (long long int)datatype, (int)root, (int)tag, (long long int)comm);
  #endif
    }
  #ifdef ERRORGEN_SKIP
    else
    {
  #endif /* ERRORGEN_SKIP */
    ret=PMPI_Bcast(buf, count, datatype, root, comm);
  #ifdef ERRORGEN_SKIP
    }
  #endif /* ERRORGEN_SKIP */
  return ret;
}
#endif

#ifdef ERRORGEN_MPI_REDUCE
  #if defined ERRORGEN_TAG_INC || defined ERRORGEN_TAG_DEC || defined ERRORGEN_TRIGGER_TAG
    #warning "no effect for using ERRORGEN_MPI_REDUCE with ERRORGEN_TAG_(INC|DEC) or ERRORGEN_TRIGGER_TAG"
  #endif
int MPI_Reduce(
  void *buf,
  void *rbuf,
  int count,
  MPI_Datatype datatype,
  MPI_Op op,
  int root,
  MPI_Comm comm
)
{
    int ret;
    static int callcount = 0;
    callcount++;
    int tag=0;
    if(ERRORGEN_TRIGGER(buf, datatype, root, op, comm))
    {
        printf("ERRORGEN: Trigger on callcount %i\n", callcount);
        printf("ERRORGEN: Original MPI_Reduce(buf=%lli, rbuf=%lli, datatype=%lli, op=%lli, dest=%i, tag=%i, comm=%lli)\n", (long long int)buf, (long long int)rbuf, (long long int)datatype, (long long int)op, (int)root, (long long int)comm);
        ERRORGEN_TYPE(buf, datatype, root, tag, comm, count);
  #ifdef ERRORGEN_SKIP
        printf("ERRORGEN: Skip Call\n");
#ifdef ERRORGEN_ADD_BARRIER
        ret=MPI_Barrier (comm);
#endif /*DERRORGEN_ADD_BARRIER*/
  #else
        printf("ERRORGEN: Modified MPI_Reduce(buf=%lli, rbuf=%lli, datatype=%lli, op=%lli, dest=%i, tag=%i, comm=%lli)\n", (long long int)buf, (long long int)rbuf, (long long int)datatype, (long long int)op, (int)root, (long long int)comm);
  #endif
    }
  #ifdef ERRORGEN_SKIP
    else
    {
  #endif /* ERRORGEN_SKIP */
    ret=PMPI_Reduce(buf, rbuf, count, datatype, op, root, comm);
  #ifdef ERRORGEN_SKIP
    }
  #endif /* ERRORGEN_SKIP */
  return ret;
}
#endif

#ifdef ERRORGEN_MPI_ALLREDUCE
  #if defined ERRORGEN_TAG_INC || defined ERRORGEN_TAG_DEC || defined ERRORGEN_TRIGGER_TAG
    #warning "no effect for using ERRORGEN_MPI_REDUCE with ERRORGEN_TAG_(INC|DEC) or ERRORGEN_TRIGGER_TAG"
  #endif
int MPI_Allreduce(
  void *buf,
  void *rbuf,
  int count,
  MPI_Datatype datatype,
  MPI_Op op,
  MPI_Comm comm
)
{
    int ret;
    static int callcount = 0;
    callcount++;
    int tag=0;
	int root=0;
    if(ERRORGEN_TRIGGER(buf, datatype, root, op, comm))
    {
        printf("ERRORGEN: Trigger on callcount %i\n", callcount);
        printf("ERRORGEN: Original MPI_Allreduce(buf=%lli, rbuf=%lli, datatype=%lli, op=%lli, comm=%lli)\n", (long long int)buf, (long long int)rbuf, (long long int)datatype, (long long int)op, (long long int)comm);
        ERRORGEN_TYPE(buf, datatype, root, op, comm, count);
  #ifdef ERRORGEN_SKIP
        printf("ERRORGEN: Skip Call\n");
#ifdef ERRORGEN_ADD_BARRIER
        ret=MPI_Barrier (comm);
#endif /*DERRORGEN_ADD_BARRIER*/
  #else
        printf("ERRORGEN: Modified MPI_Allreduce(buf=%lli, rbuf=%lli, datatype=%lli, op=%lli, comm=%lli)\n", (long long int)buf, (long long int)rbuf, (long long int)datatype, (long long int)op, (long long int)comm);
  #endif
    }
  #ifdef ERRORGEN_SKIP
    else
    {
  #endif /* ERRORGEN_SKIP */
    ret=PMPI_Allreduce(buf, rbuf, count, datatype, op, comm);
  #ifdef ERRORGEN_SKIP
    }
  #endif /* ERRORGEN_SKIP */
  return ret;
}
#endif

#ifdef ERRORGEN_MPI_BARRIER
  #if defined ERRORGEN_TAG_INC || defined ERRORGEN_TAG_DEC || defined ERRORGEN_TRIGGER_TAG
    #warning "no effect for using ERRORGEN_MPI_BARRIER with ERRORGEN_TAG_(INC|DEC) or ERRORGEN_TRIGGER_TAG"
  #endif
int MPI_Barrier(
  MPI_Comm comm
)
{
    int ret;
    static int callcount = 0;
    callcount++;
    void* buf;
    MPI_Datatype datatype=MPI_DATATYPE_NULL;
    int root=0;
    int tag=0;
    int dummyCount;
    if(ERRORGEN_TRIGGER(buf, datatype, root, tag, comm))
    {
        printf("ERRORGEN: Trigger on callcount %i\n", callcount);
        printf("ERRORGEN: Original MPI_Barrier(comm=%lli)\n", (long long int)comm);
        ERRORGEN_TYPE(buf, datatype, root, tag, comm, dummyCount);
  #ifdef ERRORGEN_SKIP
        printf("ERRORGEN: Skip Call\n");
#ifdef ERRORGEN_ADD_BARRIER
        ret=MPI_Barrier (comm);
#endif /*DERRORGEN_ADD_BARRIER*/
  #else
#error "use ERRORGEN_MPI_BARRIER with ERRORGEN_SKIP, other modifiers will not work"
        printf("ERRORGEN: Modified MPI_Barrier(comm=%lli)\n", (long long int)comm);
  #endif
    }
  #ifdef ERRORGEN_SKIP
    else
    {
  #endif /* ERRORGEN_SKIP */
    ret=PMPI_Barrier(comm);
  #ifdef ERRORGEN_SKIP
    }
  #endif /* ERRORGEN_SKIP */
  return ret;
}
#endif

