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
 * @file SpecificationNode.h
 * 		@see gti::weaver::SpecificationNode
 *
 * @author Tobias Hilbrich
 * @date 08.07.2010
 */

#ifndef SPECIFICATIONNODE_H
#define SPECIFICATIONNODE_H

#include <string>
#include <list>

#include <libxml/parser.h>
#include <libxml/tree.h>

namespace gti
{
	namespace weaver
	{
		/**
		 * Wrapper for an XML node.
		 * Used for common search and navigation tasks.
		 */
		class SpecificationNode
		{
		public:

			/**
			 * Empty Constructor.
			 */
			SpecificationNode ( );

			/**
			 * Constructor.
			 */
			SpecificationNode (xmlNodePtr node);

			/**
			 * Assignment operator with xmlNodePtr.
			 * @param node to assign.
			 * @return new object.
			 */
			SpecificationNode& operator = (xmlNodePtr node);

			/**
			 * Operator for equality with xmlNodePtr.
			 * @param node to compare against.
			 * @return true if equal, false otherwise.
			 */
			bool operator == (const xmlNodePtr& node) const;

			/**
			 * Cast operator to automatically cast a
			 * SpecificationNode to an xmlNodePtr.
			 */
			operator xmlNodePtr() const;

			/**
			 * Easy access to the wrapped node.
			 * @return node.
			 */
			xmlNodePtr operator () (void);

			/**
			 * Searches for the first child node with given name.
			 * @param name child name to search for.
			 * @return node if found, NULL representation otherwise.
			 */
			SpecificationNode findChildNodeNamed (std::string name);

			/**
			 * Searches for the first child node with any of the
			 * given names.
			 * @param names list of names to search for.
			 * @param pOutName if found it is set to the name of the found child.
			 * @return node if found, NULL representation otherwise.
			 */
			SpecificationNode findChildNodeNamed (std::list<std::string> names, std::string *pOutName);

			/**
			 * Searches for the first child node with given name.
			 * Prints an error if the node was not found.
			 * @param name child name to search for.
			 * @param error error text to print if node not found, newline will be added.
			 * @return node if found, NULL representation otherwise.
			 */
			SpecificationNode findChildNodeNamedOrErr (std::string name, std::string error);

			/**
			 * Searches for the next sibling that has the given name.
			 * @param name of the sibling node.
			 * @return node if found, NULL representation otherwise.
			 */
			SpecificationNode findSiblingNamed (std::string name);

			/**
			 * Searches for the next sibling that has any of the given
			 * names.
			 * @param names list of names to search for.
			 * @param pOutName if found it is set to the name of the found sibling.
			 * @return node if found, NULL representation otherwise.
			 */
			SpecificationNode findSiblingNamed (std::list<std::string> names, std::string *pOutName);

			/**
			 * Searches for the next sibling that has the given name.
			 * Prints an error if the node was not found.
			 * @param name of the sibling node.
			 * @param error error text to print if node not found, newline will be added.
			 * @return node if found, NULL representation otherwise.
			 */
			SpecificationNode findSiblingNamedOrErr (std::string name, std::string error);

			/**
			 * Returns the content of an attribute.
			 * @param name name of the requested attribute.
			 * @param pOutContent pointer to a string which is used to hold the
			 *                    output.
			 * @return true if the attribute was found, false otherwise (pOutContent
			 *         will be set to "" if the attribute was not found)
			 */
			bool getAttribute (std::string name, std::string *pOutContent);

			/**
			 * Returns the content of an attribute.
			 * Prints an error if the attribute did not exist.
			 * @param name name of the requested attribute.
			 * @param pOutContent pointer to a string which is used to hold the
			 *                    output.
			 * @param error error text to print if attribute was not found, newline will be added.
			 * @return true if the attribute was found, false otherwise (pOutContent
			 *         will be set to "" if the attribute was not found)
			 */
			bool getAttributeOrErr (std::string name, std::string error, std::string *pOutContent);

			/**
			 * Returns the content of the node.
			 * @return content.
			 */
			std::string getNodeContent (void);

			/**
			 * Replaces all characters in the given text that
			 * are not allowed in XML to their respective
			 * special characters. (e.g. "<", ">", and "&")
			 * @param text to transform.
			 * @return transformed text.
			 */
			static std::string textToXmlText (std::string text);

		protected:
			xmlNodePtr myNode; /**< Lib XML2 node used to represent the node. */
		};
	}/*namespace weaver*/
}/*namespace gti*/

#endif /*SPECIFICATIONNODE_H*/
