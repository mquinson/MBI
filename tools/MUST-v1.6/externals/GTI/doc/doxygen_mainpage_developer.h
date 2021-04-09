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
 * @mainpage GTI
 *
 * @todo Finish the main page (doxygen_mainpage_developer.h)
 * @todo Put Institution logos on that page!
 *
 * This page is still under development and is currently just used as a list of sections.
 *
 * Sections:
 * - @subpage MustRead Introduction and essential information on GTI
 * - @subpage RecordSpecifics Information on trace records in GTI
 * - @subpage Instantiation Information on how to instantiate GTI
 * - @subpage GenericToolInfrastructure Information on the Generic Tool Infrastructure
 *
 * In order to make Doxygen happy we need to include the GTI/MUST logo from the header somewhere,
 * done:
 * @image html must-logo.gif "MUST Logo"
 */

/**
 * @namespace gti
 * The main namespace used for GTI.
 */

/**
 * @page MustRead Must Read
 * Lists essential information on developing and using GTI.
 *
 * @section MustReadUsing Using GTI
 * Nothing yet.
 *
 * @section MustReadDeveloping Developing GTI
 * - @subpage CodingStyle The GTI coding style, read it before writing code!
 * - @subpage CTest Information on using CTest and the ZIH Dart server
 */

/**
 * @page RecordSpecifics Trace Record Related Information
 * Information on trace record related information.
 * - @subpage RecordGenerator Details on the Trace Record Generator Interface
 * - @subpage TraceCutter Details on the Trace Cutter
 * - @subpage MUSTRecordImplementation An implementation of the record generation interface
 */

/**
 * @page Instantiation How to Instantiate GTI
 * Describes how to use GTI modules to instantiate a tool.
 * - @subpage ModConfPage how modules are created, freed, and configured
 * - @subpage InstantiationProcess Describes how to create an instance of a GTI based tool
 * - @subpage WeaverUsage Using the weaver to create GTI tools
 * - @subpage ReductionUsage Creating reductions to implement scalable tools
 *
 * Developer stuff:
 * - @subpage WeaverImplementation Details on the weaver implementation
 *
 * Older stuff:
 * - @subpage ProofOfConcept Details on the initial proof of concept implementation we had
 */
