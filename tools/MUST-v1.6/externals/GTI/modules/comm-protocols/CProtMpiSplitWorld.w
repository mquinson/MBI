/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file CProtMpiSplitModule.cpp generated from cprot_mpi_split_module.w
 *       Splits MPI processes into multiple sets of processes.
 *       Intention is to use one set(id:0) for the actual application
 *       and the remaining sets for tool processes (one set for
 *       each level of the tool).
 *       Further, a separate stack is used for each of the sets,
 *       to enable tools with distinct layouts. Also, the actual
 *       application calls are separated such that MPI_COMM_WORLD
 *       is replaced by a comm representing the application
 *       processes set.
 *       For all the tool process sets only an MPI_Init and an
 *       MPI_Finalize is called, afterwards an exit(0) is issued,
 *       so all tool startup should be done in MPI_Init and it should
 *       only return once a shutdown is desired.
 *
 * @author Tobias Hilbrich
 * @date 27.07.2009
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <sstream>

#include <dlfcn.h>

#include <pnmpimod.h>
#include "GtiMacros.h"
#include "CProtMpiSplitModule.h"

/**
 * Global definitions and helpers.
 */
/*Name of split module @todo move to some common place, currently present twice*/
#define MUST_SPLIT_MODULE_NAME "split_processes"

struct process_set_t
{
	int size;
	int start_rank;
	int in_set:2;    ///< If current rank is part of this set.
	int mpi_place:2; ///< If this set consists of MPI places.
	int app_place:2; ///< If this set consists of application places.
	MPI_Comm set_comm;
	int set_index;
	PNMPI_modHandle_t stack;
};

std::vector<process_set_t> g_sets;
std::map <std::pair<int, int> ,int> g_mappings; /**< Maps a (OwnSetIndex,CommID) pair to a SetIndex, usage is to match equal comm_ids to the right set indices.*/

/**
 * The below global variable is used for the MUST-DDT integration.
 * It has the following meaning:
 * # -1 => this is a tool process
 * # 0-N => this is application rank i
 */
int MUST_application_rank = -1;

/**
 * This function serves for debugger integrations:
 * MUST issues it when its own initialization is finished to tell a potentially attached debugger
 * that the MUST initialization as well as MPI_Init was performed.
 */
extern "C" __attribute__ ((noinline)) void MUST_InitComplete (void)
{
    asm("");
#ifdef MUST_DEBUG
    std::cout << "MUST: " << getpid () << " has completed initialization." << std::endl;
#endif
}

/**
 * Query functions.
 */
extern "C" int getMySetSize (int *size)
{
	*size = -1;

	for (int i = 0;i < g_sets.size();i++)
	{
		if (g_sets[i].in_set==1)
			*size = g_sets[i].size;
	}
	return PNMPI_SUCCESS;
}

MPI_Comm realCommWorld;
extern "C" int getRealCommWorld (void *comm)
{
    *((MPI_Comm*)comm) = realCommWorld;
    return PNMPI_SUCCESS;
}

extern "C" int getMySetComm (void *comm)
{
    *((MPI_Comm*)comm) = MPI_COMM_NULL;

    for (int i = 0;i < g_sets.size();i++)
    {
        if (g_sets[i].in_set==1)
            *((MPI_Comm*)comm) = g_sets[i].set_comm;
    }
    return PNMPI_SUCCESS;
}

extern "C" int getSetInfo (int commId, int* set_size, int* start_rank)
{
	int ownSetId;
	int setIdToUse;

	//get own set id
	for (int i = 0;i < g_sets.size();i++)
	{
		if (g_sets[i].in_set==1)
			ownSetId = i;
	}

	//find set id to use
	std::map<std::pair<int,int>, int >::iterator iter =
			g_mappings.find(std::make_pair(ownSetId, commId));

	if (iter != g_mappings.end())
		setIdToUse = iter->second; //New version use the mapping (if given)
	else
		setIdToUse = commId; //Old version gicen commId is the setId to use

	assert (setIdToUse < g_sets.size() && setIdToUse >= 0);

	*set_size = g_sets[setIdToUse].size;
	*start_rank = g_sets[setIdToUse].start_rank;

	return PNMPI_SUCCESS;
}
/**
 * PNMPI_ReistrationPoint
 */
extern "C" void PNMPI_RegistrationPoint()
{
  int err;

  /* register this module*/
  err=PNMPI_Service_RegisterModule(MUST_SPLIT_MODULE_NAME);
  assert (err == PNMPI_SUCCESS);
}

#ifdef MUST_TIME
struct timeval gStart, gEnd;
#endif

static int gModuleInitialized = 0;
/* extern "C" int MPI_Init(int * argc, char *** argv)
 { */
{{fn fn_name MPI_Init MPI_Init_thread}} {

  if( gModuleInitialized )
   return MPI_SUCCESS;
  
  gModuleInitialized = 1;
  int err;
  const char *inp;
  int split_mod;
  int num_sets, nodesize=0;
  int world_size, rank;
  int curr_set_start = 0;
  PNMPI_Service_descriptor_t service;
  realCommWorld = MPI_COMM_WORLD;
  char buf[512];
  PNMPI_modHandle_t stack=0;

  err = PNMPI_Service_GetModuleSelf(&split_mod);
  assert (err==PNMPI_SUCCESS);

  sprintf (buf,"stack_0");
  err=PNMPI_Service_GetArgument(split_mod,buf,&inp);
  assert (err==PNMPI_SUCCESS);
  err=PNMPI_Service_GetStackByName("level_0",&stack);
  assert (err==PNMPI_SUCCESS);



  /* Get self module */
  extern int X{{fn_name}}_NewStack ({{ list "int stack" {{formals}} }});
  err =  X{{fn_name}}_NewStack ({{ list stack {{args}} }});
//  err = {{callfn}}  

#ifdef MUST_TIME
  gettimeofday(&gStart, NULL);
#endif

  assert (err == MPI_SUCCESS);
  err = PMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  assert (err == MPI_SUCCESS);
  err = PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  assert (err == MPI_SUCCESS);
  err = PMPI_Comm_dup(MPI_COMM_WORLD, &realCommWorld);
  assert (err == MPI_SUCCESS);

  //enable a sleep to allow attaching with a debugger
  if (getenv("MUST_WAIT_AT_STARTUP") != NULL)
  {
//        std::cout << "Rank " << rank << " has pid " << getpid() << std::endl;
// Stop random interleaving when printing
        printf("Rank %i has pid %i\n",rank,getpid());
	  sleep (atoi(getenv("MUST_WAIT_AT_STARTUP")));
  }
  
  /*
   * EXPERIMENTAL, might be of use in the future. 
   * Idea is to automatically connect a gdbserver to some ranks to allow remote debugging
   * in unfriendly environments.
   */  
  if (getenv("MUST_START_GDBSERVER_BEGIN") != NULL && getenv("MUST_START_GDBSERVER_END") != NULL)
  {
  	  int begin = atoi(getenv("MUST_START_GDBSERVER_BEGIN"));
  	  int end = atoi(getenv("MUST_START_GDBSERVER_END"));
  
  	  if (rank >= begin && rank <= end)
  	  {  
	      char host[512];
	      gethostname(host, 512);
	      std::stringstream stream;
	      stream
	      	<< "gdbserver "
	      	<< host
	      	<< ":"
	      	<< rank + 10000
	      	<< " --attach "
	      	<< getpid ()
	      	<< " &"
	      	<< std::endl;
	      if (system (stream.str().c_str())==0)
	         std::cout << "GDBSERVER for rank " << rank << " listens on " << host << ":" << rank + 10000 << std::endl;
	      else
	         std::cout << "Starting GDBSERVER for rank " << rank << " failed!" << std::endl << "Command was: " << stream.str() << std::endl;
	  }
  }

  /* Query for nodesize */
  err=PNMPI_Service_GetArgument(split_mod,"nodesize",&inp);
  if (err==PNMPI_SUCCESS)
    nodesize=atoi(inp);

  /* Query for number of sets and their stacks */
  err=PNMPI_Service_GetArgument(split_mod,"num_sets",&inp);
  assert (err==PNMPI_SUCCESS);
  num_sets=atoi(inp);
  g_sets.resize(num_sets);

/*   int in_sets = 0; */
  int found_app=0;
  for (int i = 0; i < num_sets; i++)
  {
	  int  size;

	  sprintf (buf,"size_%d",i);
	  err=PNMPI_Service_GetArgument(split_mod,buf,&inp);
	  assert (err==PNMPI_SUCCESS);
	  size=atoi(inp);

	if(i>0){
	  sprintf (buf,"stack_%d",i);
	  err=PNMPI_Service_GetArgument(split_mod,buf,&inp);
	  assert (err==PNMPI_SUCCESS);
	  err=PNMPI_Service_GetStackByName(inp,&stack);
	  assert (err==PNMPI_SUCCESS);
	}

	/* Get the place for the set. */
	sprintf (buf, "place_%d",i);
        inp = PNMPI_Service_GetArgumentSelf(buf);
	assert(inp != NULL);
        g_sets[i].app_place=0;
	if (strncmp("mpi_place", inp, 10)==0)
	{
	  g_sets[i].mpi_place=1;
	  if (!found_app)
	  {
	    g_sets[i].app_place=1;
	    found_app=1;
	  }
	}
	else
	{
	  g_sets[i].mpi_place=0;
	  g_sets[i].set_comm = MPI_COMM_SELF;
	  continue;
	}

	  if( size > 0 )
            g_sets[i].start_rank = curr_set_start;
          else
            g_sets[i].start_rank = g_sets[0].start_rank;

      int last_rank = curr_set_start + size - 1;
      g_sets[i].in_set = 0;
	  g_sets[i].set_index = i;
	  g_sets[i].stack = stack;

      if (nodesize>0 && i<2 && num_sets>1)
      {/* for given nodesize, distribute the lower levels: 1 tool rank + nodesize-1 app ranks per node */
        if(i==0)
        {
            g_sets[0].start_rank=1;
            g_sets[0].size = size;
            /* one tool node per computenode */
            /* \lfloor size / (nodesize-1) \rfloor  */
            g_sets[1].size = (size - 1) / (nodesize - 1) + 1;
            last_rank = g_sets[0].size + g_sets[1].size;
            if (rank < last_rank && (rank % nodesize) != 0 && size > 0)
                g_sets[i].in_set = 1;
        }
        else /* i==1 */
        {
            g_sets[1].start_rank=0;
            assert(g_sets[i].size == size);
            if (rank < last_rank && (rank % nodesize) == 0)
                g_sets[i].in_set = 1;
        }
      }
      else /* old split behaviour for nodesize = 0 or higher levels */
      {
        g_sets[i].size = size;
        if (rank >= g_sets[i].start_rank && rank <= last_rank && size > 0)
            g_sets[i].in_set = 1;
      }

	  err = PMPI_Comm_split (MPI_COMM_WORLD, g_sets[i].in_set, 0, &g_sets[i].set_comm);
	  assert (err == MPI_SUCCESS);

	  //MUST-DDT Integration Begin
	  if (g_sets[i].in_set)
	  {
	    if (i == 0)
	    {
	      int myNewAppRank;
	      PMPI_Comm_rank (g_sets[i].set_comm, &myNewAppRank);
	  	  MUST_application_rank = myNewAppRank;
	  	}
	  	else
	  	{
	  	  MUST_application_rank = -1;
	  	}
	  }
      //MUST-DDT Integration END

	  curr_set_start += size;
	  assert (world_size >= curr_set_start); //Sets must require no more processes than available!
  }
  /* Query for mappings */
  err=PNMPI_Service_GetArgument(split_mod,"num_mappings",&inp);
  if (err==PNMPI_SUCCESS)
  {
	  int numMappings = atoi(inp);

	  for (int i = 0; i < numMappings; i++)
	  {
		  char buf[512];
		  int  size;

		  sprintf (buf,"mapping%d",i);
		  err=PNMPI_Service_GetArgument(split_mod,buf,&inp);
		  assert (err==PNMPI_SUCCESS);

		  std::string mapping (inp);

		  int ownSetId, commId, setIdToUse;
		  std::string ownSetIdStr, commIdStr, setIdToUseStr;

		  //Own set id
		  size_t pos = mapping.find_first_of (':');
		  assert (pos != std::string::npos);
		  ownSetIdStr = mapping.substr (0, pos);

		  //comm id
		  size_t pos2 = mapping.find_first_of (':',pos+1);
		  assert (pos2 != std::string::npos);
		  commIdStr = mapping.substr (pos+1, pos2-pos-1);

		  //Set id to use
		  setIdToUseStr = mapping.substr (pos2+1, mapping.length()-pos2-1);

		  ownSetId = atoi (ownSetIdStr.c_str());
		  commId = atoi (commIdStr.c_str());
		  setIdToUse = atoi (setIdToUseStr.c_str());

		  g_mappings.insert (std::make_pair(std::make_pair(ownSetId, commId), setIdToUse));
	  }
  }

  /* Provide services to query for set information */
  sprintf(service.name,"SplitMod_getMySetSize");
  service.fct=(PNMPI_Service_Fct_t) getMySetSize;
  sprintf(service.sig,"p");
  err=PNMPI_Service_RegisterService(&service);
  assert (err == PNMPI_SUCCESS);

  sprintf(service.name,"SplitMod_getMySetComm");
  service.fct=(PNMPI_Service_Fct_t) getMySetComm;
  sprintf(service.sig,"p");
  err=PNMPI_Service_RegisterService(&service);
  assert (err == PNMPI_SUCCESS);

  sprintf(service.name,"SplitMod_getRealCommWorld");
  service.fct=(PNMPI_Service_Fct_t) getRealCommWorld;
  sprintf(service.sig,"p");
  err=PNMPI_Service_RegisterService(&service);
  assert (err == PNMPI_SUCCESS);

  sprintf(service.name,"SplitMod_getSetInfo");
  service.fct=(PNMPI_Service_Fct_t) getSetInfo;
  sprintf(service.sig,"ipp");
  err=PNMPI_Service_RegisterService(&service);
  assert (err == PNMPI_SUCCESS);

  /* Call MPI_Init for the correct stack */
  MUST_InitComplete (); //We are all set up and ready when now; MPI_Init was invoked and we decided who is application and who is tool
  for (int i = 0; i < num_sets; i++)
  {
	  if (/*!g_sets[i].app_place &&*/ g_sets[i].mpi_place && g_sets[i].in_set == 1)
	  {
		  //APPLICATION
		  if (/*i == 0 && 
		     getenv("GTI_MPI_SPLIT_NO_APPLICATION") == NULL*/ g_sets[i].app_place)
		  {
//		      err =  P{{fn_name}}_NewStack ({{ list g_sets[i].stack {{args}} }});
		      extern int X{{fn_name}} ( {{formals}} );
		      err =  X{{fn_name}} ( {{args}} );
			  assert (err == MPI_SUCCESS);
                      break;
		  }
		  //Tool Processes
		  else
		  {
			  /*Init the MPI tool place, when it returns finalize and exit*/
			  extern int X{{fn_name}}_NewStack ({{ list "int stack" {{formals}} }} );
			  err =  X{{fn_name}}_NewStack ({{ list g_sets[i].stack {{args}} }});
			  assert (err == MPI_SUCCESS);

#ifdef MUST_TIME
			  gettimeofday(&gEnd, NULL);

                          if (rank == world_size -1)
			    std::cout << "Time Post Init - Pre Finalize of Tool " << rank << " (usec): " << ((gEnd.tv_sec * 1000000 + gEnd.tv_usec) - (gStart.tv_sec * 1000000 + gStart.tv_usec)) << std::endl;
#endif

              /*
               * This does not calls the real MPI_Finalize,
               * it just passes the finalize to all modules of
               * the stack, we must still call the actual finalize
               * afterwards.
               */
			  err = XMPI_Finalize_NewStack(g_sets[i].stack);
			  assert (err == MPI_SUCCESS);
			  exit(0);
		  }
	  }
  }

  return err;
} {{endfn}}
