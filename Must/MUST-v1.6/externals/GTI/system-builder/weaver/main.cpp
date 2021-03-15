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
 * @file main.cpp
 * 		The main file for the weaver component of the system builder.
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

/**
 * TODO namespace documentation for weaver.
 * TODO namesoace documentation for sub-namespaces !
 */

/**
 * @page WeaverImplementation Details on the weaver implementation
 * @todo write it ...
 * @todo also mention the verbose mode and the verbose DOT outputs ...
 * @todo provide a graph of what files are created by the weaver and who
 *       consumes them
 * @todo add overview class diagram
 *
 * @todo All the Singletons will currently not be freed!
 */

#include "Verbose.h"
#include "Gti.h"
#include "Analyses.h"
#include "ApiCalls.h"
#include "Layout.h"

#include "GtiEnums.h"

#include <string.h>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/time.h>

#include "main.h"

/**
 * Weaver main function.
 */
int main (int argc, char **argv)
{
	struct timeval t1,t2;
    gettimeofday(&t1, NULL);
	
	//=========== 0 read input or print help ===========
	//print help if requested
	if ((argc == 2) &&
		(
				(strncmp(argv[1], "--help", strlen("--help")) == 0 ) ||
				(strncmp(argv[1], "-help", strlen("--help")) == 0 ) ||
				(strncmp(argv[1], "-h", strlen("--help")) == 0 )
		)
	   )
	{
		print_usage (argv[0], std::cout);
		return 0;
	}

	//enough arguments ?
	if (argc < 5)
	{
		std::cerr << "Error: Not enough arguments!" << std::endl << std::endl;
        print_usage (argv[0], std::cerr);
        return 1;
    }

	//sort out what inputs are which specifications and whether they exist
    std::string layoutSpec = argv[1];
    std::string gtiSpec = argv[2];
    std::list <std::string> apiSpecs;
    std::list <std::string> analysisSpecs;

    for (int i = 1; i < argc; i++)
    {
    	//check file and get root node name
    	SpecificationType type;
    	if (!checkXmlFile (argv[i], &type))
    	{
    	   	std::cerr << "argument " << i << " (\"" << argv[i] << "\") is not valid XML file!" <<std::endl;
    	   	return 1;
    	}

    	//store the file
    	if (i == 1)
    	{
    		//the layout specification
    		if (type != SPECIFICATION_LAYOUT)
    		{
    			std::cerr << "argument " << i << " (\"" << argv[i] << "\") is not a layout specification." <<std::endl;
    			print_usage (argv[0], std::cerr);
    			return 1;
    		}
    		else
    		{
    			std::string layoutSpec = argv[i];
    		}
    	}
    	else
    	if (i == 2)
    	{
    		//the GTI specification
    		if (type != SPECIFICATION_GTI)
    		{
    			std::cerr << "argument " << i << " (\"" << argv[i] << "\") is not a GTI(Generic Tool Infrastructure) specification." <<std::endl;
    			print_usage (argv[0], std::cerr);
    			return 1;
    		}
    		else
    		{
    			std::string gtiSpec = argv[i];
    		}
    	}
    	else
    	{
    		//a API/Analysis specification
    		if ( (type != SPECIFICATION_API) &&
                 (type != SPECIFICATION_ANALYSIS)   )
    		{
    			std::cerr << "argument " << i << " (\"" << argv[i] << "\") is neither an API nor an analysis specification." <<std::endl;
    			print_usage (argv[0], std::cerr);
    			return 1;
    		}
    		else
    		if (type == SPECIFICATION_API)
    		{
    			apiSpecs.push_back(argv[i]);
    		}
    		else
    		{
    			analysisSpecs.push_back(argv[i]);
    		}
    	}
    }

    //0) Add implicit GTI api and analysis specifications
    analysisSpecs.push_front (WEAVER_GTI_IMPLICIT_ANALYSIS_FILEPATH);
    apiSpecs.push_front (WEAVER_GTI_IMPLICIT_API_FILEPATH);

	//1) read GTI
    if (Gti::getInstance()->load(gtiSpec) != GTI_SUCCESS)
    	return 1;

    //2) read Analyses
    if (Analyses::getInstance()->load (analysisSpecs) != GTI_SUCCESS)
    	return 1;

	//3) read API calls
	if (ApiCalls::getInstance()->load (apiSpecs) != GTI_SUCCESS)
		return 1;

	//3b) warn of type missmatches
	if (Analyses::getInstance()->printTypeMissmatchWarnings (std::cout) != GTI_SUCCESS)
		return 1;

	//3c) Validate correct usage of reductions
	if (Analyses::getInstance()->checkCorrectnessOfReductions () != GTI_SUCCESS)
		return 1;

	//4) read Layout
	if (Layout::getInstance()->load(layoutSpec) != GTI_SUCCESS)
		return 1;

	//4a) Map implicit GTI components
	if (Layout::getInstance()->mapGtiImplicits () != GTI_SUCCESS)
	    return 1;

	//5) print DOT representations of the inputs
	//   only in verbose mode of 2 or higher
	if (Verbose::getInstance()->getLevel() >= 2)
	{
		//Level 2 overview output
		VERBOSE(2) << "A graph representation of the read functions and analyses was created"
				   << ", see the files \"weaver-verbose-*.dot\""
				   << std::endl;
		Analyses::getInstance()->writeAnalysesAsDot("weaver-verbose-analysis-");
		ApiCalls::getInstance()->writeFunctionsAsDot("weaver-verbose-api-");
		Layout::getInstance()->writeLayoutAsDot("weaver-verbose-layout.dot");

		//Level 3 detail output
		if (Verbose::getInstance()->getLevel() >= 3)
		{
			Analyses::getInstance()->writeMappingsAsDot("weaver-verbose-call-mappings-");
		}
	}

	//6) Prepare for generation
	if (Layout::getInstance()->processLayout() != GTI_SUCCESS)
		return 1;

	//print new layout (if overspecified it may change) (also reductions will only now be visible)
	if (Verbose::getInstance()->getLevel() >= 2)
	{
		Layout::getInstance()->writeLayoutAsDot("weaver-verbose-layout-final.dot");
	}

	//print info for layout
	if (Layout::getInstance()->printInfo("weaver-layout-info.xml") != GTI_SUCCESS)
		return 1;

	//7) Generate Wrapper generator input
	std::list<std::pair<std::string, std::string> > wrappInOut;
	if (Layout::getInstance()->generateWrapGenInput("weaver-wrapp-gen-input-", "weaver-wrapp-gen-output-", &wrappInOut) != GTI_SUCCESS)
		return 1;

	//8) Generate Receival & Forwarding generator input
	std::list<std::pair<std::string, std::string> > recvInOut;
	if (Layout::getInstance()->generateReceivalInput("weaver-receival-gen-input-", "weaver-receival-gen-output-", &recvInOut) != GTI_SUCCESS)
		return 1;

	//9) Print input for P^nMPI config generation
	// Note: depends on side effects of 7) and 8)
	if (Layout::getInstance()->generateModuleConfigurationInput(
			"weaver-mod-conf-input.xml",
			"./",
			"weaver-mod-conf") != GTI_SUCCESS)
		return 1;
#ifndef POST_BUILD_ONLY
	//10) Generate script to run the generators
	if (createGeneratorScript ("weaver-run-generators.sh", wrappInOut, recvInOut) != GTI_SUCCESS)
		return 1;

	//11) Generate input file for build file generator
	if (createBuildGenInput("weaver-buildgen.xml", wrappInOut, recvInOut) != GTI_SUCCESS)
		return 1;
#endif
	//Print some statistics information if wanted
#ifdef GTI_WEAVER_STATISTICS
    gettimeofday(&t2, NULL);
	
    Layout::getInstance()->printLayoutStatistics ();
    std::cout << "Weaver processing time [usec]: " << (t2.tv_sec * 1000000 + t2.tv_usec)-(t1.tv_sec * 1000000 + t1.tv_usec) << std::endl;
#endif
	
	return 0;
}

//=============================
// checkXmlFile
//=============================
bool checkXmlFile (std::string fileName, SpecificationType *pOutType)
{
	xmlDocPtr document;
	xmlNodePtr currentPointer;

	document = xmlParseFile(fileName.c_str());

	if (document == NULL )
	{
		return false;
	}

	currentPointer = xmlDocGetRootElement(document);

	if (currentPointer == NULL)
	{
		xmlFreeDoc(document);
		return false;
	}

	//what specification type is it ?
	if (pOutType)
	{
		if (xmlStrcmp(currentPointer->name, (const xmlChar *) "gti-specification") == 0)
			*pOutType = SPECIFICATION_GTI;
		else
		if (xmlStrcmp(currentPointer->name, (const xmlChar *) "analysis-specification") == 0)
			*pOutType = SPECIFICATION_ANALYSIS;
		else
		if (xmlStrcmp(currentPointer->name, (const xmlChar *) "api-specification") == 0)
			*pOutType = SPECIFICATION_API;
		else
		if (xmlStrcmp(currentPointer->name, (const xmlChar *) "layout-specification") == 0)
			*pOutType = SPECIFICATION_LAYOUT;
		else
			*pOutType = SPECIFICATION_UNKNOWN;
	}

	xmlFreeDoc(document);

	return true;
}

//=============================
// print_usage
//=============================
bool print_usage (const char *executableName, std::ostream &out)
{
    out
		<< "Usage: "
    	<< executableName
        << " <LayoutSpecificationXML> <GtiSpecificationXML> " << std::endl
        << "       (<ApiSpecificationXML>|<AnalysisSpecificationXML>)*" << std::endl
        << std::endl
        << "Where at least one API Specification and at least one Analysis Specification have to be given."<< std::endl
        << std::endl
        << "E.g.: " << executableName << " layout.xml gti.xml mpi-spec.xml mpi-correctness.xml"
        << std::endl;

	return true;
}

//=============================
// createGeneratorScript
//=============================
GTI_RETURN createGeneratorScript (
		std::string outName,
		std::list<std::pair<std::string, std::string> > wrappInOut,
		std::list<std::pair<std::string, std::string> > recvInOut)
{
	/**
	 * @TODO Needs to be adapted for final version of trace record
	 *       generation interface initialization.
	 *       (Will P^nMPI be necessary or not ...)
	 * @TODO Absolute pathes need to be configured ...
	 */

	std::list<std::pair<std::string, std::string> >::iterator iter;

	//==Open ofstream for script and pnmpi config
	std::ofstream out (outName.c_str());

	//==Write script
	out
		<< "#! /bin/bash" << std::endl
		<< "export GTI_RECORD_GEN_IMPL=\"" << WEAVER_PNMPI_LIB_PATH << "/libgtiRecordGenImpl.so\"" << std::endl
		<< "export LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH:" << WEAVER_LD_LIBRARY_PATH << "\"" << std::endl
		<< "export DYLD_LIBRARY_PATH=\"" << WEAVER_LD_LIBRARY_PATH << ":$DYLD_LIBRARY_PATH\"" << std::endl
// 		<< "if [ \"x$GTI_MPIEXEC\" == \"x\" ] " << std::endl
// 		<< "then" << std::endl
// 		<< "    MYMPIEXEC=\"" << MPIEXEC << "\"" << std::endl
// 		<< "else" << std::endl
// 		<< "    MYMPIEXEC=\"$GTI_MPIEXEC\"" << std::endl
// 		<< "fi" << std::endl
		<< std::endl;

	for (iter = wrappInOut.begin(); iter != wrappInOut.end(); iter++)
	{
	    out << WEAVER_BIN_DIR << "wrappgen "<< iter->first << std::endl;
	}

	for (iter = recvInOut.begin(); iter != recvInOut.end(); iter++)
	{
	    out << WEAVER_BIN_DIR << "recvgen "<<  iter->first << std::endl;
	}

	//==Close streams
	out.close();

	return GTI_SUCCESS;
}

//=============================
// createBuildGenInput
//=============================
GTI_RETURN createBuildGenInput(
		std::string outName,
		std::list<std::pair<std::string, std::string> > wrappInOut,
		std::list<std::pair<std::string, std::string> > recvInOut)
{
	/**
	 * @todo replace absolute paths.
	 */

	std::list<std::pair<std::string, std::string> >::iterator iter;

	//==Open ofstream
	std::ofstream out (outName.c_str());

	//==write input
	out
		<< "<buildgen-input>" << std::endl
		<< "\t<settings>" << std::endl
		<< "\t\t<cmake-module-path>" << WEAVER_CMAKE_MODULES_DIR << "</cmake-module-path>" << std::endl
		<< "\t\t<module-dir>" << WEAVER_GTI_MODULE_PATH << "</module-dir>" << std::endl
		<< "\t</settings>" << std::endl
		<< "\t<modules>" << std::endl;

	std::list<std::pair<std::string, std::string> > *plists[2] = {&wrappInOut, &recvInOut};

	for (int i = 0; i < 2; i++)
	{
		for (iter = plists[i]->begin(); iter != plists[i]->end(); iter++)
		{
			std::string baseName, sourceName, moduleName;

			baseName = iter->second;
			size_t pos = baseName.find_last_of(".");
			baseName.replace (pos, baseName.length()-pos, "");

			sourceName = baseName + ".cpp";
			moduleName = baseName;

			out
				<< "\t\t<module>" << std::endl
				<< "\t\t\t<main-source>" << sourceName << "</main-source>" << std::endl
				<< "\t\t\t<module-name>" << moduleName << "</module-name>" << std::endl
				<< "\t\t\t<gen-output>" << iter->second << "</gen-output>" << std::endl
				<< "\t\t</module>" << std::endl;
		}
	}

	out
		<< "\t</modules>" << std::endl
		<< "</buildgen-input>" << std::endl;

	//==Close stream
	out.close();

	return GTI_SUCCESS;
}

/*EOF*/
