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
 * @file wfgsigreduce.h
 * Internal definitions for a table based WFG representation.
 * DL detection is done via Signal Reduction and should have
 * O(N^2) time and space complexity. Parallelization for that 
 * version should be relatively easy.
 *
 * @author Tobias Hilbrich
 */
 
#include "wfglib.h"

#ifndef WFGSIGREDUCE_H
#define WFGSIGREDUCE_H

/**
 * Datatype used for a cell. Each cell stores its coordinates
 * this is somewhat redundant, but cells will also be used in 
 * linked lists a lot, thus we have to know their coordinates.
 */
typedef struct _cell_t
{
  int i; /**< start node. */
  int j; /**< target node. */
  int count; /**< count of arcs from i to j. */
  struct _cell_t *p_right;
  struct _cell_t *p_left;
  struct _cell_t *p_up;
  struct _cell_t *p_down;
} cell_t;

/**
 * Type used for the pointers into the row and column lists.
 */
typedef struct _header_p_t
{
  cell_t* p_first;
  cell_t* p_last;
} header_p_t;

/**
 * Datatype used per cell for the detection algorithm.
 */
typedef struct _detection_node_t
 {
   int out_count; /**< Number of outgoing arcs. */
   int cur_out_count; /**< Remaining amount of out arcs during detection. */
   int is_sink /**< Is it a sink in the current detection ? */;
   int iteration_id; /**< Magic that determines whether "cur_out_count" and "is_sink" are valid. */
   struct _row_flags_t *p_next_sink; /**< Used to create a sink list without using malloc, very inefficent realization. */
   struct _row_flags_t *p_to_check_next;
 } detection_node_t;

/**
 * Type used for the row flags.
 */
typedef struct _row_flags_t
{
  int node; /** Which node am I ? */ 
  WFG_ARC_TYPE type; /** Arc type used by the node. */
  detection_node_t det; /**< Stuff for detection. */
} row_flags_t;

/**
 * Datatype for the complete AND||OR WFG.
 */
typedef struct _wfg_t
{
  int size;
  cell_t** table;
  header_p_t* row_pointers;
  header_p_t* col_pointers;
  row_flags_t* row_flags;
  
  /**
   * @note
   * We constuct a list of nodes that got outgoing arcs at some point.
   * A node is added if it got an outgoing arc. If all arcs are removed 
   * from a node in the list it still stays there, this might be a point
   * of optimization but I think removing would be more expensive than 
   * doing a deadlock detection for a node that reaches nothing.
   */
  row_flags_t* p_to_check_first; /**< First node that got new outgoing arcs. */
  row_flags_t* p_to_check_last; /**< Last node that got new outgoing arcs. */
} wfg_t;

extern wfg_t wfg;

/**
 * Datatype for background information for 
 * signal reduction.
 */
typedef struct _signal_red_t
{
  int iteration_number; /**< current amount of sinks */
  int node_part_of_dl; /**< If a deadlock is dtected this contains the number of a node that is connected to it. */
  int *deadlocked_nodes;
} signal_red_t;

signal_red_t singnal_red;

/*
 * Additional functions.
 */
/*TODO: add doxygen tags.*/
void set_error_message (const char* string);
int construct_cycle (int cur_node, int *p_out_node_in_circle);
int find_path (int from, int to);

#endif /* WFGSIGREDUCE_H */

/*EOF*/
