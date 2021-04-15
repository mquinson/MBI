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
 * @file ModConfGen.h
 *		@see gti::codegen::ModConfGen
 *
 * @author Tobias Hilbrich
 * @date 02.11.2010
 */

#include <fstream>
#include <map>
#include <list>

#include "SpecificationNode.h"

#ifndef MODCONFGEN_H
#define MODCONFGEN_H

using namespace gti::weaver;

/**
 * Prints usage information for the generator.
 * @param execName name of the generator
 *        executable (argv[0]).
 * @param out output stream to use.
 */
void printUsage (std::string execName, std::ostream &out);

namespace gti
{
	namespace codegen
	{

		/*
		 * Forward declaration.
		 */
		class Level;
		class Module;
		class Instance;

		/**
		 * Enumeration of module types.
		 * Some of the module types cause a
		 * different behavior when writing a
		 * modules configuration.
		 */
		enum MODULE_TYPE
		{
			MODULE_UNKNOWN = 0,
			MODULE_ANALYSIS,
			MODULE_WRAPPER,
			MODULE_RECEIVAL,
			MODULE_PLACE,
			MODULE_PROTOCOL,
			MODULE_STRATEGY
		};

		/**
		 * A level of the overall tool and application.
		 * Contains a list of all modules that need
		 * to be loaded for this level along with their
		 * individual instances.
		 */
		class Level
		{
		public:

			/**
			 * Constructor.
			 * @param size of the level.
			 * @param order order id of the level.
			 */
			Level (int size, int order);

			/**
			 * Destructor.
			 */
			~Level (void);

			/**
			 * Returns the wrapper module used on
			 * this level or NULL if none is available.
			 * @return pointer to wrapper module or NULL.
			 *
			 * The pointer to the module must not be
			 * freed, its memory is managed by the level.
			 */
			Module* getWrapper (void);

			/**
			 * Returns the size of the level.
			 * @return size.
			 */
			int getSize (void);

			/**
			 * Returns the order id of the level.
			 * @return order id.
			 */
			int getOrder (void);

			/**
			 * Adds a module to this level.
			 * The order of the addModule calls will
			 * be reflected when the configuration is
			 * generated with printToConfig.
			 * The module needs to be allocated,
			 * the pointed to memory will be freed
			 * when the level is destructed and must
			 * thus not be freed by the user.
			 * @param module to add.
			 * @return true iff successful.
			 */
			bool addModule (Module* module);

			/**
			 * Prints the PnMPI configuration for this
			 * level.
			 * @param out the ostream to write to.
			 * @return true iff successful.
			 */
			bool printToConfig (std::ostream& out);

			/**
			 * Merges the module instances of two levels
			 * that will share a single module configuration
			 * and use multiple stacks. The given level
			 * must be of higher order and its module instances
			 * will be merged into this levels module instances.
			 * @param levelToMergeIn level to merge into this level.
			 * @return true iff successful.
			 */
			bool mergeInLevel (Level* levelToMergeIn);

			/**
			 * Returns a pointer to the module with the given
			 * name if found, NULL otherwise.
			 * @param libName name of the module to search for.
			 * @return pointer to module if found, NULL otherwise.
			 *
			 * The memory being pointed to by the return value
			 * is still managed by the level, the caller must
			 * not free it.
			 */
			Module* getModuleNamed (std::string libName);

			/**
			 * Searches for an instance that has a unique name
			 * within this level and returns a pointer to
			 * its instance and module if found.
			 * @param name name of the instance to find.
			 * @param pOutModule pointer to storage for the module pointer, only set if found.
			 * @param pOutInstance pointer to storage for the instance pointer, only set if found.
			 * @return true if found, false otherwise.
			 */
			bool findUniqueInstanceName (
					std::string name,
					Module **pOutModule,
					Instance **pOutInstance);

			/**
			 * Prints the whole Level as a subgraph in DOT syntax.
			 * @param out output stream to use.
			 * @param clusterIndex next index to use for DOT clusters
			 *        (subgraph), index must be incremented after usage.
			 * @param deps node dependencies to add to bottom of graph.
			 * @return true iff successful.
			 */
			bool printAsDot (std::ofstream &out, int *clusterIndex, std::string *deps);

			/**
			 * Searches this level for a protocol instance that
			 * connects to the given level.
			 * @param toOrder order of the level to which the protocol connects.
			 * @param pOutName pointer to storage for instance name of the
			 *        found protocol, only set if such a protocol was found.
			 * @return true iff successful.
			 */
			bool findProtocolName (int toOrder, std::string *pOutName);

			/*====================================================
			 *                 Static Members
			 *====================================================*/

			/**
			 * Returns a pointer to the level with the
			 * given order, or NULL if no such level
			 * exists.
			 * @param order of the level to return.
			 * @return level with given order or NULL.
			 */
			static Level* getLevel (int order);

			/**
			 * Returns the communication id that must be used
			 * for communications starting at the level
			 * fromOrder and ending at the level toOrder.
			 */
			static int getCommId (int fromOrder, int toOrder, bool isIntra = false);

		protected:
			int mySize; /**< Size of this level. */
			int myOrder; /**< Order id of this level. */

			std::list<Module*> myModules; /**< List of modules used on this level. */
			Module* myWrapper; /**< The wrapper module of this level or NULL if not present. */

			static std::map<int, Level*> ourLevels; /**< Existing levels. */
			static std::map<std::pair<int,int>, int> ourCommIds; /**< Maps a (from,to) pair of level orders to a comm id. */
			static int ourNextCommId; /**< The next communication id to use. */
			static int ourNextIntraCommId; /**< The next communication id for intra communication to use. */
		};

		/**
		 * A module which may have multiple instances.
		 */
		class Module
		{
		public:
			/**
			 * Constructor.
			 * @param libName pnmpi configuration file name of this module.
			 * @param type of this module.
			 */
			Module (std::string libName, MODULE_TYPE type);

			/**
			 * Destructor.
			 */
			~Module (void);

			/**
			 * Adds an instance to this module.
			 * The memory pointed to by "instance"
			 * will be managed by the module, thus
			 * the caller must not free this memory.
			 *
			 * @param instance to add.
			 * @return true iff successful.
			 */
			bool addInstance (Instance* instance);

			/**
			 * Prints this module as PnMPI configuration.
			 * @param out stream to print to.
			 * @return true iff successful.
			 */
			bool printToConfig (std::ostream& out);

			/**
			 * Returns the library name of this module.
			 * @return library name.
			 */
			std::string getLibName (void);

			/**
			 * Returns the type of this module.
			 * @return type.
			 */
			MODULE_TYPE getType (void);

			/**
			 * Merges the given module into this module.
			 * This requires both modules to have the same
			 * libName. This operation moves all instances
			 * of the given module into this module and
			 * marks the given module as emptied, which
			 * influences its behavior when printing the
			 * configuration file.
			 * @param module to merge into this module.
			 * @param true iff successful.
			 */
			bool mergeInModule (Module* module);

			/**
			 * Returns a pointer to the module instance with
			 * the given name if found, NULL otherwise.
			 * @param name of the instance.
			 * @return pointer to instance if found, NULL otherwise.
			 *
			 * The memory to which the return value points
			 * is still managed by the module, and must thus
			 * not be freed by the caller.
			 */
			Instance* findInstanceNamed (std::string name);

			/**
			 * Prints the Module as a subgraph in DOT syntax.
			 * @param out output stream to use.
			 * @param clusterIndex next index to use for DOT clusters
			 *        (subgraph), index must be incremented after usage.
             * @param deps node dependencies to add to bottom of graph.
			 * @return true iff successful.
			 */
			bool printAsDot (std::ofstream &out, int *clusterIndex, std::string *deps);

			/**
			 * Searches this module for a protocol instance that
			 * connects to the given level.
			 * @param toOrder order of the level to which the protocol connects.
			 * @param pOutName pointer to storage for instance name of the
			 *        found protocol, only set if such a protocol was found.
			 * @return true iff successful.
			 */
			bool findProtocolName (int toOrder, std::string *pOutName);

		protected:
			std::string myLibName; /**< Library name (used for pnmpi conf) of this module. */
			MODULE_TYPE myType; /**< Type of this module. */
			bool        myWasEmptiedAtMerge; /**< True if all instances of this module were merged into the same module of a different level. */
			Instance*   myFirstInstance; /**< Pointer to the first instance that was added to this module, not set to NULL during merge, used to refer to the instance to use for places. */

			std::list<Instance*> myInstances; /**< List of module instances. */
		};

		/**
		 * A module instance.
		 */
		class Instance
		{
		public:
			/**
			 * Constructor.
			 * @param pLevel pointer to the level to which this instance belongs.
			 *        This is important in cases where an instance is merged into
			 *        the module of another level, in such a situation it is clear
			 *        to which level this instance really belongs to.
			 * @param uniqueName uniqueInstance name, the name must be unique for
			 *        all instances of any instance of this instances module and all
			 *        other modules with an equal library name, use prefixing to
			 *        enforce this.
			 * @param isToLevel set this to true if this is a communication strategy
			 *        that needs to store to which level (order id) it is connected.
			 * @param toLevel if isToLevel is true the order id for the connection.
			 */
			Instance (
					Level* pLevel,
					std::string uniqueName,
					bool isToLevel = false,
					int toLevel = 0,
					bool isDown = false,
					bool isIntra = false);

			/**
			 * Destructor.
			 */
			~Instance (void);

			/**
			 * Adds a key value pair as a setting to this instance.
			 * @param key of the setting.
			 * @param value of the setting.
			 * @return true iff successful.
			 */
			bool addData (std::string key, std::string value);

			/**
			 * Prints this instance into a PnMPI configuration.
			 * @param out stream to use.
			 * @param instanceIndex index used for this instance.
			 * @param myType type of the module to which this instance belongs.
			 * @return true iff successful.
			 */
			bool printToConfig (std::ostream& out, int instanceIndex, MODULE_TYPE myType);

			/**
			 * Returns the instance name.
			 * @return instance name.
			 */
			std::string getUniqueName (void);

			/**
			 * Adds the name of a module and one of its
			 * instances to the list of module instances
			 * used by this module.
			 * It is up to the caller of this function to
			 * guarantee that such a module with such a
			 * library name and an instance with the given
			 * unique name exists.
			 * @param moduleName name of the module from which the used instance is (its libName).
			 * @param instanceName name of the instance being used (its unique name).
			 */
			bool addUsedInstance (std::string moduleName, std::string instanceName);

			/**
			 * Prints the Instance as a node in DOT syntax.
			 * @param out output stream to use.
			 * @param clusterIndex next index to use for DOT clusters
			 *        (subgraph), index must be incremented after usage.
			 * @param deps node dependencies to add to bottom of graph.
			 * @return true iff successful.
			 */
			bool printAsDot (std::ofstream &out, int *clusterIndex, std::string *deps);

			/**
			 * Returns true if this module instance is a protocol
			 * and connected to the level with given order.
			 * @param order of the level to be conncetced to.
			 * @return true if this instance is connected to the level, false otherwise.
			 */
			bool isToLevel (int order);

		protected:
			std::string myUniqueName; /**< Instance name that needs to be unique. */
			bool myIsToLevel; /**< True if this is a comm protocol. */
			int myToLevel; /**< If myIsToLevel==true, the level (order id) to which this protocol is connected. */
			std::map<std::string, std::string> myData; /**< Settings for this module instance. */
			Level* myLevel; /**< Pointer to the level to which this instance belongs. */
			std::list<std::pair<std::string, std::string> > myUsedInstances; /**< List of (moduleName, instanceName) pairs of used instances. */
			bool myIsDown; /**< For protocol modules if false it is directed upwards, otherwise downwards. */
			bool myIsIntra; /**< For protocol module instances, true if communication protocol is used for intra communication.*/
		};

		/**
		 * Generator to process a module configuration input
		 * XML in order to generate a PnMPI module configuration.
		 */
		class ModConfGen
		{
		public:
			/**
			 * Constructor.
			 * @param inputFile input XML to process and generate for.
			 * @param mergeFile name of a file that specifies which
			 *        levels should be merged into one configuration
			 *        file, if no merging should take place it must be
			 *        set to "" (empty string).
			 * @param retVal pointer to storage for return value to use.
			 *        (is set to 0 if generation successful and to 1
			 *         otherwise)
			 */
			ModConfGen (
					std::string inputFile,
					std::string mergeFile,
					int* retVal);

			/**
			 * Destructor.
			 */
			~ModConfGen (void);

		protected:
			std::string myOutputDir; /**< Name of the output directory to use .*/
			std::string myBaseFileName; /**< Prefix for the output configuration file names, see module-config.dtd for details. */

			std::list<std::list<Level*> > myMergedLevels; /**< List of level list, each entry in the first list represents one output configuration file, whereas the inner list is used to store which levels belong to the configuration file. */

			/**
			 * Reads the module configuration input starting
			 * at the "levels" node.
			 * @param node to start at.
			 * @return true iff successful.
			 */
			bool readModConf (SpecificationNode node);

			/**
			 * Reads a module node from the module configuration
			 * input.
			 * @param node pointing to the module node to read.
			 * @param l level to which the module node belongs.
			 * @return true iff successful.
			 */
			bool readModule (SpecificationNode node, Level* l);

			/**
			 * Reads a replationship node from the module conf
			 * input.
			 * @param node relationship node to read.
			 * @param l level to which the relationship node belongs.
			 */
			bool readRelationship (SpecificationNode node, Level* l);

			/**
			 * Generates the configurations after reading the
			 * settings, reading the configuration input and
			 * doing the merge.
			 * @return true iff successful.
			 */
			bool generateConfigurations (void);

			/**
			 * Reads the given root node of the merge file
			 * and applies the listed merges.
			 * @param node root node of merge file.
			 * @return true iff successful.
			 */
			bool readAndApplyMerge (SpecificationNode node);

			/**
			 * Searches the merged levels lists for a list
			 * which has a level with the given order
			 * as first element.
			 * @param order of the level to be in the list.
			 * @param pOutIter pointer to storage for iterator
			 *        that points to the list with the searched for
			 *        level, only set if found.
			 * @return true if successful, false otherwise.
			 */
			bool findIterForOrder (int order, std::list<std::list<Level*> >::iterator *pOutIter);
		};
	}
}

#endif /*MODCONFGEN_H*/
