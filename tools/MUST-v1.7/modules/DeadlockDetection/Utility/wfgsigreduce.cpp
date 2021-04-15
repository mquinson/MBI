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
 * @file wfgsigreduce.c
 * Implementation of the table based wfg representation with
 * Signal Reduction based detection.
 *
 * @author Tobias Hilbrich
 */

#include "wfgsigreduce.h"
#include "wfglib.h"
#include <stdio.h>
#include <stdlib.h>
/*#include "malloc.h"*/
#include <string.h>
#include <assert.h>
#include <unistd.h>

/* TODO used to control debug output */
#undef WFG_DEBUG
/*#define WFG_DEBUG*/

/*
 * Global Data.
 */
char *last_error = NULL;
int initialized = 0;
wfg_t wfg;
int *stack = NULL; /*low level integer stack used for DL detection*/

/*===============================================*/
/* set_error_message */
/*===============================================*/
void set_error_message (const char* string)
{
  if (last_error)
  {
    free(last_error);
    last_error = NULL;
  }

  last_error = (char*) malloc ((strlen(string)+1)*sizeof(char));
  sprintf(last_error,"%s",string);
}

/*===============================================*/
/* wfg_get_last_error */
/*===============================================*/
int wfg_get_last_error (int max_len, char* string)
{
  if (last_error)
  {
    if (strlen(last_error)+1 > max_len)
      return strlen(last_error)+1;

    sprintf(string,"%s",last_error);
    free (last_error);
    last_error = NULL;
    return 0;        
  }
  
  /* there was no error set ! */
  return -1;
}

/*===============================================*/
/* wfg_initialize */
/* complexity: O(N^2) */
/*===============================================*/
WFG_RETURN wfg_initialize (int numnodes)
{
  int i,j;
  
  if (initialized)
  {
    set_error_message("WFG already initialized, call"
       "wfg_finalize before initializing again.");
    return WFG_ERROR;
  }

  wfg.size = numnodes;

  /*allocate the table*/
  wfg.table = (cell_t**) malloc (wfg.size * sizeof(cell_t*)); 
  if (!wfg.table) {set_error_message("Out of memory.");return WFG_ERROR;}
  
  for (i = 0; i < wfg.size; i++)
  {
    wfg.table[i] = (cell_t*) malloc (wfg.size*sizeof(cell_t));
    if (!wfg.table[i]) {set_error_message("Out of memory.");return WFG_ERROR;}
    
    for (j = 0; j < wfg.size; j++)
    {
      wfg.table[i][j].i = i;
      wfg.table[i][j].j = j;
      wfg.table[i][j].count = 0;
      wfg.table[i][j].p_right = NULL;
      wfg.table[i][j].p_left = NULL;
      wfg.table[i][j].p_up = NULL;
      wfg.table[i][j].p_down = NULL;
    }
  }

  /*allocate the pointer arrays*/
  wfg.row_pointers = (header_p_t*) calloc (wfg.size, sizeof (header_p_t));  
  if (!wfg.row_pointers) {set_error_message("Out of memory.");return WFG_ERROR;}

  wfg.col_pointers = (header_p_t*) calloc (wfg.size, sizeof (header_p_t));  
  if (!wfg.col_pointers) {set_error_message("Out of memory.");return WFG_ERROR;}

  /*allocate the row flags*/
  wfg.row_flags = (row_flags_t*) calloc (wfg.size, sizeof (row_flags_t));
  
  for (i = 0; i < wfg.size; i++)
  {
    wfg.row_flags[i].node = i;
    wfg.row_flags[i].type = WFG_ARC_AND;
    wfg.row_flags[i].det.cur_out_count = 0;
    wfg.row_flags[i].det.is_sink = 1;
    wfg.row_flags[i].det.iteration_id = 0;
    wfg.row_flags[i].det.out_count = 0;
    wfg.row_flags[i].det.p_next_sink = NULL;
    wfg.row_flags[i].det.p_to_check_next = NULL;
  }
  
  /* Node to check list stuff */
  wfg.p_to_check_first = NULL;
  wfg.p_to_check_last = NULL;
  
  /* integer stack for DL detection */
  /* TODO less memory should be sufficient too ! */
  stack = (int*) malloc (sizeof(int)*wfg.size*wfg.size); /*low level integer stack*/
  
  /* Initialize the detection stuff */
  singnal_red.iteration_number = 1;
  singnal_red.node_part_of_dl = -1;
  singnal_red.deadlocked_nodes = (int*) calloc (wfg.size, sizeof (int)); 
  
  /* we initialized */
  initialized = 1;  

  return WFG_SUCCESS;  
}

/*===============================================*/
/* wfg_add_arc */
/* complexity: O(1) */
/*===============================================*/
WFG_RETURN wfg_add_arc (int node, int target, WFG_ARC_TYPE type)
{
  assert ((target < wfg.size) &&
          (node < wfg.size  )    );

  /* is this arc already there ? */
  if ( (wfg.table[node][target].p_left != NULL) || 
       (wfg.table[node][target].p_right != NULL) ||
       (wfg.row_pointers[node].p_first == &(wfg.table[node][target])) || 
       (wfg.col_pointers[target].p_first == &(wfg.table[node][target]))   )

  {
    wfg.table[node][target].count++;
    return WFG_SUCCESS;
  }
  
  /* arc is new, set count to 1 */
  wfg.table[node][target].count = 1;
  wfg.row_flags[node].det.out_count++;
  
  /* Do we have to add this node to the list of nodes to check ? */
  if (wfg.row_flags[node].det.is_sink)
  {
    int sdepth;
    cell_t*  p_cur_arc;
    int cur_node;
    int do_continue;
    
    /*invalidate fan-in recursive!*/
    stack[0] = node;
    sdepth = 1;

    while (sdepth)
    {
      sdepth--;
      cur_node = stack[sdepth];
      
      if (!wfg.row_flags[cur_node].det.is_sink)
        continue;
      
      /* Special stop for OR semantic nodes
       * If an or node has at least one outgoing
       * arc to sink it is still a sink.
       */
      if (wfg.row_flags[cur_node].type == WFG_ARC_OR)
      {
        do_continue = 0;
        /* Iterate over all the outgoing arcs */
        p_cur_arc = wfg.row_pointers[cur_node].p_first;
        while (p_cur_arc != NULL)
        {
          /* Ignore the node to which we are just adding arcs !*/
          if (p_cur_arc->j == node)
          {
            p_cur_arc = p_cur_arc->p_right;
            continue;
          }
          
          /* is it a "real sink" ? */
          if (!wfg.row_pointers[p_cur_arc->j].p_first)
          {
            do_continue=1;
            break;
          }
               
          p_cur_arc = p_cur_arc->p_right;
        }
        
        /* Has an OR arc to a real sink, so still a sink itself */
        if (do_continue)
          continue;
      }
      
      
      wfg.row_flags[cur_node].det.is_sink = 0;
      
      /* Add to to check list */
      if ( (wfg.p_to_check_last != &(wfg.row_flags[cur_node])) &&
           (wfg.row_flags[cur_node].det.p_to_check_next == NULL) )
      {
        if (wfg.p_to_check_last != NULL)
        {
          /* new last node */
          wfg.p_to_check_last->det.p_to_check_next = &(wfg.row_flags[cur_node]);
          wfg.p_to_check_last = &(wfg.row_flags[cur_node]);
          wfg.row_flags[cur_node].det.p_to_check_next = NULL;
        }
        else
        {
          /* is first and last node */
          wfg.p_to_check_first = 
            wfg.p_to_check_last = &(wfg.row_flags[cur_node]);
          wfg.row_flags[cur_node].det.p_to_check_next = NULL;          
        }
      }
      
      /* Iterate over all the incoming arcs */
      p_cur_arc = wfg.col_pointers[cur_node].p_first;
      while (p_cur_arc != NULL)
      {
        /* handle arc */
        stack[sdepth] = p_cur_arc->i;
        sdepth++; 
        
        p_cur_arc = p_cur_arc->p_down;
      }
    }
  } /*Invalidate fan-in*/
  
  /* adding to fan out of node "node" */
  if (wfg.row_pointers[node].p_first == NULL)
  {
    /*first out arc for node*/
    wfg.row_pointers[node].p_first = &(wfg.table[node][target]);
    wfg.row_pointers[node].p_last = &(wfg.table[node][target]);
    wfg.row_flags[node].type = type;
    wfg.table[node][target].p_left = NULL;
    wfg.table[node][target].p_right = NULL;
  }
  else
  { 
    /* does new arc type fit the old one ? */
    if (type != wfg.row_flags[node].type)
    {
      set_error_message("Illegal add, this node already has outgoing"
         "arcs of another arc type. In a AND||OR wfg all nodes may"
         "only use one arc type.");
      return WFG_ERROR;       
    }

    /* append the table element to the row list */
    wfg.row_pointers[node].p_last->p_right = &(wfg.table[node][target]);
    wfg.table[node][target].p_left = wfg.row_pointers[node].p_last;
    wfg.table[node][target].p_right = NULL;
    wfg.row_pointers[node].p_last = &(wfg.table[node][target]);
  }  

  /* adding to fan in of node "target" */
  if (wfg.col_pointers[target].p_first == NULL)
  {
    /*first in arc for target*/
    wfg.col_pointers[target].p_first = &(wfg.table[node][target]);
    wfg.col_pointers[target].p_last = &(wfg.table[node][target]);
    wfg.table[node][target].p_up = NULL;
    wfg.table[node][target].p_down = NULL;
  }
  else
  {
    /* append the table element to the column list */
    wfg.col_pointers[target].p_last->p_down = &(wfg.table[node][target]);
    wfg.table[node][target].p_up = wfg.col_pointers[target].p_last;
    wfg.table[node][target].p_down = NULL;
    wfg.col_pointers[target].p_last = &(wfg.table[node][target]);    
  }

  return WFG_SUCCESS;    
}

/*===============================================*/
/* wfg_add_arcs */
/* complexity: O(k) where k is num_arcs */
/*===============================================*/
WFG_RETURN wfg_add_arcs (int node, int num_arcs, int* targets, WFG_ARC_TYPE type)
{
  WFG_RETURN ret = WFG_SUCCESS;
  int i;

  for (i = 0; i < num_arcs; i++)
  {
    ret = wfg_add_arc (node,targets[i],type);
    if (ret != WFG_SUCCESS)
      return WFG_ERROR;
  }  

  return ret;    
}

/*===============================================*/
/* wfg_add_arcs_to_all */
/* complexity: O(N) */
/*===============================================*/
WFG_RETURN wfg_add_arcs_to_all (int node, WFG_ARC_TYPE type)
{
  /** 
   * @todo one might increase performance by implementing
   *       this as special version instead of calling add_arc.
   */

  WFG_RETURN ret = WFG_SUCCESS;
  int i;

  for (i = 0; i < wfg.size; i++)
  {
    ret = wfg_add_arc (node,i,type);
    if (ret != WFG_SUCCESS)
      return WFG_ERROR;
  }  

  return ret;
}

/*===============================================*/
/* wfg_remove_arc */
/* complexity: O(1) */
/*===============================================*/
WFG_RETURN wfg_remove_arc (int node, int target)
{
  /* validity check */
  assert ((target < wfg.size) &&
          (node < wfg.size  )    );
          
  /* does the arc exists? */
  if ( 
       ( 
         (wfg.table[node][target].p_left == NULL) && 
         (wfg.row_pointers[node].p_first != &(wfg.table[node][target])) 
       ) ||
       ( 
         (wfg.table[node][target].p_right == NULL) && 
         (wfg.row_pointers[node].p_last != &(wfg.table[node][target])) 
       )
     )
  {
    /* error, it is not in the list! */
    set_error_message("Illegal remove, this node does not have"
                      " the specified arc.");
    return WFG_ERROR;
  }
  
  /* Decrement the arc count. */
  wfg.table[node][target].count--;
  
  /* do we have an arc left ? */
  if (wfg.table[node][target].count > 0)
  {   
    return WFG_SUCCESS;
  }
  
  /* decrement out count */
  wfg.row_flags[node].det.out_count--;
  
  /* remove from fan-out list */
  /* fix left neighbor or row p_first*/
  if (wfg.row_pointers[node].p_first == &(wfg.table[node][target]))
  {
    /* is it the first element of the fan-out list */
    wfg.row_pointers[node].p_first = wfg.table[node][target].p_right;
  }
  else
  {
    /* it has a predecessor in the list */
    wfg.table[node][target].p_left->p_right = wfg.table[node][target].p_right;    
  }
  
  /* fix right neighbor or row p_last */
  if (wfg.row_pointers[node].p_last == &(wfg.table[node][target]))
  {
    /* it is the last entry */
    wfg.row_pointers[node].p_last = wfg.table[node][target].p_left;
  }
  else
  {
    /* right neighbor exists */
    wfg.table[node][target].p_right->p_left = wfg.table[node][target].p_left;
  }
      
  /* remove from fan-in list */
  /* fix uper neighbor or col p_first*/
  if (wfg.col_pointers[target].p_first == &(wfg.table[node][target]))
  {
    /* is it the first element of the fan-in list */
    wfg.col_pointers[target].p_first = wfg.table[node][target].p_down;
  }
  else
  {
    /* it has a predecessor in the list */
    wfg.table[node][target].p_up->p_down = wfg.table[node][target].p_down;    
  }
  
  /* fix lower neighbor or col p_last */
  if (wfg.col_pointers[target].p_last == &(wfg.table[node][target]))
  {
    /* it is the last entry */
    wfg.col_pointers[target].p_last = wfg.table[node][target].p_up;
  }
  else
  {
    /* right neighbor exists */
    wfg.table[node][target].p_down->p_up = wfg.table[node][target].p_up;
  }
  
  /* kill the pointers in the node */
  wfg.table[node][target].p_left = NULL;
  wfg.table[node][target].p_right = NULL;
  wfg.table[node][target].p_up = NULL;
  wfg.table[node][target].p_down = NULL;
  
  return WFG_SUCCESS;    
}

/*===============================================*/
/* wfg_remove_arcs */
/* complexity: O(k) where k is num_arcs */
/*===============================================*/
WFG_RETURN wfg_remove_arcs (int node, int num_arcs,int* targets)
{
  WFG_RETURN ret = WFG_SUCCESS;
  int i;

  for (i = 0; i < num_arcs; i++)
  {
    ret = wfg_remove_arc (node,targets[i]);
    if (ret != WFG_SUCCESS)
      return WFG_ERROR;
  }
  
  return ret;    
}

/*===============================================*/
/* wfg_remove_all_arcs_node */
/* complexity: ??? */
/*===============================================*/
WFG_RETURN wfg_remove_all_arcs_node (int node)
{
  /** 
   * @todo one might increase performance by implementing
   *       this as special version instead of calling add_arc.
   */

  WFG_RETURN ret = WFG_SUCCESS;
  int j;
  
  while (wfg.row_pointers[node].p_first)
  {
    j = wfg.row_pointers[node].p_first->j;
    wfg.table[node][j].count = 1;
    ret = wfg_remove_arc (node,j);
  }
  
  return ret;
}

/*===============================================*/
/* wfg_clean */
/* complexity: O(N^2) */
/*===============================================*/
WFG_RETURN wfg_clean (void)
{
  int i,j;
  
  for (i = 0; i < wfg.size; i++)
  {
    wfg.col_pointers[i].p_first = 
     wfg.col_pointers[i].p_last = 
      wfg.row_pointers[i].p_first = 
       wfg.row_pointers[i].p_last = NULL;
       
    wfg.row_flags[i].type = WFG_ARC_AND;
    wfg.row_flags[i].node = i;
    wfg.row_flags[i].det.cur_out_count = 0;
    wfg.row_flags[i].det.is_sink = 0;
    wfg.row_flags[i].det.iteration_id = 0;
    wfg.row_flags[i].det.out_count = 0;
    wfg.row_flags[i].det.p_next_sink = NULL;
    wfg.row_flags[i].det.p_to_check_next = NULL;
    
    for (j = 0; j < wfg.size; j++)
    {
      wfg.table[i][j].p_down =
       wfg.table[i][j].p_up =
        wfg.table[i][j].p_left = 
         wfg.table[i][j].p_right = NULL;
      
      wfg.table[i][j].count = 0;  
    }
  }
  
  wfg.p_to_check_first = 
    wfg.p_to_check_last = NULL;
  
  return WFG_SUCCESS;    
}

/*===============================================*/
/* wfg_finalize */
/* complexity: ??? */
/*===============================================*/
WFG_RETURN wfg_finalize (void)
{
  int i;
  
  if (!initialized)
  {
    /* error, it is not in the list! */
    set_error_message("Tried to finalize a unitialized wfg.");
    return WFG_ERROR;
  }
  
  if (wfg.col_pointers)
    free (wfg.col_pointers);
  wfg.col_pointers = NULL;
  
  if (wfg.row_pointers)
    free (wfg.row_pointers);
  wfg.row_pointers = NULL;
  
  if (wfg.table)
  {
    for (i = 0; i < wfg.size; i++)
  {
    if (wfg.table[i])
      free (wfg.table[i]);
    wfg.table[i] = NULL;
  }
  
    free (wfg.table);
  }
  wfg.table = NULL;
  
  if (stack)
    free (stack);
  stack = NULL;
  
  initialized=false;

  return WFG_SUCCESS;    
}

/*===============================================*/
/* wfg_deadlock_check */
/* complexity: ??? */
/*===============================================*/
WFG_RETURN wfg_deadlock_check (void)
{
  /**
   * @note
   * one might also come up with a tree based approach here
   * this is interesting as it avoids the undo "madness".
   * 
   * - Start with some "interesting sink"
   * - loop over all the incoming arcs
   *   + if starting node is marked as sink
   *     # continue
   *   + if the incoming arc is OR 
   *     # add node to sinks to look at
   *     # increment amount of sinks by one
   *   + if the incoming arc is AND
   *     # decrement nodes out count (use a magic value for that)
   *     # if rmeaining out count == 0
   *       ~ add the node to the sinks to look at
   *       ~ increment amount of sinks by one
   * - if amount of sinks not amount of nodes search the OR Knot
   * 
   * ? How to avoid redoing it for all the unchanged nodes again and again ?   
   */
  
  int  iteration_id; /* Used to invalidate old data without having to loop over it. */
  row_flags_t*  p_start_node = wfg.p_to_check_first;
  row_flags_t*  p_temp_node;
  int       start_node,
                cur_node; 
  int           sdepth = 0;
  int           component_size;
  row_flags_t*  p_first_sink = NULL;
  row_flags_t*  p_last_sink = NULL;
  row_flags_t*  p_temp_sink = NULL;
  row_flags_t*  p_temp_sink_copy = NULL;
  cell_t*       p_cur_arc;
  int           temp_num_arcs_added;
  
  /* loop over all the nodes we have to look at */
  while (p_start_node != NULL)
  {
    /* get the node number and set the stuff for the next node */
    start_node = p_start_node->node;
    p_temp_node = p_start_node; 
    p_start_node = p_start_node->det.p_to_check_next;
    p_temp_node->det.p_to_check_next = NULL;
    p_first_sink = NULL;
    p_last_sink = NULL;
        
    if (wfg.row_flags[start_node].det.is_sink)
    {
      /* The node was already considered in a check */
      continue;
    }
    
    /*increment and set the iteration number*/
    singnal_red.iteration_number++;
    iteration_id = singnal_red.iteration_number;
    
#ifdef WFG_DEBUG
    printf ("\nPreparing: %d\n",start_node);
#endif
   
    /* I) search step:
     *     - find amount of reachable nodes
     *     - find the sinks 
     */
    stack[0] = start_node;
    sdepth = 1;
    component_size = 0;
    
    while (sdepth > 0)
    {
      sdepth--;
      cur_node = stack[sdepth];
      
      /* This node already handled ? */
      if (wfg.row_flags[cur_node].det.iteration_id == iteration_id)
        continue;

      if (wfg.row_flags[cur_node].det.is_sink)
      {
        /* add to the sink list */
        if (p_first_sink == NULL)
        {
          p_first_sink = 
            p_last_sink = &(wfg.row_flags[cur_node]);
        }
        else
        {
          p_last_sink->det.p_next_sink = &(wfg.row_flags[cur_node]);
          p_last_sink = &(wfg.row_flags[cur_node]);
        }
        component_size++;
        continue;
      }
      
      /* handle this node */
      wfg.row_flags[cur_node].det.cur_out_count = wfg.row_flags[cur_node].det.out_count;
      wfg.row_flags[cur_node].det.iteration_id = iteration_id;
      wfg.row_flags[cur_node].det.p_next_sink = NULL;  
      
      /* iterate over the out arcs */
      p_cur_arc = wfg.row_pointers[cur_node].p_first;
      temp_num_arcs_added = 0; /* Used to perform an stack roleback if we have a OR node. */
        
      while (p_cur_arc != NULL)
      {
        /* handle arc */
        if ( (wfg.row_flags[p_cur_arc->j].det.is_sink) )
        {
          /* Arc was already handled by DL check. 
           * If this node was deadlocked we would have aborted at
           * that point, as a result we can assume it will not deadlock!
           */
          if (wfg.row_flags[cur_node].type == WFG_ARC_AND)
          {
            /* Just ignore this one arc */
            wfg.row_flags[cur_node].det.cur_out_count--;
            p_cur_arc = p_cur_arc->p_right;
            continue;
          }/*I am a AND node*/
          else
          {
            /* Ignore all outgoing arcs */
            wfg.row_flags[cur_node].det.cur_out_count = 0;
            sdepth -= temp_num_arcs_added; /*remove the recursion of the other node arcs*/
            break;
          }/*I am a OR node*/
        } /* Arc target already DL checked */
        
        /* Add target node to the recursion */
        stack[sdepth] = p_cur_arc->j;
        sdepth++; 
        temp_num_arcs_added++;
          
        /* next out arc */
        p_cur_arc = p_cur_arc->p_right;
      }/* while out arcs left */
                
      /* Is it a sink ? */
      if (wfg.row_flags[cur_node].det.cur_out_count == 0)
      {
        /* mark as sink */
        wfg.row_flags[cur_node].det.is_sink = 1;
        
        /* add to the sink list */
        if (p_first_sink == NULL)
        {
          p_first_sink = 
            p_last_sink = &(wfg.row_flags[cur_node]);
        }
        else
        {
          p_last_sink->det.p_next_sink = &(wfg.row_flags[cur_node]);
          p_last_sink = &(wfg.row_flags[cur_node]);
        }
      }      
      
      /* Increment the component size */
      component_size++;
    }/* while stack not empty */
    
#ifdef WFG_DEBUG
    printf("===================\n");
    printf("==COMPONENT========\n");
    printf("===================\n");
    printf("component_size=%d\n",component_size);
    printf("sinks={");
    p_temp_sink = p_first_sink;
    while (p_temp_sink != NULL)
    {
      printf ("%d,",p_temp_sink->node);     
      p_temp_sink = p_temp_sink->det.p_next_sink;
    }
    printf("}\n");
#endif
    
    /*
     * II) signal reduction step:
     *      - use the sinks and perform signal reduction
     */
    p_temp_sink = p_first_sink;
    
    while (p_temp_sink != NULL)
    {
      /* Handle the sink */
      cur_node = p_temp_sink->node;
      component_size--; /* one more component is a sink */ 
      
      /* Iterate over all the incoming arcs */
      p_cur_arc = wfg.col_pointers[cur_node].p_first;
              
      while (p_cur_arc != NULL)
      {
        /* handle arc */
        if (wfg.row_flags[p_cur_arc->i].det.iteration_id != iteration_id)
        {
          /* Arc is not in the component */
          p_cur_arc = p_cur_arc->p_down;
          continue;
        }
          
        if (wfg.row_flags[p_cur_arc->i].det.is_sink)
        {
          /* Arc is already a sink, so it is either already handled or
           * in the sink list
           */
          p_cur_arc = p_cur_arc->p_down;
          continue;
        }
        
        /* Modify the node from which the arc starts */
        if (wfg.row_flags[p_cur_arc->i].type == WFG_ARC_AND)
        {
          /* Signal reduce for AND arc removes one arc */
          wfg.row_flags[p_cur_arc->i].det.cur_out_count--;
        }
        else
        {
          /* Signal reduce for OR arc removes all arcs */
          wfg.row_flags[p_cur_arc->i].det.cur_out_count = 0;        
        }      
        
        /* Is the node now a sink ? */
        if (wfg.row_flags[p_cur_arc->i].det.cur_out_count == 0)
        {
          /* Add the new sink to the list */
          p_last_sink->det.p_next_sink = &(wfg.row_flags[p_cur_arc->i]);
          p_last_sink = &(wfg.row_flags[p_cur_arc->i]);
          wfg.row_flags[p_cur_arc->i].det.p_next_sink = NULL;
            
          /* Set as sink */
          wfg.row_flags[p_cur_arc->i].det.is_sink = 1;            
        }   
        
        /* next in-arc */
        p_cur_arc = p_cur_arc->p_down;
      }/* while in arcs left */
      
      /* Next sink */
      p_temp_sink_copy = p_temp_sink; 
      p_temp_sink = p_temp_sink->det.p_next_sink;
      p_temp_sink_copy->det.p_next_sink = NULL; /* save as handled */ 
    }
    
    /* Reset first and last sink */
    p_first_sink = 
      p_last_sink = NULL;
      
#ifdef WFG_DEBUG
    printf("component_size_new=%d\n",component_size);
#endif
    
    /* TODO make that nice, what do we have to clean up ? */
    if (component_size != 0)
    {
      wfg.p_to_check_first = NULL;
      wfg.p_to_check_last = NULL;
      singnal_red.node_part_of_dl = start_node;
      return WFG_DEADLOCK;
    }
    
  } /* for nodes to check */
   
  /* reset the node to check list pointers */
  wfg.p_to_check_first = NULL;
  wfg.p_to_check_last = NULL;  
  
  return WFG_SUCCESS;    
}

/*===============================================*/
/* wfg_get_deadlocked_nodes */
/* complexity: ??? */
/*===============================================*/
WFG_RETURN wfg_get_deadlocked_nodes (int* out_count, int* out_nodes)
{
  int origin;
  int component_size;
  int sdepth;
  int cur_node;
  int iteration_id;
  row_flags_t*  p_first_sink = NULL;
  row_flags_t*  p_last_sink = NULL;
  row_flags_t*  p_temp_sink = NULL;
  row_flags_t*  p_temp_sink_copy = NULL;
  cell_t*       p_cur_arc;
  int i;
  int knot_found;
    
  /* 
   * Implements the detection algorithm that is part of my thesis 
   */
  
  /* 1) perform an actual signal reduction on the data*/
  origin = singnal_red.node_part_of_dl;

  /* I) search step:
   *     - find the sinks 
   */
  stack[0] = origin;
  sdepth = 1;
  component_size = 0;
  singnal_red.iteration_number++;
  iteration_id = singnal_red.iteration_number;

  while (sdepth > 0)
  {
    sdepth--;
    cur_node = stack[sdepth];

    /* This node already handled ? */
    if (wfg.row_flags[cur_node].det.iteration_id == iteration_id)
      continue;

    /* handle this node */
    wfg.row_flags[cur_node].det.iteration_id = iteration_id;
    wfg.row_flags[cur_node].det.p_next_sink = NULL;
    wfg.row_flags[cur_node].det.is_sink = 0;

    /* iterate over the out arcs */
    p_cur_arc = wfg.row_pointers[cur_node].p_first;

    while (p_cur_arc != NULL)
    {
      /* Add target node to the recursion */
      stack[sdepth] = p_cur_arc->j;
      sdepth++; 

      /* next out arc */
      p_cur_arc = p_cur_arc->p_right;
    }/* while out arcs left */

    /* Is it a sink ? */
    if (wfg.row_flags[cur_node].det.out_count == 0)
    {
      /* mark as sink */
      wfg.row_flags[cur_node].det.is_sink = 1;

      /* add to the sink list */
      if (p_first_sink == NULL)
      {
        p_first_sink = 
          p_last_sink = &(wfg.row_flags[cur_node]);
      }
      else
      {
        p_last_sink->det.p_next_sink = &(wfg.row_flags[cur_node]);
        p_last_sink = &(wfg.row_flags[cur_node]);
      }
    }      

    /* Increment the component size */
    component_size++;
  }/* while stack not empty */

#ifdef WFG_DEBUG
  printf("===================\n");
  printf("==COMPONENT========\n");
  printf("===================\n");
  printf("component_size=%d\n",component_size);
  printf("sinks={");
  p_temp_sink = p_first_sink;
  while (p_temp_sink != NULL)
  {
    printf ("%d,",p_temp_sink->node);     
    p_temp_sink = p_temp_sink->det.p_next_sink;
  }
  printf("}\n");
#endif

  /*
   * II) signal reduction step:
   *      - use the sinks and perform signal reduction
   */
  p_temp_sink = p_first_sink;

  while (p_temp_sink != NULL)
  {
    /* Handle the sink */
    cur_node = p_temp_sink->node;
    component_size--; /* one more component is a sink */ 

    /* Signal reduce until no more incoming arcs! */
    while (wfg.col_pointers[cur_node].p_first)
    {
      p_cur_arc = wfg.col_pointers[cur_node].p_first;
  
      /* handle arc */
      /*We have to reduce, otherwise still incoming arcs */
      if (wfg.row_flags[p_cur_arc->i].det.iteration_id != iteration_id)
      {
        /* Arc is not in the component, reduce semantics unimportant */
        wfg_remove_all_arcs_node (p_cur_arc->i);
        p_cur_arc = p_cur_arc->p_down;
        continue;
      }

      /* Modify the node from which the arc starts */
      if (wfg.row_flags[p_cur_arc->i].type == WFG_ARC_AND)
      {
        /* Signal reduce for AND arc removes one arc */
        while (p_cur_arc->count)
          wfg_remove_arc(p_cur_arc->i,cur_node);
      }
      else
      {
        /* Signal reduce for OR arc removes all arcs */
        wfg_remove_all_arcs_node (p_cur_arc->i);        
      }      

      /* Is the node now a sink ? */
      if (wfg.row_flags[p_cur_arc->i].det.out_count == 0)
      {
        /* Add the new sink to the list */
        p_last_sink->det.p_next_sink = &(wfg.row_flags[p_cur_arc->i]);
        p_last_sink = &(wfg.row_flags[p_cur_arc->i]);
        wfg.row_flags[p_cur_arc->i].det.p_next_sink = NULL;

        /* Set as sink */
        wfg.row_flags[p_cur_arc->i].det.is_sink = 1;            
      }   
    }/*while in arcs left*/

    /* Next sink */
    p_temp_sink_copy = p_temp_sink; 
    p_temp_sink = p_temp_sink->det.p_next_sink;
    p_temp_sink_copy->det.p_next_sink = NULL; /* save as handled */ 
  }

  /* Reset first and last sink */
  p_first_sink = 
    p_last_sink = NULL;

#ifdef WFG_DEBUG
  printf("component_size_new=%d\n",component_size);
#endif

  if (component_size == 0)
  {
    set_error_message("Asked for deadlocked nodes, but no deadlock present!");
    return WFG_ERROR;
  }
  
  /* Get a starting node in the component */
  for (i = 0; i < wfg.size; i++)
  {
    /* Node of the component ? */
    if (wfg.row_flags[i].det.iteration_id != iteration_id)
      continue;
    
    if (wfg.row_flags[i].det.out_count)
    {
      origin = i;
      singnal_red.node_part_of_dl = i; /*For Debuging, but should not hurt in general!*/
      break;
    }
  }
  
  /* Lets to the thesis algorithm */
  knot_found = 0;
  /* Note: "origin" is our starting node !*/
  /* Note: to construct lists we use the sink list pointers, hady little friends they are */
  while (!knot_found)
  {
#ifdef WFG_DEBUG
    printf ("New Origin: %d\n",origin);
#endif
    
    /* find a circle for origin */
    wfg.row_flags[origin].det.p_next_sink = NULL;
    construct_cycle (
        origin /*current node*/,
        &origin /*output: some node in the circle*/);
    
#ifdef WFG_DEBUG
  printf("Cycle={");
  p_temp_sink = wfg.row_flags[origin].det.p_next_sink;
  while (p_temp_sink)
  {
    printf ("%d,",p_temp_sink->node);
    if (p_temp_sink->node == origin)
      p_temp_sink = NULL;
    else
      p_temp_sink = p_temp_sink->det.p_next_sink;
  }
  printf("}\n");
#endif
  
    /* mark nodes in the cycle */
    singnal_red.iteration_number++;
    iteration_id = singnal_red.iteration_number;
    
    p_temp_sink = wfg.row_flags[origin].det.p_next_sink;
    while (p_temp_sink)
    {
      p_temp_sink->det.iteration_id = iteration_id;
      if (p_temp_sink->node == origin)
        p_temp_sink = NULL;
      else
        p_temp_sink = p_temp_sink->det.p_next_sink;
    }
    
    /*enlarge the cycle until no outgoing OR arcs*/
    do 
    {
      /* Is there an outgoing or arc*/
      knot_found = 1;
      for (i = 0;i < wfg.size; i++)
      {
        if (wfg.row_flags[i].det.iteration_id != iteration_id)
          continue;
        
        if (wfg.row_flags[i].type == WFG_ARC_AND)
          continue;
        
        /* iterate over the out arcs */
        p_cur_arc = wfg.row_pointers[i].p_first;

        while (p_cur_arc != NULL)
        {
          /* Add target node to the recursion */
          if (wfg.row_flags[p_cur_arc->j].det.iteration_id != iteration_id)
          {
            knot_found = 0;
            if (!find_path (p_cur_arc->j,i))
            {
              /*No path present, restart with new origin*/
              knot_found = -1;
              origin = p_cur_arc->j;
            }
            else
            {
              /*add the path*/
              p_temp_sink = &(wfg.row_flags[p_cur_arc->j]);
              while (p_temp_sink)
              {
                p_temp_sink->det.iteration_id = iteration_id;
                p_temp_sink = p_temp_sink->det.p_next_sink;
              }
            }
            break;
          }
          
          /* next out arc */
          p_cur_arc = p_cur_arc->p_right;
        }/* while out arcs left */
        
        /*change, abort check*/
        if (knot_found != 1)
          break;
      }/*check*/
      
      /* new origin ? */
      if (knot_found == -1)
      {
        knot_found = 0;
        break;
      }
              
      /*changed something, check again ...*/
      if (!knot_found)
        continue;
      
      /*JUHAY, no violation we have a OR-Knot !!!*/
      break;
    }
    while ( 1 == 1 /*TODO This is bad design, think about better solutions ...*/);
  }/*while origin changing*/
  
  /*Remember the OR-Knot*/
#ifdef WFG_DEBUG
  printf ("OR-Knot={");
  for (i = 0;i < wfg.size; i++)
  {
    if (wfg.row_flags[i].det.iteration_id == iteration_id)
    {
      printf("%d,",i);  
    }
  }
  printf("}\n");
#endif
  
  if (out_nodes)
    *out_count = 0;
  
  for (i = 0;i < wfg.size; i++)
  {
    if (wfg.row_flags[i].det.iteration_id == iteration_id)
    {
      singnal_red.deadlocked_nodes[i] = 1;
    }
    else
    {
      singnal_red.deadlocked_nodes[i] = 0;
      continue;
    }
    
    if (out_nodes)
    {
      out_nodes[*out_count] = i;
      *out_count = *out_count + 1;
    }
  }
  
  return WFG_SUCCESS;    
}

/*===============================================*/
/* construct_circle */
/* complexity: probably O(N^2) but usually it will be small */
/*             in addition not of much interest ... */
/*===============================================*/
int construct_cycle (int cur_node, int *p_out_node_in_circle)
{
  int sdepth;
  int i;
  cell_t* next_node;
  
  sdepth = 0;
  stack[sdepth] = cur_node;
  sdepth++;
  
  /*prepare, set cur_outcount to zero for all nodes*/
  for (i = 0;i < wfg.size; i++)
    wfg.row_flags[i].det.cur_out_count = 0;
  
  /* look at all the possible paths */
  while (sdepth)
  {
    cur_node = stack[sdepth-1];
   
    /*Is already on stack ?*/
    for (i = 0; i < sdepth-1; i++)
    {
      if (stack[i] == cur_node)
      {
        /*cycle_found*/
        *p_out_node_in_circle = cur_node;
        
        /* the path is on the stack */
        for (i = sdepth-2; i >= 0;i--)
        {
          /*only go back till the cycle found*/
          if (stack[i] == cur_node)
            break;
          
          wfg.row_flags[stack[i]].det.p_next_sink = &(wfg.row_flags[stack[i+1]]);   
        }
        
        /*close cycle*/
        wfg.row_flags[cur_node].det.p_next_sink = &(wfg.row_flags[stack[i+1]]);
        return WFG_SUCCESS;
      }
    }
    
    /* No cycle yet */
    /* Add a reachable node to the stack ... */
    next_node = wfg.row_pointers[cur_node].p_first;
    for (i = 0;i < wfg.row_flags[cur_node].det.cur_out_count; i++)
    {
      if (next_node)
        next_node = next_node->p_right;
      else 
        break;
    }
    
    if (next_node)
    {
      wfg.row_flags[cur_node].det.cur_out_count++;
      stack[sdepth] = next_node->j;
      sdepth++;
      continue;
    }
    
    /*remove from stack*/
    sdepth--;
  }
  
  /* No cycle found */
  return WFG_ERROR;
}

/*===============================================*/
/* find_path */
/* complexity: I guess O(N^2) */
/*===============================================*/
int find_path (int from, int to)
{
  int sdepth;
  int i;
  cell_t* next_node;
  int cur_node;

  sdepth = 0;
  stack[sdepth] = from;
  sdepth++;

  /*prepare, set cur_outcount to zero for all nodes*/
  for (i = 0;i < wfg.size; i++)
    wfg.row_flags[i].det.cur_out_count = 0;

  /* look at all the possible paths */
  while (sdepth)
  {
    cur_node = stack[sdepth-1];

    if (cur_node == to)
    {
      /* the path is on the stack */
      for (i = 0; i < sdepth-1; i++)
      {
        wfg.row_flags[stack[i]].det.p_next_sink = &(wfg.row_flags[stack[i+1]]);   
      }

      wfg.row_flags[stack[sdepth-1]].det.p_next_sink = NULL;
      return 1;
    }

    /* No cycle yet */
    /* Add a reachable node to the stack ... */
    next_node = wfg.row_pointers[cur_node].p_first;
    for (i = 0;i < wfg.row_flags[cur_node].det.cur_out_count; i++)
    {
      if (next_node)
        next_node = next_node->p_right;
      else 
        break;
    }

    if (next_node)
    {
      wfg.row_flags[cur_node].det.cur_out_count++;
      stack[sdepth] = next_node->j;
      sdepth++;
      continue;
    }

    /*remove from stack*/
    sdepth--;
  }
  
  return 0;
}

/*===============================================*/
/* wfg_print_wfg */
/* complexity: O(N^2) */
/*===============================================*/
WFG_RETURN wfg_print_wfg (char *filename, get_arc_label_f arc_function, get_node_label_f node_function)
{
  int i;
  FILE *file = NULL;
  cell_t* p;
  
  file = fopen (filename,"w");
  if (!file)
  {
    set_error_message("Failed to open output file.");
    return WFG_ERROR;
  }
  
  fprintf (file,"digraph finite_state_machine {\n");
  fprintf (file,"size=\"7,10\"\n");
  /*fprintf (file,"node [color = black];\n");*/
  fprintf (file,"node [shape = circle]\n");
  
  /* Print a node for the node labels */
  fprintf(file, "\nlabelnode [rank = source, shape = record, label = \"");
  for (i = 0; i < wfg.size; i++)
  {
    fprintf (file, "%d:%s",i,node_function(i));
    
    if (i != wfg.size-1)
      fprintf (file, " | ");
  }
  fprintf(file, "\"];\n");
  fprintf(file, "\"labelnode\" -> \"0\" [color = white]\n");
  
  /* Print using IN-Arcs lists (for verification) */
  for (i = 0; i < wfg.size; i++)
  {
    p = wfg.col_pointers[i].p_first;
    
    while (p)
    {
      assert (p->j == i);
      
      if (wfg.row_flags[p->i].type == WFG_ARC_AND)
        fprintf (file,"\"%d\" -> \"%d\" [label=\"(%d) %s\"];\n",p->i,i,p->count, arc_function(p->i,i));
      else 
        fprintf (file,"\"%d\" -> \"%d\" [style=dotted,label=\"(%d) %s\"];\n",p->i,i,p->count,arc_function(p->i,i));
        
      p = p->p_down;
    }
  }

  fprintf (file,"}\n");
  fclose (file);
  
  return WFG_SUCCESS;
}

/*===============================================*/
/* wfg_print_deadlock */
/* complexity: O(N^2) */
/*===============================================*/
WFG_RETURN wfg_print_deadlock (char *filename, get_arc_label_f arc_function, get_node_label_f node_function)
{
  int i;
  FILE *file = NULL;
  cell_t* p;
  int is_first;
  int some_node = -1;
  
  file = fopen (filename,"w");
  if (!file)
  {
    set_error_message("Failed to open output file.");
    return WFG_ERROR;
  }
  
  fprintf (file,"digraph deadlock {\n");
  fprintf (file,"size=\"7,10\"\n");
  fprintf (file,"node [color = red, fontcolor = red];\n");
  fprintf (file,"node [shape = circle]\n");
  
  /* Print a node for the node labels */
  fprintf(file, "\nlabelnode [rank = source, fontcolor = red, shape = record, label = \"");
  is_first = 1;
  for (i = 0; i < wfg.size; i++)
  {
    if (singnal_red.deadlocked_nodes[i])
    {
      if (!is_first)
        fprintf (file, " | ");
      
      fprintf (file, "%d:%s",i,node_function(i));
      is_first = 0;
      
      if (some_node == -1)
        some_node = i;
    }
  }
  fprintf(file, "\"];\n");
  fprintf(file, "\"labelnode\" -> \"%d\" [color = white]\n",some_node);
  
  /* Print using IN-Arcs lists (for verification) */
  for (i = 0; i < wfg.size; i++)
  {
    p = wfg.col_pointers[i].p_first;
    
    while (p)
    {
      assert (p->j == i);
      
      if (singnal_red.deadlocked_nodes[i] &&
          singnal_red.deadlocked_nodes[p->i])
      {
        if (wfg.row_flags[p->i].type == WFG_ARC_AND)
          fprintf (file,"\"%d\" -> \"%d\" [fontcolor = red, color = red, label=\"(%d) %s\"];\n",p->i,i,p->count, arc_function(p->i,i));
        else 
          fprintf (file,"\"%d\" -> \"%d\" [fontcolor = red, color = red, style=dotted,label=\"(%d) %s\"];\n",p->i,i,p->count,arc_function(p->i,i));
      }
        
      p = p->p_down;
    }
  }

  fprintf (file,"}\n");
  fclose (file);
  
  return WFG_SUCCESS;
}

/*EOF*/
