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
 * @file Setting.h
 * 		@see gti::weaver::Setting
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef SETTING_H
#define SETTING_H

#include <string>
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace layout
		{
			/**
			  * A setting of a communication protocol or strategy.
			  * It refers to a used setting which got a value
			  * assigned. Descriptions of what settings are valid
			  * are made with gti::weaver::modules::SettingsDescription.
			  */
			class Setting : virtual public Printable
			{
			public:
			  /**
			   * Invalid object Constructor
			   */
			  Setting ( );

			  /**
			   * Proper constructor.
			   * @param name of the setting.
			   * @param value set value for the setting.
			   */
			  Setting (std::string name, std::string value);

			  /**
			   * Destructor
			   */
			  virtual ~Setting ( );

			  /**
			   * Sets the name of this setting.
			   * @param new_var name.
			   */
			  void setName ( std::string new_var );

			  /**
			   * Returns the the name of the setting.
			   * @return name.
			   */
			  std::string getName ( );

			  /**
			   * Sets the set value for the setting.
			   * @param new_var value for setting.
			   */
			  void seValue ( std::string new_var );

			  /**
			   * Returns the value that is set for this
			   * setting.
			   * @return value.
			   */
			  std::string getValue ( );

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			  /**
			   * Clones this setting and returns a new
			   * object with the same state.
			   * The storage to which the returned pointer
			   * shows must be freed by the caller.
			   */
			  Setting* clone (void);

			protected:
			  std::string myName;
			  std::string myValue;
			};
		} /*namespace layout*/
	}
}
#endif // SETTING_H
