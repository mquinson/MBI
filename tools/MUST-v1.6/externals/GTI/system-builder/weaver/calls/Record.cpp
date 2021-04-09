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
 * @file Record.cpp
 * 		@see gti::weaver::Record
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "Record.h"

using namespace gti::weaver::calls;

// Constructors/Destructors
//

Record::Record ( ) {
}

Record::~Record ( ) { }

//
// Methods
//


// Accessor methods
//


/**
 * Add a MyArguments object to the m_myargumentsVector List
 */
void Record::addMyArguments ( Argument * add_object ) {
  m_myargumentsVector.push_back(add_object);
}

/**
 * Remove a MyArguments object from m_myargumentsVector List
 */
void Record::removeMyArguments ( Argument * remove_object ) {
  int i, size = m_myargumentsVector.size();
  for ( i = 0; i < size; ++i) {
  	Argument * item = m_myargumentsVector.at(i);
  	if(item == remove_object) {
  		std::vector<Argument *>::iterator it = m_myargumentsVector.begin() + i;
  		m_myargumentsVector.erase(it);
  		return;
  	}
   }
}

/**
 * Get the list of MyArguments objects held by m_myargumentsVector
 * @return vector<Argument *> list of MyArguments objects held by
 * m_myargumentsVector
 */
std::vector<Argument *> Record::getMyArgumentsList ( ) {
  return m_myargumentsVector;
}

// Other methods
//


