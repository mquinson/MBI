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
 * @file EnumList.h
 * 		@see gti::weaver::EnumList
 *
 * @author Tobias Hilbrich
 * @date 7.7.2010
 */

#ifndef ENUMLIST_H
#define ENUMLIST_H

#include <string>
#include <list>
#include <ostream>

#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Class to hold information for an enumeration setting.
			 */
			class EnumList : virtual public Printable
			{
			public:

				/**
				 * Constructor for initially invalid object.
				 */
				EnumList (void);

				/**
				 * Constructor for empty enumeration.
				 * Note that an enumeration should have
				 * at least one entry.
				 * @param id identifier for the enum.
				 */
				EnumList (int id);

				/**
				 * Constructor with id and list of entries.
				 * @param id identifier for the enum.
				 * @param entries list of entries for the enumeration.
				 */
				EnumList (int id, std::list<std::string> entries);

				/**
				 * Returns the id of the enumeration.
				 * @return identifier of the enum.
				 */
				int getId (void);

				/**
				 * Sets the id of the enumeration.
				 * @param id new identifier for the enumeration.
				 */
				void setId (int id);

				/**
				 * Returns the list of entries of this enumeration.
				 * @return list of entries.
				 */
				std::list<std::string> getEntries (void);

				/**
				 * Returns the firs entry for this enumeration.
				 * @return  first entry.
				 */
				std::string getFirstEntry (void);

				/**
				 * Checks whether given string is a valid entry
				 * for this enumeration.
				 * @param entry entry to check for
				 * @return true if valid entry, false otherwise.
				 */
				bool isValidEntry (std::string entry);

				/**
				 * Adds an entry at the end of the list of valid
				 * entries.
				 * @param entry entry to add.
				 */
				void addEntry (std::string entry);

				/**
				 * Removes the entry with the given name.
				 * @param entry entry to remove.
				 * @return true if the entry existed, false otherwise.
				 */
				bool removeEntry (std::string entry);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

			protected:
				/**
				 * Helper to get an iterator on an entry with given name.
				 * @param entry name of the entry to look up.
				 * @return iterator to the found entry, myEntries.end() if
				 *         the entry was not found.
				 */
				std::list<std::string>::iterator findEntry (std::string entry);

				int myId; /**< Identifier of the enumeration. */
				std::list<std::string> myEntries; /**< List of entries for the enumeration. */
			};
		} /*namespace modules*/
	}/*namespace weaver*/
}/*namespace gti*/

#endif /*ENUMLIST_H*/
