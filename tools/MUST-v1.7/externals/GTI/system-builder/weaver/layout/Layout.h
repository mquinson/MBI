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
 * @file Layout.h
 * 		@see gti::weaver::LevelGraph
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef LEVELGRAPH_H
#define LEVELGRAPH_H

#include <string>
#include <vector>

#include "GtiEnums.h"
#include "Communication.h"
#include "Level.h"
#include "SpecificationNode.h"

using namespace gti;

namespace gti
{
	namespace weaver
	{
		namespace layout
		{
			/**
			  * Singleton for the level layout and the default
			  * communication mode.
			  */
			class Layout
			{
			public:
			  ///////////////////////////
			  // public Methods
			  ///////////////////////////
			  /**
			   * Returns the instance of the singleton.
			   * @return instance.
			   */
			  static Layout* getInstance (void);

			  /**
			   * Loads a layout specification XML.
			   * @param layoutXml specification file name.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN load (std::string layoutXml);

              /**
               * Writes a representation of the layout as a DOT
               * file.
               * @param fileName output file name to use.
               * @return GTI_SUCCESS if successful.
               */
              GTI_RETURN writeLayoutAsDot (std::string fileName);

              /**
               * Prints basic information about the layout to the given file.
               * It contains the number of levels, the size of each level, and
               * which level is connected to what other levels.
               * @param fileName to print to.
               * @return GTI_SUCCESS if successful.
               */
              GTI_RETURN printInfo (std::string fileName);

              /**
               * Starts the preparation for the actual generation.
               * (Processes what call arguments are required where)
               * @return GTI_SUCCESS if successful.
               */
              GTI_RETURN processLayout (void);

              /**
               * Generates the input XML files for the wrapper
               * generator. Must only be called after a successful
               * load and processLayout.
               * @param baseName name that is prepended to the names of
               *        the generated input XML files, the naming scheme is
               *        [baseName]+[LevelOrder]+".xml"
               * @param outputBaseName name that is prepended to the names
               *        of the files to generate by the wrapper generator
               *        the naming scheme is [outputBaseName]+[LevelOrder]+[".cpp"|".h"|".xml"]
               * @param outXmlNames pointer to list of string pairs, used as output to store
               *                    the names of the input and output xml files for and of the
               *                    wrapper generator.
               * @return GTI_SUCCESS if successful.
               */
              GTI_RETURN generateWrapGenInput (
            		  std::string baseName,
            		  std::string outputBaseName,
            		  std::list<std::pair<std::string, std::string> > *outXmlNames);

              /**
               * Generates the input XML files for the receival
               * generator. Must only be called after a successful
               * load and processLayout.
               * @param baseName name that is prepended to the names of
               *        the generated input XML files, the naming scheme is
               *        [baseName]+[LevelOrder]+".xml"
               * @param outputBaseName name that is prepended to the names
               *        of the files to generate by the wrapper generator
               *        the naming scheme is [outputBaseName]+[LevelOrder]+[".cpp"|".h"|".xml"]
               * @param outXmlNames pointer to list of string pairs, used as output to store
               *                    the names of the input and output xml files for and of the
               *                    receival/forward generator.
               * @return GTI_SUCCESS if successful.
               */
              GTI_RETURN generateReceivalInput (
            		  std::string baseName,
            		  std::string outputBaseName,
            		  std::list<std::pair<std::string, std::string> > *outXmlNames);

              /**
               * Generates the input for the generator that creates
               * the module configuration.
               * @param outFileName file name for the input to
               *        create.
               * @param genOutputDir directory to use for output
               *        by the module configuration generator
               *        that is going to process the XML file
               *        created by this function.
               * @param genBaseOutputFileName base output file name
               *        to use by the generator that is going
               *        to process the input generated by this
               *        function. The naming scheme used
               *        for the outputfiles and the base name
               *        is described in module-config.dtd.
               */
              GTI_RETURN generateModuleConfigurationInput (
            		  std::string outFileName,
            		  std::string genOutputDir,
            		  std::string genBaseOutputFileName);

              /**
               * Maps implicit GTI components as necessary.
               */
              GTI_RETURN mapGtiImplicits (void);

              /**
               * Returns information on the format of the channel ids
               * used for this configuration.
               * @param pOutNum64s pointer to storage for int, will be set to number of 64bit values needed to store the channel id.
               * @param pOutNumBitsPerSubId pointer to storage for int, will be set to number of bits needed for each channel sub id.
               *
               * Will only return valid values after the channel id was computed as part of the processLayout function.
               */
              void getChannelIdInfo (int *pOutNum64s, int *pOutNumBitsPerSubId);

              /**
               * Returns the total number of levels in the layout.
               * @return number of levels.
               */
              int getNumLevels (void);

			  /**
			   * Prints statistics about the layout to std::cout.
			   */
			  GTI_RETURN printLayoutStatistics (void);
				
			protected:
			  ///////////////////////////
			  // Protected Methods
			  ///////////////////////////
			  /**
			   * Constructor
			   */
			  Layout ( );

			  /**
			   * Destructor
			   */
			  virtual ~Layout ( );

			  /**
			   * Reads the communication specifications.
			   * @param node the communications node.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readCommunications (SpecificationNode node);

			  /**
			   * Reads a level from the layout specification.
			   * @param node the XML node of the level to read.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readLevel (SpecificationNode node);

			  /**
			   * Reads a comm-strategy node.
			   *
			   * If ppOutStrategy is not NULL we search for a inter communication strategy,
			   * otherwise CommStrategyIntra must be valid and we search for a intra
			   * communication strategy.
			   *
			   * @param node to read.
			   * @param ppOutStrategy points to a pointer that is used to store
			   *                      the output communication strategy (inter) pointer.
			   * @param ppOutStrategyIntra points to a pointer that is used to store
               *                      the output communication strategy (intra) pointer.
			   * @param pOutSettings pointer to an allocated list of setting pointers,
			   *        used to enqueue all the settings that are read.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readCommStrategy(
			          SpecificationNode node,
			          CommStrategy **ppOutStrategy,
			          CommStrategyIntra **ppOutStrategyIntra,
			          std::list<Setting*> *pOutSettings);

			  /**
			   * Reads a comm-protocol node.
			   * @param node to read.
			   * @param ppOutProtocol points to a pointer that is used to store
			   *                      the output communication protocol pointer.
			   * @param pOutSettings pointer to an allocated list of setting pointers,
			   *        used to enqueue all the settings that are read.
			   * @return GTI_SUCCESS if successful.
			   */
              GTI_RETURN readCommProtocol(SpecificationNode node, CommProtocol **ppOutProtocol, std::list<Setting*> *pOutSettings);

              /**
               * Reads a connection between two levels.
               * @param node of the connection.
               * @return GTI_SUCCESS if successful.
               */
              GTI_RETURN readConnection (SpecificationNode node);

              /**
               * Reads a settings node.
               * @param node to read.
               * @param pOutSettings pointer to an allocated list of setting pointers,
               *        used to enqueue all the settings that are read.
               * @return GTI_SUCCESS if successful.
               */
              GTI_RETURN readSettings (SpecificationNode node, std::list<Setting*> *pOutSettings);

              /**
               * Determines the format of the channel ids and writes the results to
               * myChannelIdNum64s and myChannelIdBitsPerSubId.
               * @return GTI_SUCCESS if successful.
               */
              GTI_RETURN calculateChannelIdFormat (void);

			  ///////////////////////////
			  // Attributes
			  ///////////////////////////
              int myChannelIdNum64s; /**< Number of 64 bit values used for channel id.*/
              int myChannelIdBitsPerSubId; /**< Number of bits to be used for one of the two values in each sub id of the channel id.*/
			  std::map<int, Level*> myLevels; /**< Map for level order and level. */
			  Level * myRoot; /**< Root level.*/
			  Communication* myDefaultComm; /**< Default communication implementation.*/
			  std::list<Communication*> myCommunications; /**< List of used communication implementations.*/
			  static Layout* ourInstance; /**< Singleton instance.*/
			  bool myProfiling;
			};
		} /*namespace layout*/
	}
}
#endif // LEVELGRAPH_H
