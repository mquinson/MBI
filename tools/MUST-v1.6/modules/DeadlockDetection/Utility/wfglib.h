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
 * @file wfglib.h
 * Library used to implement a AND||OR WFG and to detct deadlocks.
 * This header describes the interface to the WFG lib, its 
 * implementation may use an arbitrary internal representation.
 *
 * @author Tobias Hilbrich
 */

#ifndef WFGLIB_H
#define WFGLIB_H

/**
 * Enum used for the return values of the 
 * library functions.
 */
typedef enum WFG_RETURN_T 
{
  WFG_SUCCESS = 0,
  WFG_ERROR,
  WFG_DEADLOCK
} WFG_RETURN;

/**
 * Enum used to select the arc type. 
 */
typedef enum WFG_ARC_TYPE_T  
{
  WFG_ARC_AND,
  WFG_ARC_OR
} WFG_ARC_TYPE;

/**
 * Error description retrival function.
 * An error may only be retrieved once, afterwards it is deleted.
 * @param max_len maximum length for which string alocated.
 * @param string stores the error message and is allocated 
 *        for at least max_len elements.
 * @return 0 if the call is successfull, -1 if no error is 
 *         present and the length of the stored error message
 *         if that number is higher than max_len.
 */
int wfg_get_last_error (int max_len, char* string);

/* TODO: add doxygen tags ! */
WFG_RETURN wfg_initialize (int numnodes);
WFG_RETURN wfg_add_arc (int node, int target, WFG_ARC_TYPE type);
WFG_RETURN wfg_add_arcs (int node, int num_arcs, int* targets, WFG_ARC_TYPE type);
WFG_RETURN wfg_add_arcs_to_all (int node, WFG_ARC_TYPE type);
WFG_RETURN wfg_remove_arc (int node, int target);
WFG_RETURN wfg_remove_arcs (int node, int num_arcs,int* targets);
WFG_RETURN wfg_remove_all_arcs_node (int node);
WFG_RETURN wfg_clean (void);
WFG_RETURN wfg_finalize (void);

WFG_RETURN wfg_deadlock_check (void);
WFG_RETURN wfg_get_deadlocked_nodes (int* out_count, int* out_nodes);

/**
 * Function used to get a label for an arc.
 * It is used when printing a deadlock.
 */
typedef const char* (*get_arc_label_f) (int,int);

/**
 * Function used to get a label for a node.
 * It is used when printing a deadlock.
 */
typedef const char* (*get_node_label_f) (int);

/**
 * Prints the WFG to a file, e.g. in DOT representation.
 * Used for Debuging and visualization.
 * @param filename the name of the file to store the 
 *        graph representation in.
 * @return WFG_SUCCESS if the call succedes, otherwise WFG_ERROR.
 */
WFG_RETURN wfg_print_wfg (char *filename, get_arc_label_f arc_function, get_node_label_f node_function); 

/**
 * Prints a already detected deadlock to a file, e.g. in DOT 
 * representation. Used for Debuging and visualization.
 * @param filename the name of the file to store the 
 *        graph representation in.
 * @return WFG_SUCCESS if the call succedes, otherwise WFG_ERROR.
 */
WFG_RETURN wfg_print_deadlock (char *filename, get_arc_label_f arc_function, get_node_label_f node_function);

#endif /* WFGLIB_H */

/*EOF*/
