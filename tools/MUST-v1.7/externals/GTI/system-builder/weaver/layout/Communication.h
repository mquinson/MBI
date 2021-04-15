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
 * @file Communication.h
 * 		@see gti::weaver::Communication
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string>
#include <vector>

#include "CommStrategy.h"
#include "CommStrategyIntra.h"
#include "CommProtocol.h"
#include "Setting.h"
#include "Printable.h"

using namespace gti::weaver::modules;

namespace gti
{
	namespace weaver
	{
		namespace layout
		{
			/**
			  * A selection of a communication strategy and a
			  * communication protocol used for implementing
			  * a inter level communication.
			  *
			  * Can be used either for a inter or intra level communication.
			  * In the first one uses the calls and constructor that uses a
			  * regular comm strategy. For intra communication one uses
			  * the calls and constructor with a intra communication.
			  */
			class Communication : virtual public Printable
			{
			public:
			  /**
			   * Invalid object Constructor.
			   */
			  Communication (void);

			  /**
			   * Proper constructor (inter).
			   */
			  Communication (CommStrategy *pStrategy, CommProtocol *pProtocol);

			  /**
			   * Proper constructor (intra).
			   */
			  Communication (CommStrategyIntra *pStrategy, CommProtocol *pProtocol);

			  /**
			   * Proper constructor with settings.
			   * The storage for the settings given as lists
			   * is given to this communication and will be
			   * freed upon destruction of the object.
			   * (inter)
			   */
			  Communication (
					  CommStrategy *pStrategy,
					  CommProtocol *pProtocol,
					  std::list<Setting*> stratSettings,
					  std::list<Setting*> protSettings);

			  /**
			   * Proper constructor with settings.
			   * The storage for the settings given as lists
			   * is given to this communication and will be
			   * freed upon destruction of the object.
			   * (intra)
			   */
			  Communication (
			          CommStrategyIntra *pStrategy,
			          CommProtocol *pProtocol,
			          std::list<Setting*> stratSettings,
			          std::list<Setting*> protSettings);

			  /**
			   * Destructor.
			   */
			  virtual ~Communication (void);

			  /**
			   * Sets the communication strategy for this communication.
			   * @param new_var comm strategy to use.
			   */
			  void setCommStrategy ( CommStrategy * new_var );

			  /**
			   * Returns the communication strategy used for this
			   * communication.
			   * @return communication strategy.
			   */
			  CommStrategy * getCommStrategy (void);

			  /**
			   * Sets the communication strategy for this communication (intra).
			   * @param new_var comm strategy to use.
			   */
			  void setCommStrategyIntra ( CommStrategyIntra * new_var );

			  /**
			   * Returns the communication strategy used for this
			   * communication.
			   * @return communication strategy.
			   */
			  CommStrategyIntra * getCommStrategyIntra (void);

			  /**
			   * Sets the communication protocol used for this
			   * communication.
			   * @param new_var new comm protocol.
			   */
			  void setCommProtocol ( CommProtocol * new_var );

			  /**
			   * Returns the communication protocol used for this
			   * communication.
			   * @return comm protocol.
			   */
			  CommProtocol * getCommProtocol (void);

			  /**
			   * Adds a setting to the communication strategy
			   * used for this communication.
			   * @param add_object the setting to add.
			   * @return true if the setting was successfully
			   *         added, false if the setting was invalid
			   *         for the communication strategy.
			   */
			  bool addStrategySetting ( Setting * add_object );

			  /**
			   * Returns the list of settings for the commuication
			   * strategy of this communication.
			   * @return setting list.
			   */
			  std::vector<Setting *> getStrategySettings (void);

			  /**
			   * Adds a setting for the communication protocol of
			   * this communication.
			   * @param add_object the setting to add.
			   * @return true if the setting was successfully
			   *         added, false if the setting was invalid
			   *         for the communication protocol.
			   */
			  bool addProtocolSetting ( Setting * add_object );

			  /**
			   * Returns the list of settings for the communication
			   * protocol.
			   * @return list of settings.
			   */
			  std::vector<Setting *> getProtocolSettings ( void );

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			protected:
			  void freeStrategySettings (void); /**< Frees the settings for the strategy. */
			  void freeProtocolSettings (void); /**< Frees the settings for the protocol. */

			  CommStrategy * myCommStrategy; /*Communication strategy used.*/
			  CommStrategyIntra * myCommStrategyIntra; /*Communication strategy used.*/
			  CommProtocol * myCommProtocol; /*Communication protocol used.*/

			  std::vector<Setting*> myStrategySettings; /*List of strategy settings.*/
			  std::vector<Setting*> myProtocolSettings; /*List of protocol settings.*/
			};
		} /*namespace layout*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // COMMUNICATION_H
