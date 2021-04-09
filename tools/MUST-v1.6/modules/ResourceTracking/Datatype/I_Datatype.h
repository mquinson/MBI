/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file I_Datatype.h
 *       @see I_Datatype.
 *
 *  @date 22.06.2011
 *  @author Joachim Protze
 */

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"

#include <list>
#include <string>
#include <iostream>
#include "I_Destructable.h"
#include "StridedBlock.h"

#ifndef I_DATATYPE_H
#define I_DATATYPE_H

/**
 * Datatype used for Typemaps.
 */
  
typedef std::list<std::pair<MustMpiDatatypePredefined, MustAddressType> > MustTypemapType;
  
/**
 * Datatype used for Typesigs.
 */
    
    typedef std::list<std::pair<int, MustMpiDatatypePredefined> > MustTypesigType;
    
       

namespace must
{
    /**
     * Interface for storage and accessing Information 
     * on a datatype as defined in MPI.
     */
    class I_Datatype
    {
    public:

        /**
        * Basic information for a datatype handle.
        */
        virtual bool isNull(void) const=0; /**< True if this is MPI_DATATYPE_NULL, isKnown=true in that case, the other pieces of information are not set. */

        //(Only!) For predefined handles
        virtual bool isPredefined(void) const=0; /**< True if this type is predefined and not MPI_DATATYPE_NULL.*/
        virtual bool isOptional(void) const=0; /**< True if this is a predefined optional type.*/
        virtual bool isForReduction(void) const=0; /**< True if this type is only for special reductions (MPI_MINLOC, MPI_MAXLOC).*/
        virtual bool isBoundMarker(void) const=0; /**< True if this is an MPI_UB or MPI_LB marker.*/

        //(Only!) For user handles (isNull==false && isPredefined==false)
        virtual bool isCommited(void) const=0; /**< True if the type is committed.*/

        //For both predefined and user handles (isNull == false)
        virtual bool isC(void) const=0; /**< True if this is a C type, note some predefined types are both for C and Fortran!.*/
        virtual bool isFortran(void) const=0; /**< True if this is a Fortran type.*/

        /**
        * Information on where a derived datatype was created and commited.
        *
        * Used to print details for the locations that created or commited a
        * derived datatype.
        */
        virtual MustParallelId getCreationPId(void) const=0; /**< Information for call that created the datatype.*/
        virtual MustLocationId getCreationLId(void) const=0; /**< Information for call that created the datatype.*/

        virtual MustParallelId getCommitPId(void) const=0; /**< Information for call that commited the datatype.*/
        virtual MustLocationId getCommitLId(void) const=0; /**< Information for call that commited the datatype.*/

        virtual MustAddressType getLb(void) const=0;/**< Lower bound of the datatype.*/
        virtual MustAddressType getUb(void) const=0;/**< Upper bound of the datatype.*/
        virtual MustAddressType getExtent(void) const=0;/**< Extent of the datatype.*/
        virtual MustAddressType getTrueLb(void) const=0;/**< True lower bound of the datatype.*/
        virtual MustAddressType getTrueUb(void) const=0;/**< True upper bound of the datatype.*/
        virtual MustAddressType getTrueExtent(void) const=0;/**< True extent of the datatype.*/
        virtual MustAddressType getSize(void) const=0;/**< Size of the datatype.*/

        virtual MustAddressType checkAlignment(void) const=0;/**< Max alignment of a datatype.*/

        /**
         * For datatypes that are predefined and not MPI_DATATYPE_NULL,
         * returns an enumeration that identifies the name of the
         * predefined datatype.
         */
        virtual MustMpiDatatypePredefined getPredefinedInfo (void) = 0;

        /**
         * If this is a predefined handle, returns the textual name of the
         * predefined MPI handle it represents.
         * @return name of handle.
         */
        virtual std::string getPredefinedName (void) = 0;

        /**
         * Returns the list of types on which this datatype is based.
         * Only returns the parents, not all ancestors!
         * @return list of parent types, empty if this is a base type.
         */
        /**
         * Returns a list of datatype handles that were used as base types
         * during construction of this datatype.
         *
         * @return list of references.
         */
        virtual std::list<I_Datatype*> getReferencedTypes (void) = 0;

        /**
         * Prints information for a specified datatype.
         * Designed for printing in a style that suits the usage
         * of CreateMessage.
         *
         * @param out stream to use for output.
         * @param pReferences current references to which any additional references for the new handle will be added.
         * @return true if successful.
         */
        virtual bool printInfo (
                std::stringstream &out,
                std::list<std::pair<MustParallelId,MustLocationId> > *pReferences) = 0;

        /**
         * Compares the typesig of type A with type B
         * @param countA repetition of this type.
         * @param typeA datatype B.
         * @param countB repetition of datatype B.
         * @param errorpos (out) returns the position of the mismatch
         * @return a message id, that can be used for generating the errormessage
         */
        virtual MustMessageIdNames isSubsetOfB (
                int countA,
                I_Datatype * typeB,
                int countB,
                MustAddressType* errorpos
                ) = 0;

        /**
         * Compares the typesig of type with type B
         * @param countA repetition of this type.
         * @param typeA datatype B.
         * @param countB repetition of datatype B.
         * @param errorpos (out) returns the position of the mismatch
         * @return a message id, that can be used for generating the errormessage
         */
        virtual MustMessageIdNames isEqualB (
                int countA,
                I_Datatype * typeB,
                int countB,
                MustAddressType* errorpos
                ) = 0;

        /**
         * Returns the typemap of a derived datatype as defined in MPI standard.
         * @param out 
         * @return successful?
         */
        virtual bool printTypemapString (std::ostream &out) = 0;

        /**
         * Prints the path to a position in the datatype tree.
         * @param out ("out") stream to print the output to
         * @param errorpos (in) position within the datatype that should be described
         * @return successful?
         */
        virtual bool printDatatypePos(std::ostream &out, MustAddressType errorpos) = 0;
        virtual bool printDatatypeLongPos(std::ostream &out, MustAddressType errorpos) = 0;

        /**
         * Prints the path to a position in the datatype tree in dotstyle.
         * @param out ("out") stream to print the output to
         * @param errorpos (in) position within the datatype that should be described
         * @return successful?
         */
//         virtual bool printDatatypeDotTree(std::ostream &out, MustAddressType errorpos) = 0;
        virtual bool printDatatypeDotOverlap(std::ostream &out, MustAddressType errorposA, MustAddressType addressA, std::string callNodeA, I_Datatype * typeB, MustAddressType errorposB, MustAddressType addressB, std::string callNodeB) = 0;
        virtual bool printDatatypeDotTypemismatch(std::ostream &out, MustAddressType errorpos, std::string callNodeA, I_Datatype * typeB, std::string callNodeB) = 0;


        /**
         * Returns the typemap of a derived datatype as defined in MPI standard.
         * @return typemap as list.
         */
        virtual const MustTypemapType& getTypemap (void) = 0;

        /**
         * Returns the intervalls blocked by a derived datatype.
         * It's garantied that the list is sorted by first entry
         * @return intervalls as list\<first, last\>.
         */
//         virtual MustBlocklistType& getBlockList (bool& overlap) = 0;
        virtual BlockInfo& getBlockInfo () = 0;

        /**
        * Returns a list of <*Repetitions and Basetypes*>
        * It's garantied that neighboring entries have different basetypes
        * In the MPI-Standard known as typesig
        * @return typesig, single entry with Location=0 if this is a base type.
        */
        virtual const MustTypesigType& getTypesig (void) = 0;

        virtual std::pair<int,int> getSelfoverlapCache(void) = 0;
        virtual void setMaxNoOverlap(int nooverlap) = 0;
        virtual void setMinOverlap(int overlap) = 0;


    };/*class I_Datatype*/

    /**
     * Interface for storage and accessing Information
     * on a datatype as defined in MPI. This is the persistent
     * version of the interface. The user needs to call I_DatatypePersitent::erase
     * when he is finished with the persistent interface.
     */
    class I_DatatypePersistent : public I_Datatype, public virtual I_Destructable
    {
    };/*class I_DatatypePersistent*/

}/*namespace must*/
#endif
