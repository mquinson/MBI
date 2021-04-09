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
 * @page MustBaseAnalyses MUST Basic Analyses and Operations
 *
 * MUST provides default analyses and operations for handling MPI, providing information on rank and
 * location information, and for creating messages that are logged.
 * This is covered in the following sections:
 * - @ref MUSTBase_mpi_base
 * - @ref MUSTBase_locations
 * - @ref MUSTBase_logging
 *
 * @section MUSTBase_mpi_base Basic operations for MPI
 *
 * MUST provides basic operations for MPI functions that are
 * used to compute array length for MPI calls, as well as to
 * convert MPI handles to integers, which can be stored in
 * records. Note that MPI handles can't be put into records
 * as their internals are unknown and may contain pointers.
 * These operations are specified in:
 * @see mpi_base_specification.xml
 *
 * @section MUSTBase_locations Source locations and parallel Ids
 *
 * @see must_base_specification.xml
 * @see must_base_api_spec.xml
 *
 * Most analyses will need a parallel id (e.g. rank, thread, accelerator, ...) as
 * well as a source code location to perform their tasks while also providing reasonably detailed
 * error descriptions. MUST provides operations
 * and analyses to provide this functionality. It has two datatypes for this information:
 * @image html MUSTBaseOps_Types.png "Data types for process/thread/... id and source code location."
 *
 * A MustParallelId is used to refer to a parallel id, a parallel id analysis is used to retrieve
 * detailed information like rank/thread from this id (I_ParallelIdAnalysis).
 * The id storage is provided by an operation (an array operation that either returns access to the
 * value of the id or a pointer that allows a modification of the id).
 * An integrity analysis (runs first!) then sets this value.
 * The operation provides the set value to all analyses that need this information.
 *
 * MustLocationId is also an uint64_t id that -- along with a parallelId -- can be
 * used to retrieve the call name, source file name, and source line, ... (depending on selected implementation)
 * for a location. The id storage is provided in wrappers on the application processes
 * by the operation "provideLocationIdStorage".
 * An automagic analysis that is added as integrity then fills the provided storage with an actual value and
 * provides a mapping of these id's to information on these id's (call name and the like).
 * Any analysis module can then map this id to the actual pieces of location
 * information with the Location analysis (I_LocationAnalysis). The idea behind this is to avoid sending all these
 * individual pieces of information as part of every trace record, as the strings for source name
 * and call name have a significant length. However, to enable the mapping of the locationId to
 * the actual pieces of information it is necessary to provide this information on ALL tool places.
 * This is achieved with the following design:
 *
 * @image html MUSTBaseOps_Dependency.png "Dependencies for MUST MPI checking modules and the Location analysis."
 *
 * ALL checks and modules that need location information have a dependency to the
 * location analysis (The module is registered as "LocationAnalysis" in group "MUST_Base").
 * It stores the necessary mapping information and can be queried to
 * return detail information for a location id. The mapping of the Location analysis is refreshed with the
 * following design:
 *
 * @image html MUSTBaseOps_LocationOnApplication.png "Sequence diagram for handling a new source location on the application."
 *
 * The analysis "InitLocationId" checks whether the location is already known, if not the
 * call "handleNewLocation" is issued with the respective location information. An API
 * specification for this call maps the location analysis to it, such that the location
 * analysis gets the information for the new mapping, as all places that use a check
 * will also run the Location module (due to the dependency of the modules) the record
 * for the information will be forwarded to all other places as well.
 * The behavior of extra places is shown in the following sequence diagram:
 *
 * @image html MUSTBaseOps_LocationOnPlace.png "Sequence diagram for handling a new source location on an extra place."
 *
 * The implementation of the InitLocationId analysis tries
 * to assign equal ids to locations that are equal on all processes. As this is done
 * without inter process communication it will only work for certain cases at a certain
 * success rate. This highly depends on the amount of information that is mapped
 * to MustLocationId.
 *
 * With this design, location Ids that arrive at tool places were the id is already mapped
 * to the right information are redundant. A location reduction is used to remove these
 * redundant location ids from the network. This reduction is just a filter and determines
 * whether the id was already known with the given detailed information, if so it just
 * drops the record. The reduction is implemented in I_LocationReduction.
 *
 * @section MUSTBase_logging Logging of events
 *
 * @see must_base_specification.xml
 *
 * Forwarding and processing of log events is provided by the MUST base analyses.
 * These use the following active components:
 *
 * @image html MUSTLogging_Overview.png "Logging components (This is not a class diagram!)."
 *
 * The components are:
 * - CreateMessage: Analysis (I_CreateMessage registered as "CreateMessage") to which each analysis -- that may create a message to log -- has a dependency.
 *                 It takes the details for the message to log and calls the wrapp-everywhere function "handleNewLog".
 * - handleNewLog: An API call to which all implementations of I_MessageLogger are mapped.
 *                   With that design the log events will be
 *                   forwarded to the I_MessageLogger implementation automatically.
 *                   Important: to enable logging on tool places, a wrapper for this
 *                              call needs to be generated on tool places too. This
 *                              is done by marking this call as
 *                              "wrapped on all places" (wrapp-everywhere).
 * - I_MessageLogger: Takes log events and logs them with whatever the implementation
 *                 of the processor defines. It is mapped to the "handleNewLog" API
 *                 call (e.g. MUST::MsgLoggerStdOut, MUST::MsgLoggerHtml).
 *
 * Below is a list of sequence diagrams that show how these components interact with each
 * other:
 *
 * @image html MUSTLogging_ApplicationNewLog.png "Sequence diagram for creation and handling of a new log event on the application."
 * @image html MUSTLogging_PlaceNewLog.png "Sequence diagram for creation and handling of a new log event on a tool place."
 * @image html MUSTLogging_ReceiveAndForwardLog.png "Sequence diagram for receiving a log record and forwarding it."
 * @image html MUSTLogging_ApplicationNewLogProcessed.png "Sequence diagram for creating and processing a log record on the application."
 * @image html MUSTLogging_ReceiveAndProcessLog.png "Sequence diagram for receiving and processing a log record."
 */
