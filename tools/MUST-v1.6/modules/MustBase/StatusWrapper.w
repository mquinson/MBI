/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/// -*- c++ -*-
/// PnMPI Wrapper.  These replaces all MPI_STATUS[ES]_IGNORE 
/// by allocated status[es].
#include <mpi.h>
#include "mustConfig.h"

/**
 * Single status
 */
{{fn fn_name MPI_Iprobe
             MPI_Probe
             MPI_Recv
             MPI_Sendrecv
             MPI_Sendrecv_replace
             MPI_Test
             MPI_Testany
             MPI_Wait
             MPI_Waitany
             }}
{
  //get current status
  MPI_Status *cur = {{sub 
                        {{filter '^(const +)?MPI_Status' {{formals}} }}
                        'MPI_Status \*' 
                        ''
                    }};
  MPI_Status temp;
  
  //if status = MPI_STATUS_IGNORE use a local status
#ifdef HAVE_MPI_STATUS_IGNORE
  if(cur == MPI_STATUS_IGNORE)
  {
        {{sub {{filter '^(const +)?MPI_Status' {{formals}} }} 'MPI_Status \*' ''}} = &temp;
  }
#endif 

  {{callfn}}
  
} {{endfn}}

/**
 * Arrays in wait & test
 */
{{fn fn_name    MPI_Testall
	MPI_Waitall
	MPI_Testsome
	MPI_Waitsome}}
{
  bool free = false;
  
  //if status is MPI_STATUSES_IGNORE alloc new status array of size count (first argument)
#ifdef HAVE_MPI_STATUSES_IGNORE
  if({{sub {{sub {{filter '^(const +)?MPI_Status ' {{formals}} }} 'MPI_Status[^a-zA-Z0-9]*' ''}} '\[\]' ''}} == MPI_STATUSES_IGNORE && {{args 0}} > 0)
  {
        {{sub {{sub {{filter '^(const +)?MPI_Status ' {{formals}} }} 'MPI_Status[^a-zA-Z0-9]*' ''}} '\[\]' ''}} = new MPI_Status [{{args 0}}];
		free = true;        
  }
#endif

  {{callfn}}

#ifdef HAVE_MPI_STATUSES_IGNORE
  if (free)
  {
  	delete [] {{sub {{sub {{filter '^(const +)?MPI_Status' {{formals}} }} 'MPI_Status[^a-zA-Z0-9]*' ''}} '\[\]' ''}};
  	{{sub {{sub {{filter '^(const +)?MPI_Status' {{formals}} }} 'MPI_Status[^a-zA-Z0-9]*' ''}} '\[\]' ''}} = MPI_STATUSES_IGNORE;
  }
#endif
} {{endfn}}
