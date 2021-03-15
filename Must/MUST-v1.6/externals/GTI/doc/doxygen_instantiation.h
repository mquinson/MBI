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
 * @page InstantiationProcess The GTI Instantiation Process
 *
 * The individual modules of GTI and P^nMPI need to be combined
 * in order to create a working instance of the GTI tool framework. Besides
 * these pre-built modules there is a need for further intermediate modules
 * that are used to glue modules together, to implement wrappers, to handle
 * the receival and forwarding of trace data, to trigger analyzes from trace
 * data, and so on.
 *
 * This section describes the overall process that is involved in
 * instantiating GTI. An instance is constructed from a given executable,
 * the pre-built modules, the intermediate modules, and a P^nMPI module
 * configuration. The later two are constructed by the so called "System
 * Builder" component. This component uses various specifications as input.
 * The following chart shows this from a high level perspective:
 @dot digraph InstantiationHigh {

     InSpec [label="Input Specifications", shape=box, fontsize=10];

     SysBuilder [label="System Builder", shape=box, fontsize=10];

	 IntMod [label="Intermediate Modules", shape=box, style=rounded, fontsize=10];
	 ModConf [label="Module Conf. for\nP^nMPI/Dynasty", shape=box, fontsize=10];

	 InSpec->SysBuilder [label="", weight=1, fontsize=10];
	 SysBuilder->IntMod [label="", weight=1, fontsize=10];
	 SysBuilder->ModConf [label="", weight=1, fontsize=10];

	 subgraph cluster3 {
		 color=black;
         style=invis;
         label="Module Library";
         TempNode [label="Module Library", shape=box, style=rounded, fontsize=10]
		 }

	 ApplicationExe [label="Application\nExecutable", shape=box, fontsize=10];
	 GTIInstance [label="GTI\nInstance", shape=box, fontsize=10];

	 IntMod->GTIInstance [label="", weight=1];
	 TempNode->GTIInstance [label="", weight=1];
	 ModConf->GTIInstance [label="", weight=1];
	 ApplicationExe->GTIInstance [label="", weight=1];
 }
 \enddot
 * Figure 1: High level perspective for instantiation.
 *
 * The input specifications describe the paradigms/API's of interest
 * -- e.g. MPI, OpenMP, ... --, the tool modules and the module layout.
 * The individual specifications are:
 * - GTI specification:
 *  - describes the individual modules of the generic tool infrastructure
 *    (communication modules and other infrastructure modules).
 * - Analysis specifications:
 *  - What analyses exist
 *  - What inputs do these analyses need
 *  - Inter analysis dependencies
 *  - Data transformations executed in the wrapper (e.g. MPI_Request -> int)
 * - API specifications:
 *  - What API calls are in an API
 *  - What are their arguments
 *  - What analyses need what data of these calls
 * - Layout specification:
 *  - What layers exist
 *  - How many places in each layer
 *  - What analyses run on each layer
 *  - What implementations of comm. strategies/protocols to use
 *
 * The individual specifications are shown in Section @ref DTDSpecs.
 * They are designed in the document type definition syntax.
 *
 * Each of these specifications is provided by a different actor.
 * The chart below shows who will create what type of input:
 *
 * @image html MUSTGeneration.png "Figure 2: The input 'Who is who'"
 * @image latex MUSTGeneration.png "The input 'Who is who'" width=13cm
 *
 * So the user of a GTI tool will either create a specifically
 * tailored layout for the tool, or uses a default configuration.
 * Such a default configuration will likely use all types of analyses
 * and a basic distribution of the analyses.
 * Default configurations can be adapted to the needs of a certain
 * computing system by the administrator who installs GTI.
 * The other specifications are created by experts that have knowledge
 * of the API/paradigm of interest, or by developers of GTI/Tool modules.
 *
 * With this information, the system builder component of GTI is
 * able do decide what
 * trace information is needed at what tool places,
 * what calls need to be wrapped,
 * which analyzes have to be executed if a certain place receives a
 * trace record, and what data needs to be forwarded to certain tool
 *  places.
 * Two different types of modules are used to implement these activities:
 * - The "Wrapper Module" wraps all calls that trigger any of the selected
 *   analyses. It executes analyses that need to be directly executed
 *   on the
 *   application processes and builds trace records for data forwarding.
 * - The "Receival/Forward Modules" (one for each type of place)
 *   take a trace record that was received from the communication
 *   network and
 *   forward its data to all analyses that need this data. Afterwards,
 *   they prepare the trace record for a communication to further places,
 *   as part of the preparation a trace record may be reduced in size,
 *   in order to remove superfluous data.
 *
 * Each of these modules is build by a distinct generator, which takes
 * an XML specification as input. These input XMLs are built by
 * the Weaver component. The specifications for these XMLs are given
 * in Section @ref GenDTDSpecs.
 *
 * The chart below highlights this process:
 *  @dot digraph InstantiationDetailsLegend {
	 subgraph cluster1 {
		 EXE [label="EXE", shape=circle, fillcolor=orange, style=filled, fontsize=10];
		 XML [label="XML", shape=circle, fillcolor=grey, style=filled, fontsize=10];
		 MOD [label="MOD", shape=circle, fillcolor=lightblue, style=filled, fontsize=10];
		 TXT [label="TXT", shape=circle, fillcolor=chartreuse, style=filled, fontsize=10];

		 color=black;
         style=rounded;
         label="Legend";
		 }
    }
    @enddot
 @dot digraph InstantiationDetails {
     GUI [label="Layout GUI", shape=box, fillcolor=orange, style=filled, fontsize=10];
     AnalysisSpec [label="{Analysis Spec. 1\ne.g. Correctness| ... |Analysis Spec. N}", shape=record, fillcolor=grey, style=filled, fontsize=10];
     LayoutSpec [label="Layout Spec.", shape=box, fillcolor=grey, style=filled, fontsize=10];

	 APISpec1 [label="{API Spec. 1\n (e.g. MPI)| ... | API Spec. N}", shape=record, fillcolor=grey, style=filled, fontsize=10];
     GTISpec [label="GTI Spec.", shape=box, fillcolor=grey, style=filled, fontsize=10];

     { rank = same; "APISpec1"; "GTISpec"; "LayoutSpec"; "AnalysisSpec"; }
     { rank = source; "GUI"; }

     subgraph cluster4 {
         color=black;
         style=rounded;
         label="System Builder";

		 SysBuilder [label="Weaver", shape=box, fillcolor=orange, style=filled, fontsize=8];

		 GUI->LayoutSpec [label="", weight=1, fontsize=8];

		 APISpec1->GUI [label="", weight=0, style=dashed, fontsize=8];
		 GTISpec->GUI [label="", weight=0, style=dashed, fontsize=8];
		 AnalysisSpec->GUI [label="", weight=0, style=dashed, fontsize=8];

		 LayoutSpec->SysBuilder [label="", weight=1, fontsize=8];
		 APISpec1->SysBuilder [label="", weight=1, fontsize=8];
		 GTISpec->SysBuilder [label="", weight=1, fontsize=8];
		 AnalysisSpec->SysBuilder [label="", weight=1, fontsize=8];

		 WrappGenIn [label="Wrapper Generator\n Input", shape=box, fillcolor=grey, style=filled, fontsize=8];
		 ReceivalGenIn [label="{Receival/Forward Module\n Generator Input | ... | N}", shape=record, fillcolor=grey, style=filled, fontsize=8];

		 SysBuilder->WrappGenIn [label="", weight=1];
		 SysBuilder->ReceivalGenIn [label="", weight=1];

		 WrappGen [label="Wrapper Generator", shape=box, fillcolor=orange, style=filled, fontsize=8];
		 ReceivalGen [label="Receival/Forward \n Module Generator", shape=box, fillcolor=orange, style=filled, fontsize=8];

         ModConfGen [label="P^nMPI/Dynasty\nConfiguration\nGenerator", shape=box, fillcolor=orange, style=filled, fontsize=8];
         ModConfInput [label="P^nMPI/Dynasty\nConfiguration\nInput", shape=box, fillcolor=grey, style=filled, fontsize=8];

		 SysBuilder->ModConfInput [label="", weight=1];
		 ModConfInput->ModConfGen [label="", weight=1];

		 BuildInput [label="Build File\nInput", shape=box, fillcolor=grey, style=filled, fontsize=8];
		 BuildGen [label="Build File\nGenerator", shape=box, fillcolor=orange, style=filled, fontsize=8];
		 BuildFile [label="Build File\n(CMake)", shape=box, fillcolor=grey, style=filled, fontsize=8];
		 SysBuilder->BuildInput [label="", weight=1];
		 BuildInput->BuildGen [label="", weight=1];
		 BuildGen->BuildFile [label="", weight=1];

		 WrappGenIn->WrappGen [label="", weight=1];
		 ReceivalGenIn->ReceivalGen [label="", weight=1];
		 }

	 ModConf [label="Module Conf. for\nP^nMPI/Dynasty", shape=box, fillcolor=chartreuse, style=filled, fontsize=8];
     ModConfGen->ModConf [label="", weight=1, fontsize=8];

	 subgraph cluster2 {
		 WrappMod [label="{Wrapper Module 1|...|Wrapper Module N}", shape=record, fillcolor=lightblue, style=filled, fontsize=8];
		 ReceivalMod [label="{Receival/Forward Module 1|...|Receival/Forward Module N}", shape=record, fillcolor=lightblue, style=filled, fontsize=8];
		 color=black;
         style=rounded;
         label="Intermediate Modules";
		 }

	 WrappGen->WrappMod [label="", weight=1];
	 ReceivalGen->ReceivalMod [label="", weight=1];

	 BuildFile->WrappMod [label="", weight=1, style=dashed];
	 BuildFile->ReceivalMod [label="", weight=1, style=dashed];

	 subgraph cluster3 {
		 color=black;
         style=invis;
         label="Module Library";
         TempNode [label="Module Library", shape=box, style=rounded, fontsize=8]
		 }

	 ApplicationExe [label="Application\nExecutable", shape=box, fillcolor=orange, style=filled, fontsize=8];
	 GTIInstance [label="GTI\nInstance", shape=box, fillcolor=red, style=filled, fontsize=8];

	 WrappMod->GTIInstance [label="", weight=1];
	 ReceivalMod->GTIInstance [label="", weight=1];
	 TempNode->GTIInstance [label="", weight=1];
	 ModConf->GTIInstance [label="", weight=1];
	 ApplicationExe->GTIInstance [label="", weight=1];
 }
 \enddot
 * Figure 3: The instantiation process in detail.
 *
 * @section DTDSpecs Specifications of the Weaver input XML Files
 *
 * The XML specification for the layout is:
 * @verbinclude layout-specification.dtd
 *
 * The XML specification for the API is:
 * @verbinclude api-specification.dtd
 *
 * The XML specification for the GTI is:
 * @verbinclude modules-specification.dtd
 *
 * The XML specification for an analysis group is:
 * @verbinclude analysis-specification.dtd
 *
 * @section GenDTDSpecs Specifications of the Generator input XML Files
 *
 * The Wrapper generator uses the following DTD specification:
 * @verbinclude wrapp-gen-input.dtd
 *
 * The Receival/Forward generator uses the following DTD specification:
 * @verbinclude receival-gen-input.dtd
 */

/*
 * TODO:
 * - Superset module creation (all checks, default placement, records for all arguments, ...)
 * - Simpliefied creation of a layout (A GUI task ?)
 * - Post record size is non optimal:
 *  - in cases where a post analysis uses inputs that were already used for some pre analysis
 *    or pre call record forwarding, it would be possible to reuse that data instead of adding
 *    it to the post record
 *   - requires a "getPreRecord()" functionality that can be called when receiving a post record
 *   - required information is present for arguments, in that case all arguments of the "in" type can be reused
 *   - for operation results the "pre"/"post" order of its mapping can be used
 * - API call with void as return value fails in generation (tries to create "void ret...;")
 * - Shutdown and ORDER_PRE analyses that run on a non application place:
 * -- Shutdown will work
 * -- The pre records will arrive and be used for analysis execution at places directly connected to application
 * -- On other places the record only arrives if a connected place shuts down
 * -- Thus the pre op will only be executed if the place that should send it actually shuts down.
 * - Headers given in specifications
 * -- Currently some specs have a "include-directory" that is prepended to all given headers
 * -- This include directory should instead be added as INCLUDE_DIRECTORIES () in the generated CMake build file
 * -- Furthermore, it is necessary to be able to specify whether a header is a system header or not, e.g. whether to use "bla.h" or <bla.h>
 * - P-calls
 * -- For wrapp everwhere calls the P-call is not needed and just a burden to add, calls should be able to specify this
 * -- In a more general solution that also supports dynamic library call interception, it is necessary to soecify the technology used to implement the P-call
 * - Recursion guards
 * -- For MPI, a recursion guard should be implemented that avoids intercepting MPI calls that are used to implement another MPI call
 * -- For other calls it may be desirable to intercept such recursive calls, e.g. a wrapp everywhere call that is called from within another wrapped call
 * -- Currently recursion guards are disabled in the wrapper generator
 * -- In the future calls should be able to specify whether they need such a guard or not
 * - Module directory
 * -- PnMPI only supports one module directory
 * -- For GTI using tools this is troublesome as it requires the creation of a common directory with both the tools modules and the GTI modules
 */
