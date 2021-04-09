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
 * @file Record.h
 * 		@see gti::weaver::Record
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef RECORD_H
#define RECORD_H

#include <string>
#include <vector>

#include "Argument.h"

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			/**
			  * class Record
			  *
			  */

			class Record
			{
			public:

			  // Constructors/Destructors
			  //


			  /**
			   * Empty Constructor
			   */
			  Record ( );

			  /**
			   * Empty Destructor
			   */
			  virtual ~Record ( );

			  // Static Public attributes
			  //

			  // Public attributes
			  //


			  std::vector<Argument*> m_myargumentsVector;

			  // Public attribute accessor methods
			  //


			  // Public attribute accessor methods
			  //


			  /**
			   * Add a MyArguments object to the m_myargumentsVector List
			   */
			  void addMyArguments ( Argument * add_object );

			  /**
			   * Remove a MyArguments object from m_myargumentsVector List
			   */
			  void removeMyArguments ( Argument * remove_object );

			  /**
			   * Get the list of MyArguments objects held by m_myargumentsVector
			   * @return vector<Argument *> list of MyArguments objects held by
			   * m_myargumentsVector
			   */
			  std::vector<Argument *> getMyArgumentsList ( );

			protected:

			  // Static Protected attributes
			  //

			  // Protected attributes
			  //

			public:


			  // Protected attribute accessor methods
			  //

			protected:

			public:


			  // Protected attribute accessor methods
			  //

			protected:


			private:

			  // Static Private attributes
			  //

			  // Private attributes
			  //

			public:


			  // Private attribute accessor methods
			  //

			private:

			public:


			  // Private attribute accessor methods
			  //

			private:



			};
		} /*namespace calls*/
	}
}
#endif // RECORD_H
