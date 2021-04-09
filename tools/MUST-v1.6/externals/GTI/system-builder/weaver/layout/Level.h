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
 * @file Level.h
 * 		@see gti::weaver::Level
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <vector>
#include <stdint.h>

#include "GtiEnums.h"
#include "Communication.h"
#include "Adjacency.h"
#include "Calculation.h"
#include "AnalysisModule.h"
#include "Printable.h"
#include "Place.h"
#include "CallProperties.h"
#include "OperationInput.h"

using namespace gti::weaver::analyses;
using namespace gti::weaver::modules;
using namespace gti::weaver::calls;
using namespace gti;

namespace gti
{
	namespace weaver
	{
		//Forward declaration of the Wrapper class
		namespace generation
		{
			class GenerationBase;
			class ReceiveForwarding;
			class Wrapper;
			class ModuleConfig;
		}

		namespace layout
		{
			class Adjacency;

			/**
			  * A level (layer) of processes that executes analyses.
			  */
			class Level : virtual public Printable
			{
			public:
				/*
				 * We need to make friends with the generation classes,
				 * I guess due to them not being in the same namespace.
				 * If someone knows for sure he is welcome to enlight
				 * me ;)
				 */
				friend class generation::GenerationBase;
				friend class generation::Wrapper;
				friend class generation::ReceiveForwarding;
				friend class generation::ModuleConfig;

			  /**
			   * Invalid object Constructor.
			   */
			  Level ( );

			  /**
			   * Proper object constructor.
			   * @param order order if of this level.
			   * @param size # of processes in this level.
			   * @param place for this level, or NULL if application level.
			   */
			  Level (int order, uint64_t size, Place *place);

			  /**
			   * Destructor
			   */
			  virtual ~Level ( );

			  /**
			   * Adds an incoming arc to this level.
			   * @param add_object arc to add.
			   * @return true if successfully added, false if duplicate.
			   */
			  bool addInArc ( Adjacency * add_object );

			  /**
			   * Returns the list of in-going arcs of this level.
			   * @return list or arcs.
			   */
			  std::vector<Adjacency *> getInList ( );

			  /**
			   * Adds an outgoing arc.
			   * @param add_object arc to add.
			   * @return true if successfully added, false if duplicate.
			   */
			  bool addOutArc ( Adjacency * add_object );

			  /**
			   * Removes out arcs to the given level.
			   * @param targetLevel the level to which arcs
			   *        are removed.
			   */
			  void removeOutArc (Level *targetLevel);

			  /**
			   * Returns the list of out-going arcs of this level.
			   * @return list of arcs.
			   */
			  std::vector<Adjacency *> getOutList ( );

			  /**
			   * Sets the level order (layer id).
			   * @param new_var new order.
			   */
			  void setOrder ( int new_var );

			  /**
			   * Returns the level order (layer id).
			   * @return order.
			   */
			  int getOrder ( );

			  /**
			   * Sets the size of the level (number of processes).
			   * @param new_var size.
			   */
			  void setSize ( int new_var );

			  /**
			   * Returns the size of the level (number of processes).
			   * @return size.
			   */
			  int getSize ( );

			  /**
			   * Adds an analysis module to the list of analyses
			   * executed on this level.
			   * Automatically adds all dependencies of
			   * the new analysis module to the list of analysis modules
			   * for this level.
			   *
			   * Also computes the overall list of analysis functions computed
			   * on this level and stores it in myAnalyses, can be retrieved with
			   * getAnalyses.
			   *
			   * @param analysis module to add.
			   * @param allowReductions set to true to add a reduction to the level.
			   *
			   * Beware: reductions should not be added manually, but are rather
			   * added when calling calculateReductionPlacement.
			   */
			  void addAnalysisModule (AnalysisModule* anModule, bool allowReductions = false);

			  /**
			   * Returns the list of analyses executed
			   * on this level.
			   * @return list.
			   */
			  std::list<Analysis*> getAnalyses (void);

			  /**
			   * Returns true if the given analysis module is placed
			   * on this level.
			   * @param module module to check for.
			   * @return true iff placed.
			   */
			  bool isAnalysisModulePlaced (AnalysisModule* module);

			  /**
			   * Returns the list of analysis modules executed
			   * on this level.
			   * @return list.
			   */
			  std::list<AnalysisModule*> getAnalysisModules (void);

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			  /**
			   * Returns a pointer to the GTI place that represents
			   * this level.
			   * May return NULL for appliaction level and invalid
			   * objects.
			   * @return pointer to place.
			   */
			  Place* getPlace (void);

			  /**
			   * Returns true if the level graph is acyclic.
			   * @param pOutNumReachable pointer to int, used
			   *      to store the number of reachable nodes in.
			   *      (Is computed as a side effect of the computation)
			   * @return true or false.
			   */
			  bool isAcyclic (int *pOutNumReachable);

			  /**
			   * Drops all in-arcs but one, the one remaining
			   * is the from the level with the highest order.
			   * Use case: to create a tree from a DAG.
			   */
			  void reduceToOneInArc (void);

			  /**
			   * Adds the maximum number of reductions to this
			   * level.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN calculateReductionPlacement (void);

			  /**
			   * Loops over all level analyses and their mappings
			   * to determine which inputs are used for what calls,
			   * it stores this output in myCallProperties.
			   */
			  void calculateUsedArgs (void);

			  /**
			   * Graph propagation algorithm which determines the
			   * args to receive for each level. Should be executed
			   * on the root node of the level layout.
			   */
			  void calculateArgsToReceive (void);

			  /**
			   * Graph propagation algorithm which determines the
			   * args to receive for wrapp-down calls on each level.
			   * Should be executed
			   * on the root node of the level layout.
			   */
			  void calculateArgsToReceiveReverse (void);

			  /**
			   * Assigns uids for incoming records of this level
			   * and all of its descendants. Should be evoked
			   * on the root of the level graph.
			   */
			  void calculateUIds (void);

			  /**
			   * Assigns uids for incoming records of this level
			   * and all of its descendants. Should be evoked
			   * on the root of the level graph.
			   */
			  void calculateUIdsReverse (void);

			  /**
			   * Prints the call properties of this level to the
			   * given out stream. Beware: the output may be very
			   * lengthy for moderately sized input descriptions.
			   */
			  std::ostream& printCallProperties (std::ostream& out);

			  /**
			   * Calculated which operations need to per executed
			   * for what call.
			   */
			  void calculateNeededOperations (void);

			  /**
			   * Generates the wrapper generator input for this
			   * level.
			   * @param fileName output file name.
			   * @param sourceFileName name of the source
			   *        file that should be created by
			   *        the wrapper generator.
			   * @param headerFileName name of the header
			   *        file that should be created by the
			   *        wrapper generator.
			   * @param logFileName name of the log file that
			   *        should be created by the wrapper
			   *        generator.
			   */
			  void generateWrappGenInput (
					  std::string fileName,
					  std::string sourceFileName,
					  std::string headerFileName,
					  std::string logFileName);

			  /**
			   * Generates the receival generator input for this
			   * level.
			   * @param fileName output file name.
			   * @param sourceFileName name of the source
			   *        file that should be created by
			   *        the generator.
			   * @param headerFileName name of the header
			   *        file that should be created by the
			   *        generator.
			   * @param logFileName name of the log file that
			   *        should be created by the
			   *        generator.
			   */
			  void generateReceivalInput (
					  std::string fileName,
					  std::string sourceFileName,
					  std::string headerFileName,
					  std::string logFileName);

			  /**
			   * Generates part of the module configuration input
			   * that is used to generate a PnMPI/Dynasty configuration.
			   * It prints the modules of this level along with their
			   * relationship and information on the level itself to
			   * the XML. The header and footer along with the settings
			   * still need to be written by another entity.
			   * @param fileName output file name.
			   */
			  void generateModuleConfigurationInput (std::string fileName);

			  /**
			   * Adds arguments and operations needed to forward and initialize
			   * channel Ids in wrappers and receival modules.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN addChannelIdData ();

			  /**
			   * Returns true if this level has an intra communication.
			   * @return true if has intra communication.
			   */
			  bool hasIntraCommunication (void);

			  /**
			   * Sets a intra communication for this level.
			   * The memory associated with the pointer is managed
			   * by the Layout singleton and must not be freed.
			   * @param intraComm communication to set.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN setIntraCommunication (Communication* intraComm);

			  /**
			   * Checks whether any module on this layer creates a wrap across call,
			   * if so we must have a intra communication module!
			   */
			  GTI_RETURN checkWrapAcrossUsage (void);

			  /**
			   * Goes over all call properties of this level and determines whether
			   * any wrap across call is actually created by a module running on this
			   * level, if so we mark it as to be wrapped.
			   */
			  GTI_RETURN markWrapAcrossCallsToWrap (void);

			  /**
			   * Loops over all modules that are marked as add-automagically and adds them
			   * when possible.
			   * These automagic modules are added if all of their inputs are in the used args
			   * (without args to receive, but rather just the actually used args on the layer).
			   */
			  GTI_RETURN addAutomagicModules (void);

			protected:

			  int myOrder;
			  uint64_t mySize;
			  Place* myPlace;

			  std::string myWrapperSourceName,
                               myReceivalSourceName;

			  std::vector<Adjacency*> myInList;
			  std::vector<Adjacency*> myOutList;
			  std::list<Analysis*>	  myAnalyses;
			  std::list<AnalysisModule*>	  myAnalysisModules;

			  std::map<Call*, CallProperties*> myCallPropertiesPre;
			  std::map<Call*, CallProperties*> myCallPropertiesPost;

			  std::map<std::pair<Call*, CalculationOrder>, std::list<int> > myReductionForwards; /**< Stores for each event to which a reduction is mapped on this place the list of out channels to which the reduction must be forwarded.*/

			  std::list<OperationInput*> myOpInputsToFree;

			  Communication* myIntraCommunication; /**< Optional intra layer communication, memory is managed by Layout singleton!.*/
			  bool myProfiling;

			  /**
			   * Emptys a list of arcs and frees the
			   * Adjacency in it.
			   * @param target the list of arcs to free.
			   */
			  void freeArcs (std::vector<Adjacency*> *target);

			  /**
			   * Returns a map for all nodes reachable from this
			   * level.
			   */
			  std::map<Level*, bool> getReachableNodes (void);

			  /**
			   * Adds all arguments of the given properties to the
			   * args to receive for this level.
			   * Usage: adds the args required for a descendant.
			   * @param order order for which to do the operation.
			   * @param properties args to add.
			   * @param forWrappDown if true only properties of wrapp-down calls are added,
			   *              in this case this is actually called with call properties of an ancestor instead
			   *              of a descandant, otherwise all non wrapp-down properties are added,
			   *              which reflects the originial behavior of this function.
			   */
			  void addArgsFromDescendant (
					  CalculationOrder order,
					  std::map<Call*, CallProperties*> properties,
					  bool forWrappDown = false);

			  /**
			   * Returns the pre or post order properties dependeing on
			   * the given order.
			   * @param order to return properties for.
			   * @return pointer to properties.
			   */
			  std::map<Call*, CallProperties*>* getCallPropertiesForOrder (CalculationOrder order);
			};
		} /*namespace layout*/
	}
}
#endif // LEVEL_H
