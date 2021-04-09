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
 * @file doxygen_reductions.h
 * Doxygen help page on usage and specification of reductions.
 *
 * @author Tobias Hilbrich
 * @date 21.12.2010
 */

/**
 * @page ReductionUsage Creating Scalable Tools with Reductions
 *
 * @section ReductionOverview Overview
 *
 * Analyses that need global information (i.e. are marked as global) will have a major impact on tool scalability.
 * These analyses will need to receive their input records from all processes. With increasing numbers of application
 * processes the total amount of records will increase, which leads to a higher workload for the communication network of
 * the GTI tool and a higher workload for the analysis itself.
 *
 * In order to remove such scalability limitations it is necessary to avoid any global analyses that receive individual
 * records from all processes. This is possible by using so called "reductions". A reduction is a special analysis
 * which receives records from multiple processes and aggregates them into a new record. It is not just executed
 * on one tool layer but rather on as many layers as possible. Thus, it is able to reduce the total number of records
 * that arrive at a global analysis dramatically.
 *
 * As an example, imagine eight processes are each sending out an integer number and an analysis "sum" wants to determine
 * the sum of these values. Without a reduction each of the numbers would need to be forwarded to the analysis and it would
 * have to calculate the sum itself. With a reduction "r", and a 4 layer layout it is possible to calculate this sum as follows (nodes
 * are used for level places, their labels depicts which analysis/reduction is running on them, and the arc labels show what
 * number is transfered between two places):
 *
@dot digraph WithoutReductions {

     0 [label="App 0", shape=circle, fontsize=10];
     1 [label="App 1", shape=circle, fontsize=10];
     2 [label="App 2", shape=circle, fontsize=10];
     3 [label="App 3", shape=circle, fontsize=10];
     4 [label="App 4", shape=circle, fontsize=10];
     5 [label="App 5", shape=circle, fontsize=10];
     6 [label="App 6", shape=circle, fontsize=10];
     7 [label="App 7", shape=circle, fontsize=10];

     {rank=same; 0 1 2 3 4 5 6 7}
     {rank=same; r1 r2 r3 r4}
     {rank=same; r5 r6}
     0->1 [style=invis];
     1->2 [style=invis];
     3->4 [style=invis];
     4->5 [style=invis];
     5->6 [style=invis];
     6->7 [style=invis];
     r1->r2 [style=invis];
     r2->r3 [style=invis];
     r3->r4 [style=invis];
     r5->r6 [style=invis];

     r1 [label="r", shape=circle, fontsize=10];
     r2 [label="r", shape=circle, fontsize=10];
     r3 [label="r", shape=circle, fontsize=10];
     r4 [label="r", shape=circle, fontsize=10];

     r5 [label="r", shape=circle, fontsize=10];
     r6 [label="r", shape=circle, fontsize=10];

     a [label="a", shape=circle, fontsize=10];

     inv [style=invis];

	 0->r1 [label="5", fontsize=10];
	 1->r1 [label="6", fontsize=10];
	 2->r2 [label="7", fontsize=10];
	 3->r2 [label="8", fontsize=10];
	 4->r3 [label="9", fontsize=10];
	 5->r3 [label="10", fontsize=10];
	 6->r4 [label="11", fontsize=10];
	 7->r4 [label="12", fontsize=10];

	 r1->r5 [label="11", fontsize=10];
	 r2->r5 [label="15", fontsize=10];

	 r3->r6 [label="19", fontsize=10];
	 r4->r6 [label="23", fontsize=10];

	 r5->a [label="26", fontsize=10];
	 r6->a [label="42", fontsize=10];

	 a->inv [label="68", fontsize=10];
     }
@enddot
 * Here each reduction calculates an intermediate sum and forwards it instead of the records used to calculate the sum.
 * This reduces the workload at the root of the tree dramatically and provides a way of implementing global but still
 * scalable analyses.
 *
 * There are requirements that determine whether an analysis can be made scalable by using a reduction. The
 * operation performed by the analysis that is applied to all the trace records must be:
 * - Commutative
 * - Associative
 *
 * In that case it can be distributed throughout the tree and be made scalable with a reduction.
 *
 * @section ReductionWorkflow Workflow for Reductions
 *
 * Reductions use the following workflow to perform their task:
 * - Receive records until all information for the reduction arrived
 * - Decide whether the reduction can succeed or not
 *  - If it succeeds create the reduced record (using a wrapp everywhwere call) and do not forward the original records
 *  - Otherwise fail and forward the original records
 *
 * When a reductions gets the first record for a reduction it is unclear whether the reduction is going to
 * succeed or not, thus, the receival module must buffer the forward of this record. It will buffer
 * the forwarding until the reduction signals whether the redduction was successful -- the forwarding must not be
 * executed -- or failed -- the forwarding must be executed.
 * As a result, the placement driver will not process records from channels that have a pending forwarding,
 * until the reduction finished.
 * This is implemented by blocking the channel (See section on channel ids below) from which the record was
 * received until the reduction finished.
 * All records for this channel will be suspended in the meantime. Further, in a potentially erroneous 
 * environment -- this infrastructure will also be used for debugging purposes -- it is
 * possible that the data needed for a reduction never arrives, in such a case the placement driver may
 * issue a timeout and abort all ongoing reductions.
 *
 * Analyses must explicitly specify that they support certain reductions, this means that they accept the original records
 * as well as the reduced records, which implies that they should have a mapping to the wrapp everywhere function
 * used by the reduction. It can not be guaranteed to an analysis that all reductions succeed, so it may receive a mix
 * of reduced and original records.
 *
 * @section ReductionCreation Creating a Reduction
 * A reduction is created by simply setting the attribute "reduction" to "yes" in the specification of the analysis for the reduction.
 * This has the following consequences:
 * - It may create a reduced representation record that contains information for ALL the api calls to which it is mapped
 *    (except for its wrapp-everywhere call)
 * - The originial record(s) that were used by the reduction will be replaced with the
 *    reduced record
 *  - This can only be done if all analyses that run on places that would receive the original
 *     record specify that they support this reduction
 *  - If there is an analysis not supporting the
 *     reduced record, the weaver will not forward the reduced record as long as this analysis still needs any of the records in the instance
 *  - This decision will be made for each forwarding of the reduction (there is one forwarding for level connected to the level in question,
 *     so level 1 may forward the reduced record to level 2 while forwarding the original records to level 3, becaus some analysis on level
 *     3 won't accept the reduced record)
 * - A reduction must not use two successive records from the same application process
 * - A reduction uses a  wrapp-everywhere call to create the reduced record
 *  - If the reduction is a filter in fact, it may also not have such a function
 *  - This call is called by the reduction itself to create the reduced record
 *  - Analyses that support the reduction should be mapped to this call too
 *  - You may want to put a comment into the analysis description of a reduction to name the wrapp-everywhere call that creates the reduced record
 *  - Beware: if you map a reduction to a second wrapp-everywhere call (one that does not creates the reduced record), it will still not be executed in wrappers
 *  - (non reduction) Analyses that are mapped to a call to which the reduction is mappedl are run in a modified order,
 *     which is that the reduction is run first and all other analyses are run afterwards in the order specified in the api specification for this event
 *  - For analyses that are mapped to calls that may be reduced by a reduction, the following rules apply:
 *   - Analyses that do not support the reduction will be run in receival modules of these calls,
 *      independent of whether the reduction provides a reduced representation of the record or not
 *      Beware: if for some reason such a non-supporting analysis is mapped to the reductions wrapp-everywhere call,
 *      it will be called for the original and the reduced records. However, mapping an analysis to a reductions
 *      wrapp-everywhere call without supporting this reduction is a very questionable design anyways
 *   - Analyses that support the reduction are only run in the receival if the reduction of the record fails; otherwise
 *     the analysis will be called for the reduced record; Rational: without this definition, analyses that need a final reduced
 *     value would have to do the last reduction step themselves
 * - A reduction is only executed in receival modules, otherwise there would be an inifinte recursion
 *    on the wrapp-everywhere call (Reduction is mapped to it and calls it)
 * - A reduction has different return values than regular analyses
 *   (They need to specify whether the reduction is still waiting for further input or whether it succeeded)
 * - Two reductions must not be mapped to the same pre/post event of an API call
 *   (This limitation results from the handling of reductions and how waiting for a reduction to finish is implemented)
 * - The weaver should give helpful output on cases were a reduction can't be used due to an analysis not supporting it,
 *    or if two reductions are mapped to the same call
 * - Must not be global, process-global, or a local integrity!
 * - Beware of dependencies
 *  - Dependencies of a reduction will be placed wherever a reduction is placed
 *  - These dependencies may render the applicability of other reductions impossible or
 *     cause substantial overheads to other levels
 *  - Keep this in mind when adding such dependencies
 * - Must not be placed onto levels in the Layout description, is placed automatically by the weaver
 *
 * An analysis may specify that it supports a certain reduction by providing the "reductions" child node in its XML specification.
 *
 * @section ReductionPlacement Placement
 * Reductions are not placed by the user, but rather by the weaver automatically. 
 * It will try to place all suitable reductions onto levels of
 * the tool and print output if there where useful reductions that could not be 
 * utilized. This may happen if there are analyses not supporting
 * a reduction that needs
 * information from calls that would be reduced by this reduction. 
 * 
 * There are two different concepts at work here, one is the placement of a reduction, the second
 * is the forwarding of the reduced record. Placing a reduction is always possible, though 
 * reductions are only placed if it makes sense, i.e. there is some analysis on the level itself that
 * can benefit from the reduction, or there is some connected level for which the reduction is
 * helpful. Even though some reduction is placed, it may happen that some connected levels
 * do not accept the reduced records. In that case the reduced records will only be forward to 
 * the levels that accept the reduced records. The weaver will give detailed output on which
 * reduction can be used to forward reduced records to what leveles.
 * 
 * So if you use reductions it may be helpful to look at the weaver output.
 * You may also use the verbose mode of the weaver (export GTI_VERBOSE=3) to process
 * and display the graph in the file
 * "weaver-verbose-layout-reduced.dot", which shows you which reductions were placed to what levels.
 *
 * @section ReductionChannelIds ChannelIds and ChannelId Trees
 *
 * A key question for reductions is: "From which processes did I get data and which other processes are connected to me?".
 * The key problem here is that the individual reductions should not have knowledge of the whole communication tree
 * that leads to them, which is expensive for nodes close to the root. Usually (if reductions succeed), a reduction will
 * only receive reduced records from the nodes directly connected to this place and not any original or reduced records
 * from lower levels of the tree. To provide a simple means of information that states for which processes/"parts of the tree" a record
 * (reduced/original) was received, a so called ChannelId is used (gti::I_ChannelId). It is not a global identifier
 * like a rank but rather an identifier that marks a sub-tree or an leaf. The identifier specifies a path to the intended sub-tree
 * or leaf that starts at the current node in the communication network.
 * Example, a reduction might run on node X in the following tree:
 @dot digraph ChannIDExample {
     X [label="X", shape=circle, fontsize=10];

     1 [label="A", shape=circle, fontsize=10];
     2 [label="B", shape=circle, fontsize=10];

     11 [label="C", shape=circle, fontsize=10];
     12 [label="D", shape=circle, fontsize=10];
     13 [label="D", shape=circle, fontsize=10];

     21 [label="E", shape=circle, fontsize=10];
     22 [label="F", shape=circle, fontsize=10];
     23 [label="G", shape=circle, fontsize=10];

     {rank=same; 11 12 13 21 22 23}
     11->12 [style=invis];
     12->13 [style=invis];
     13->21 [style=invis];
     21->22 [style=invis];
     22->23 [style=invis];

     X->1 [label="0", fontsize=10];
     X->2 [label="1", fontsize=10];

     1->11 [label="0", fontsize=10];
     1->12 [label="1", fontsize=10];
     1->13 [label="2", fontsize=10];

     2->21 [label="0", fontsize=10];
     2->22 [label="1", fontsize=10];
     2->23 [label="2", fontsize=10];
}
@enddot
 *
  *There may possibly be more nodes in the overall tree of which this tree is just a sub-tree. Each node has multiple channels
 * from which it may receive records, these channels are used for identification of leafs and subtrees. A channel id is a list of channels
 * indices to use in order to arrive at a certain node or leaf. In the example, the channel id "1.2" (for X) refers to the node connected to X
 * on channel 1 and from this node the node connect to channel 2, which is "G". So a channel id is read from left to right.
 * The special value "-1" is used for "not set" and marks the subtree starting at the current node. E.g. -1.1 would refer to the
 * full subtree starting at X, whereas 0.-1 refers to the whole sub-tree starting at A. In order to have a fixed memory size
 * of all channel ids in the system, there is the gti::I_ChannelId.getNumUsedSubIds function which returns how many
 * sub ids were set for a channel id.
 *
 * Further, each sub-id has not just a channel-index, but also a total number of channels for this sub-id. This is used to determine how many
 * other nodes may exist in the system.
 *
 * Each record in the system has such a channel id which may be used to determine whether information from all processes arrived at a reduction
 * or not. In most situations the reductions running closer to the root will only receive reduced records from their directly connected children,
 * while not receiving any records from farther below in the tree. Thus these nodes will not need to know the full layout of the sub-tree to
 * which they are attached. A ChannelId tree (gti::I_ChannelTree) may be used to calculate whether a record from all connected processes
 * arrived. This is provided in the implementation of gti::CompletionTree. This tree evaluates the channel ids given to a reduction and
 * determines whether the set of given channel ids includes all connected processes.
 *
 * An example on how the gti::CompletionTree works is given below, in the beginning the reduction that runs on node "R" has an empty CompletionTree.
 * If a first record with the channel id 0.1.-1 arrives (having channel sizes of 2.3.2), the tree looks as follows (green are nodes for which a channel id arrived, yellow
 * nodes for which all childs are completed, and red are nodes for which at least one completion is still missing):
 *
 *  @dot digraph ChannTreeExample1 {
     X [label="R", shape=circle, fontsize=10,style=filled,color=red];

     0 [label="0", shape=circle, fontsize=10,style=filled,color=red];
     01 [label="0.1", shape=circle, fontsize=10,style=filled,color=green];

     1 [label="1", shape=circle, fontsize=10,style=invis];
     00 [label="0.0", shape=circle, fontsize=10,style=invis];
     02 [label="0.2", shape=circle, fontsize=10,style=invis];
     010 [label="0.1.0", shape=circle, fontsize=10,style=invis];
     011 [label="0.1.1", shape=circle, fontsize=10,style=invis];

     {rank=same; 00 01 02}
     {rank=same; 0 1}
     00->01 [style=invis];
     01->02 [style=invis];
     0->1 [style=invis];

     X->0 [label="0", fontsize=10];
     X->1 [label="1", fontsize=10];

     0->00 [label="0", fontsize=10];
     0->01 [label="1", fontsize=10];
     0->02 [label="2", fontsize=10];

     01->010 [label="0", fontsize=10];
     01->011 [label="1", fontsize=10];
}
 @enddot
 *
 * If a further record with channel id 0.2.-1 (channel sizes 2.3.2) arrives the tree will be updated as follows:
 *
@dot digraph ChannTreeExample2 {
     X [label="R", shape=circle, fontsize=10,style=filled,color=red];

     0 [label="0", shape=circle, fontsize=10,style=filled,color=red];
     01 [label="0.1", shape=circle, fontsize=10,style=filled,color=green];

     1 [label="1", shape=circle, fontsize=10,style=invis];
     00 [label="0.0", shape=circle, fontsize=10,style=invis];
     02 [label="0.2", shape=circle, fontsize=10,style=filled,color=green];
     010 [label="0.1.0", shape=circle, fontsize=10,style=invis];
     011 [label="0.1.1", shape=circle, fontsize=10,style=invis];
     020 [label="0.2.0", shape=circle, fontsize=10,style=invis];
     021 [label="0.2.1", shape=circle, fontsize=10,style=invis];

     {rank=same; 00 01 02}
     {rank=same; 0 1}
     00->01 [style=invis];
     01->02 [style=invis];
     0->1 [style=invis];

     X->0 [label="0", fontsize=10];
     X->1 [label="1", fontsize=10];

     0->00 [label="0", fontsize=10];
     0->01 [label="1", fontsize=10];
     0->02 [label="2", fontsize=10];

     01->010 [label="0", fontsize=10];
     01->011 [label="1", fontsize=10];

     02->020 [label="0", fontsize=10];
     02->021 [label="1", fontsize=10];
}
@enddot
 * If a record with channel id 0.0.-1 (channel sizes 2.3.3) arrives, the completion tree will be:
 *
 *@dot digraph ChannTreeExample3 {
     X [label="R", shape=circle, fontsize=10,style=filled,color=red];

     0 [label="0", shape=circle, fontsize=10,style=filled,color=yellow];
     01 [label="0.1", shape=circle, fontsize=10,style=filled,color=green];

     1 [label="1", shape=circle, fontsize=10,style=invis];
     00 [label="0.0", shape=circle, fontsize=10,style=filled,color=green];
     02 [label="0.2", shape=circle, fontsize=10,style=filled,color=green];
     000 [label="0.0.0", shape=circle, fontsize=10,style=invis];
     001 [label="0.0.1", shape=circle, fontsize=10,style=invis];
     002 [label="0.0.2", shape=circle, fontsize=10,style=invis];
     010 [label="0.1.0", shape=circle, fontsize=10,style=invis];
     011 [label="0.1.1", shape=circle, fontsize=10,style=invis];
     020 [label="0.2.0", shape=circle, fontsize=10,style=invis];
     021 [label="0.2.1", shape=circle, fontsize=10,style=invis];

     {rank=same; 00 01 02}
     {rank=same; 0 1}
     {rank=same; 010 011 020 021 000 001 002}
     00->01 [style=invis];
     01->02 [style=invis];
     0->1 [style=invis];
     010->011 [style=invis];
     011->020 [style=invis];
     020->021 [style=invis];
     002->010 [style=invis];
     000->001 [style=invis];
     001->002 [style=invis];

     X->0 [label="0", fontsize=10];
     X->1 [label="1", fontsize=10];

     0->00 [label="0", fontsize=10];
     0->01 [label="1", fontsize=10];
     0->02 [label="2", fontsize=10];

     01->010 [label="0", fontsize=10];
     01->011 [label="1", fontsize=10];

     02->020 [label="0", fontsize=10];
     02->021 [label="1", fontsize=10];

     00->000 [label="0", fontsize=10];
     00->001 [label="1", fontsize=10];
     00->002 [label="2", fontsize=10];
}
@enddot
 *
 * Finally, if a record with channel id 1.-1.-1 (sizes 2.3.?) arrives the completion tree will be:
 * @dot digraph ChannTreeExample4 {
     X [label="R", shape=circle, fontsize=10,style=filled,color=yellow];

     0 [label="0", shape=circle, fontsize=10,style=filled,color=yellow];
     01 [label="0.1", shape=circle, fontsize=10,style=filled,color=green];

     1 [label="1", shape=circle, fontsize=10,style=filled, color=green];
     00 [label="0.0", shape=circle, fontsize=10,style=filled,color=green];
     02 [label="0.2", shape=circle, fontsize=10,style=filled,color=green];
     000 [label="0.0.0", shape=circle, fontsize=10,style=invis];
     001 [label="0.0.1", shape=circle, fontsize=10,style=invis];
     002 [label="0.0.2", shape=circle, fontsize=10,style=invis];
     010 [label="0.1.0", shape=circle, fontsize=10,style=invis];
     011 [label="0.1.1", shape=circle, fontsize=10,style=invis];
     020 [label="0.2.0", shape=circle, fontsize=10,style=invis];
     021 [label="0.2.1", shape=circle, fontsize=10,style=invis];

     10 [label="1.0", shape=circle, fontsize=10,style=invis];
     11 [label="1.1", shape=circle, fontsize=10,style=invis];
     12 [label="1.2", shape=circle, fontsize=10,style=invis];

     {rank=same; 00 01 02 10 11 12}
     {rank=same; 0 1}
     {rank=same; 010 011 020 021 000 001 002}
     00->01 [style=invis];
     01->02 [style=invis];
     02->10 [style=invis];
     10->11 [style=invis];
     11->12 [style=invis];

     0->1 [style=invis];
     010->011 [style=invis];
     011->020 [style=invis];
     020->021 [style=invis];
     002->010 [style=invis];
     000->001 [style=invis];
     001->002 [style=invis];

     X->0 [label="0", fontsize=10];
     X->1 [label="1", fontsize=10];

     0->00 [label="0", fontsize=10];
     0->01 [label="1", fontsize=10];
     0->02 [label="2", fontsize=10];

     01->010 [label="0", fontsize=10];
     01->011 [label="1", fontsize=10];

     02->020 [label="0", fontsize=10];
     02->021 [label="1", fontsize=10];

     00->000 [label="0", fontsize=10];
     00->001 [label="1", fontsize=10];
     00->002 [label="2", fontsize=10];
}
@enddot
 *
 * With this, the last record needed to complete the tree arrived, i.e. a record representing each process arrived. As one can see, it is
 * not necessary to gather knowledge about the full communication tree here. As long as reductions do not
 * fail regularly these trees will be small in size. The usage of the completion tree is very simple, use:
 * - gti::CompletionTree.addCompletion to add a channel id as completed to the tree (will automatically add nodes to the tree if necessary)
 * - gti::CompletionTree.isCompleted to query the tree on whether it is complete or not (in the example this function would return true for the last tree while returning false for all the other trees)
 *
 * @todo Future additions to the completion tree must provide functionality to set filters that modify the
 *            behavior of the tree.
 *
 * @section ReductionImplementation Implementing a Reduction
 *
 * All reduction analyses must implement the interface gti::I_Reduction. This interface requires you to implement the function
 * gti::I_Reduction.timeout which is used to notify a reduction about a timeout. First a reduction without the handling for
 * timeouts will be presented, while the additions for timeouts will be added in the next section.
 *
 * Analysis functions of reductions get two extra parameters, which are:
 * - thisChannel : I_ChannelId* -- Channel id of the record.
 * - outFinishedChannels : std::list<I_ChannelId*>* -- If the reduction returns SUCCESS or IRREDUCIBLE, all OTHER channel ids that were reduced (besides the one given to this analysis call) need to be added to this list. The channel ids added to the list will be freed by the caller of the analysis function and must not be used by the reduction after the analysis call ended.
 *
 * The "thisChannel" argument tells the reduction the channelId of the new record, whereas the "outFinishedChannels" argument
 * is used to return a list of reopened channels to the receival module calling the reduction.
 *
 * Each reduction uses the following return values:
 * - GTI_ANALYSIS_SUCCESS -- Returned when a reduction was successful and replaced the input records
 * - GTI_ANALYSIS_FAILURE -- Returned for critical errors that should lead to an abort of the tool and application
 * - GTI_ANALYSIS_WAITING -- Returned if the reduction needs further input to perform the reduction
 * - GTI_ANALYSIS_IRREDUCIBLE -- Returned if all input for a reduction arrived, but it is not suitable for reduction, e.g. due to invalid data in the records. Or if the reduction had a timeout to handle records that belong to an aborted reduction.
 *
 * When returning GTI_ANALYSIS_SUCCESS, it is necessary to add all channelIds (besided the one that was given to the
 * analysis function) that were previously suspended with a return value of GTI_ANALYSIS_WAITING to the list in the
 * outFinishedChannels argument. The reduction needs to add all channelIds of records that participate in the completed
 * reduction only. The memory for the channelIds added to the list will be freed by the caller of the reduction and must
 * not be touched after adding them to the list. The memory for the channel id given in thisChannel must be freed by
 * the reduction and will not be touched by the caller after the analysis returns. (One will usually free the "thisChannel"
 * id by adding it to the list for outFinishedChannels at some point)
 *
 * @section ReductionTimeouts Timeouts
 *
 * Usually reductions will wait for "waves" of input records, i.e. one record from each process connected to the reduction.
 * If for some reasons -- a semantic error or an application crash -- one of the waited for records never arrives, it may
 * be necessary to abort reductions in order to unblock suspended channels, which is needed to allow further progress
 * in the processing of the available records. In such a situation the placement driver notifies the recieval module to
 * abort all reductions, who in turn notifies each reduction of the timeout. In addition, the receival module
 * executes all pending forwards (pending due to
 * ongoing reductions). The reductions are notified by using the gti::I_Reduction.timeout function implemented by each reduction.
 *
 * The implementation of the timeout function should do the following:
 * - Free all stored up channel ids for suspended channels
 * - Start a new completion tree
 * - Store the old completion tree in order to determine whether future records are from a 
 *    new reduction or were from the timed out reduction.
 *
 * As it may happen that the missing records for the reduction arrive at a later point in time, reductions must differentiate
 * whether a record belongs to a timed out reduction or a new one. This is possible by storing the current state of the
 * completion tree and using the gti::CompletionTree.wasCompleted function to query whether the record was already
 * completed in a timed-out reduction or not. If the record belonged to the timed out reduction the analysis function
 * must return GTI_ANALYSIS_IRREDUCIBLE and must not store the channel id. Otherwise, the reduction can return
 * GTI_ANALYSIS_WAITING or GTI_ANALYSIS_SUCCESS as usual.
 *
 * Keep in mind that multiple successive timeouts may occur, in that case it is necessary to store a list of the aborted
 * CompletionTrees to determine whether the record belongs into any of these reductions or whether it belongs
 * to a new -- and possibly successful -- reduction.
 *
 * @section ReductionExample Example
 *
 * In the folder "tests/reduction" there is a full example of a GTI based tool with a simple reduction. The example
 * intercepts MPI_Init and MPI_Finalize, the first call is used to generate a floating point number, which is the rank
 * of the process, using an operation. The ranks will be summed up by the reduction gti::SumFloatReduction and
 * printed by the analysis module gti::PrintFloatSum.
 *
 * The interface for the
 * reduction that is passed to the generated receival module looks as follows:
 * @include SumFloatReduction.h
 * It basically inherits from gti::I_Module and gti::I_Reduction while adding the analysis function for the reduction,
 * which is called "reduce". This function has the floating point number to be summed up as the first argument and the extra
 * arguments that analysis functions of a reduction must accept according to gti::I_Reduction.
 *
 * The implementation of the reduction is given below:
 * @include SumFloatReduction.cpp
 * It handles successful reductions as well as potential timeouts and can be considered as a reference implementation
 * for handling timeouts. One may put the handling for timeouts into a base class to avoid the need to handle
 * this for each reduction.
 *
 * The analysis specification for the two analyses and the operation to return the rank in MPI_Init are given in the specification
 * below:
 * @verbinclude analysis-spec.xml.in
 *
 * The API specification for the example includes the two MPI calls and the wrapp-everywhere call that is used by the
 * reduction:
 * @verbinclude api-spec.xml.in
 *
 * Finally, a simple layout for the example is given below:
 * @verbinclude layout-spec.xml
 * This layout uses 8 application processes and 3 layers of tool processes with 4, 2, and 1 process(es).
 *
 * The output of the application should include:
 * @verbatim
PrintFloatSum (f=28)
 @endverbatim
 *
 * which is the final sum computed on the root process, which is afterwards passed to the print module.
 *
 * @section TimeoutsChannelIDs Non-Reductions Receiving Channel Ids
 * Usually, if no timeout occurs and the reduction data is consistent and correct, analyses that
 * support reductions will only receive reduced representations. However, if errors or timeouts
 * occur, analyses will receive partial inputs that are not yet completly reduced. Depending on
 * the analysis, it may be necessary to compute a final reduced representation -- or to receive
 * some input from all or a set of processes -- before the analysis can do its computation. For
 * these analysis there is a need to get information on which processes are part of a received
 * record. To support this, an analysis function specification of any analysis may specify that it
 * needs a channel id as an extra input. (Analysis specification, add the attribute "needs-channel-id"
 * with a value of "yes" in the "analysis-function" node of the respective function) With this
 * channel id an analysis can query from which processes the record is, i.e. whether it is a partial
 * input or a completely reduced record. The following rules apply:
 * - The channel id must not be added to the list of arguments of the function
 * - The extra argument is added as the last argument
 * - The type of the argument is "I_ChannelId*"
 * - The channel Id is only valid until the analysis function returns
 * - It must not be freed by the analysis, as it will be freed by the caller at some point
 * - If the analysis needs the channel id for a longer time, it must create a copy for storage
 * - If the record was created on this place itself, then there is no channel id, in that case
 *    a NULL pointer will be given as argument
 * - With that, for analyses supporting reductions a NULL pointer as channel id means that the
 *    record was reduced on this place, a non-NULL value means that this record was not
 *    reduced on this level; The channel id will give information on where the record comes from
 *    in that case.
 *
 * The example already uses this mechanism for demonstration purposes in the PrintFloatSum
 * module.
 *
 * @section ReductionDeveloper GTI Developer Information Dump
 * The following list of modifications was applied to GTI in order to implement reductions (not necessarily all up to date):
 * @verbatim
GTI Extension for reduction operations on trace records

2) An analysis must have an option to be a "reduction"
-> Enhance DTDs
-> Enhance Weaver
-> (It should somehow point to the wrapp-everywhere call it uses to create the reduced trace record)
-> It creates only one single reduced record, not multiple !
     (Split a reduction if it creates multiple different records)
-> It must be mapped to all API calls events (pre/post) for which it generates a reduced record
-> Special rule: Reductions are only executed during receivals and never in wrappers
     (Otherwise infinite recursion in the wrapp-everywhere call)
-> Rule: Two reductions must not be mapped to the same event (pre/post) of an API call
    --> The Weaver should warn in such a case and drop any one of the two reductions
    --> Background:
              With this limitation a channel (see 5) will wait for at most one reduction, this drastically simplifies the implementation
              of the logic that is used to determine when to use a reduced trace record instead of the original record. Also see To 5)
              for details on this.

3) Return values of analyses/reductions
-> Return value given when calling an analysis function
-> Analyses return: SUCCESS or FAILURE (the later one causes critical abort)
-> Reductions return: SUCCESS, IRREDUCIBLE, WAITING, or FAILURE (the last one causes critical abort)
     --> SUCCESS, or IRREDUCIBLE are returned when all input for the reduction is present (e.g. a collective from all processes or a send and a receive call)
           Use SUCCESS if a reduction is possible and IRREDUCIBLE if the data is not suited for the reduction, e.g. due to inconsistencies or an error in the input data
     --> WAITING is returned if the reduction is still waiting for further input data
           (Even if it is clear that the data is irreducible, a reduction should return WAITING until all data that is needed arrived, as otherwise the consistency
            (Epoch) for the reduction might be lost)

4) An analysis module must be allowed to specify that it accepts certain reductions
-> If the list of accepted reductions is r_1,...,r_n
     Then the analysis function may be run on an original record for a certain event (pre/post of an API call) or on a reduced record created by
     any r_i (i in 1...n) for this event. Note that there will always be at most one reduced record that is used instead, as no two reductions may
     run on a single event.
-> It should be mapped to ALL of the wrapp-everywhere functions used by r_1...r_n (If not it can't really be accepting the respective reduction ...)

5alpha) Extension of records being generated to include a "channel id"
-> Weaver must determine the size and format of the channel id
-----> A channel id consists of multiple sub-ids, the width (in bits) of these ids needs to be determined, the total amount of sub-ids needs to be determined
-----> sub id width=ceil(max(branching)+1)
-----> number of sub ids=max(order) (of all levels)
-----> When receiving a record, the receiving level writes the id of the channel it received the record from into the sub id with index level.order
---------> Thus the channel id uniquely defines from which process in the application the record came from
-----> As writing and reading something in a record is impossible to the places, it uses a function of the receival record for that
-> The weaver must allocate a enough bytes in each record for this channel id and name it with a predefined name. e.g. "GtiChannelId"
-> This needs to be reflected in the inputs to the wrapper and receival generators
-> The value 0 is reserved in the sub-ids, it represents an invalid value not being set, this may be used to distinguish reduced records from regular ones
     --> As a result when channel id x should be written, write x+1 instead, do the inverse when reading

5beta) Extension of Receival to provide a "updateChannelId Function"
-> Input: buffer, uid, order, channelIdReceivedFrom  ((Order really needed or is it already known to the receival generator?))
-> Output: newChannelId
-> Takes a record and writes the given channel id (see 5alpha) the channel from which the place received the record)
-> It returns the complete channel id after the update

5gamma) Extend I_CStratDown to return the channel
-> wait and test must return channel!

5zeta) Correct Channel ID usage
-> Now distance of layer received from to app layer is used as index when writing the sub-ids
-> New interface function in I_ChannelID to query for number of sub-ids being used

5) Extension of receival to postpone a forward
-> If a record is given to the receival, do the following:
    a) pass the record to all the analyses and -- if used -- the reduction that needs it
    b) If the reduction returns WAITING
        --> store the record along with its state in a list of pending forwards (for this channel)
        --> Return to the placement driver and tell it that the records cannel ID(!) is now in the waiting state
    c) If the reduction returns a SUCCESS or IRREDUCIBLE
        --> Query the reduction for the list of OTHER channels ids that have been in waiting state
        --> Loop over these channels ids:
        ------> trigger the execution of the forwarding for this channel id (record and state is in list of pending forwards) (see d) on how to handle this)
        ------> Store this channel id in a list of reactivated channels (needs to be returned to placement driver)
    d) If the reduction returned SUCCESS
       --> Then the reduction already created its reduced record
       --> For all forwards that use the reduced record the forwarding took already place in the wrapper for the wrapp-everywhere function of the reduction
       --> Loop over all forwards that are not happy with the reduced record and forward them the original record
    e) If the reduction retunes IRREDUCIBLE
       --> Loop over all forwards and forward them the original record
    f) If the reduction originally returned a SUCCESS or IRREDUCIBLE
       --> Return the list of reactivated channel ids (see c)) to the placement driver

6) Extension of the placement driver to support suspension of channel ids
-> One queue for each channel id needed
-> Plus an extra queue for each complete channel
-> The queues will store received records that can currently not be processed due to a channel/channel id being suspended to finish a reduction
-> Use the return values from receival to set queues as suspended or remove their suspension
-> If a new record R is received from channel C:
      --> If: C is not suspended
            Then:
                      If (channelId(R) is not suspended)
                      Then:
                            call receival for record
                            suspend/activate based on return
                      Else
                            enqueue in queue for channelId(R)
            ELSE
                      Enqueue R in queue for C
-> Run over queues until all records are processed or in queues that are from suspended channels
     --> If no more records can be processed: wait for a new record from the input comm. strategy
-> When processing records after a queue was removed from suspension
----> start with all records enqueued for channelIDs of a reacitvated Channel
----> Afterwards go on with records enqueued for the channel itself

7) Place reductions automatically
-> Reduction r is placed on level i if and only if:
     Some descendant level of i accepts r and
     For all events e to which r is mapped holds:
            For all descendants(i): every analysis(not reduction) mapped to e accepts r
-> Place reductions BEFORE calculating the needed arguments!
-> For all levels i:
     1) Loop over all out channels (leading to place j)
     -> Get all reductions accepted at j and descendants (j) -> R
     -> For all r in R:
           --> If r is already placed on i, continue
           --> Get list of events E to which r is maps (call, order)
           --> rAcceptable = true
           --> For all e in E:
                 ---> Get all analyses run on j and descendants(j) A
                 ---> For all a in A:
                         ----> If a is mapped to e:
                                  ----> If a not accepts r
                                           ------> rAcceptable = false
           --> If rAcceptable
                 ---> Place r on i
-> Calculate the needed arguments as usual (If aggregation fails the original record is still needed, so no simplification yet)
-> Todo: may it happen that we run too many reductions somewhow ? (My guess is no, due to the one reduction per event restriction)

10) Weaver must provide the right forwarding for reductions
-> Extend wrapper and receival generator input to support reductions
-> Wrapper:
     --> Do not run reductions in wrapper modules
     --> Do not run analyses supporting reduction r in the wrapp-everywhere call used to generate the reduced record by r
           (Otherwise this analysis would once receive the unreduced record and the reduced record afterwards)
     --> The forwarding needs not to be changed and can use the regular calculated call args
     => This can easily be done as a modification in the weaver
-> Receival:
     --> If an event (call, order) has a reduction:
           --> Mark the reduction as a reduction (so the generator knows that it has to check its return value)
           --> Mark to which of the forwards the reduction is applicable!
                 (The generator only needs to handle the reduction for these forwards, for the other ones it can just forward
                  as always)
           --> Generator has to generate the right code to suspend the forward based on the reduction return value
           --> Still use the full calculated call args, no modification needed here

11alpha) Implement the test case and get it running :D

11) Timeout
-> In a possibly incorrect environment it may happen that a reduction never finishes
-> Add a timeout to the placement driver that tracks how long a certain queue was suspended
-> If the timeout exceeded the trigger value, a virtual abort of the reduction is done
     --> This means that the forwards that where suspended take place, and the suspension of the queue is removed
     --> However, the reduction must still be tracked in order to determine when it is finished and when the next reduction may be started
     --> See first element in the "Important" section
     --> The functionality for IRREDUCIBLE might very well be used for that -> basically one would forward the dependent records and force the reduction to return IRREDUCIBLE without aborting and reseting it (note that one must not perform the wake inactive channels procedure when the reduction finally returns its IRREDUCIBLE as this was already done as part of the timeout actions)

13) Reductions and Dependencies (to other analyses)
-> Do we actually add dependencies correctly, i.e. can a module get its dependent modules ?
-> Reductions should also be allowed to have dependencies
---> Under which conditions ?
---> Issue here: we must not add modules when loading a reduction that renders this or other reductions inapplicable!
 @endverbatim
 
 * @subsection ReductionDeveloperReductionChange1 First modification to reductions
 * ==================
Reduction Modification
==================

Issue
-------------------------------
Assume three layers as below:
L0->L1->L2

Further, assume analysis a is mapped to L1 and supports reduction r that is also mapped to L1.

In this case a is mapped to the wrapp-everywhere call c used by the reduction, otherwise it couldn't really support the reduction.

If a reduced record arrives (an event for c), a will be called in the receival module. Now, if the reduction is successfull and calls the wrapper for c, then a will be called again in that wrapper. Thus, it receives the partial pieces of a reduction and the reduction result. What should be intended is that a only receives the reduction result if the reduction is successful. 

Issue 2
-------------------------------
In the scenario above, if a is mapped onto L2. In that case r would never be mapped to L2, as there is no connected process needing the reduced record, so a will only be able to receive the partial reduced inputs, and would have to perform the last reduction step itself. As a result, a would need to receive information about the channel ids to do so. 

Extra issue: getting rid of the requirement to need the channel ids in analyses that support reductions would be nice, though, due to the timeouts they will be required anyways. So performing the last step reduction manually in the analysis appears to be something that can't be avoided pretty much ...

Solutions
-------------------------------

1) Receiving channel ids in analyses
---
- Add attribute "needs-channel-id" to analysis specification, is added to analysis-function nodes
- Read the attribute in the weaver and store it as property of analysis functions
- Pass this property into the input xmls of the wrapper and receival generators
- Definition: this adds the extra argument "gti::I_ChannelId *channel"
- For wrappers it is set to NULL (they can't create a channel id, it represents a record from this place, possibly (if from a wrapp everywhere call of a reduction, a completly reduced record)) !
- For receival it is the recordChannelId value given to the ReceiveRecord interface and will be deleted after the analysis function returns
- add the placement of the channel id to the receival and wrapper generators

2) Fixing Issues 1 +  2 
---
- Rework reduction placement to also place them on layers that run a supporting analysis
-- Weaver!
- For a calls to which a reduction is mapped:
-- In Receival: 
--- Run supporting reductions in executeForward (uid, pRecordIn, avoidReducibleForwards) 
--- Only run them if avoidReducibleForwards==false 
   (Reduction was not possible -> irreducible oder timeout)
--- Non-supporting analyses will be run independently of whether the reduction succeeds or not, but the order will stay the same for this call
=> Add special rule for execution order of analyses for this case it will be: 1) reduction 2) UsualOrder (other analyses)
-- In Wrapper:
--- Nothing to do, check whether there was some special rule disabling something there ...

Filters:
- Not filtered out
-- Return is IRREDUCIBLE
--> analyses are run in executeForward
- Filtered out
-- Return is SUCCESS
-- But no wrapp everywhere call is called
--> analyses are not run at all
--> this is a new behavior, and should be added to the missing Filter example in GTI

3) Documentation updaten
----
!!!!!
 * 
 *
 *
 */

