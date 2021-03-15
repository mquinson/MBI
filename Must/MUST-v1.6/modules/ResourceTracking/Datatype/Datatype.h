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
 * @file Datatype.h
 *       @see MUST::Datatype.
 *
 *  @date 22.06.2011
 *  @author Joachim Protze
 */

// defines the default maximum printed entries of a typemap 
// used for Datatype::printInfo
#define MAXTYPEMAP 10
#define MAXDOTSIBLINGS 5

#include "ModuleBase.h"
#include "I_Datatype.h"
#include "I_DatatypeTrack.h"
#include "I_BaseConstants.h"
// #include "DatatypeTrackBaseStorage.h"
// #include "DatatypeTrackDerivedStorage.h"
#include "DatatypeDotTree.h"

#include <map>
#include <vector>
#include <string.h>

#ifndef DATATYPE_H
#define DATATYPE_H

using namespace gti;

namespace must
{
    class DatatypeTrack;
    class Datatype;
    class FullBaseTypeInfo;
//     typedef std::list<std::pair<FullBaseTypeInfo*, MustAddressType> > MustTypemap;
    typedef std::list<std::pair<int, FullBaseTypeInfo*> > MustTypesig;
    typedef std::list<std::pair<FullBaseTypeInfo*, MustAddressType> > MustTypemap;

    std::vector<MustAddressType> getAddressVector(MustAddressType base, int offset, MustAddressType stride, size_t n);

    std::vector<MustAddressType> getAddressVector(MustAddressType base, MustAddressType offset, MustAddressType stride, size_t n);

    struct posInfo{
        int index;
        int count;
        std::vector<MustAddressType> pos;
        std::vector<MustAddressType> add;
        const char* name;
        posInfo(int index, int count, std::vector<MustAddressType> e, std::vector<MustAddressType> a, const char* name):index(index), count(count), pos(e), add(a), name(name){}
//         posInfo(int index, int count, MustAddressType e, MustAddressType a, MustAddressType na, MustAddressType ne, const char* name):index(index), count(count), errorpos(getAddressVector(e,ne,MAXDOTSIBLINGS)), erroradd(getAddressVector(a,na,MAXDOTSIBLINGS)), name(name){}
        posInfo(MustAddressType e, MustAddressType a):index(0), count(1), pos(std::vector<MustAddressType>(1,e)), add(std::vector<MustAddressType>(1,a)), name(""){}
    };

    /**
     * Implementation of must::I_Datatype.
     */
    class Datatype : public I_DatatypePersistent, public HandleInfoBase
    {
    protected:



        DatatypeTrack * track;
        MustDatatypeType handle;
        
        MustTypesigType myTypesig;
        bool cachedTypesig;
        
        MustTypesig myLongTypesig;
        bool cachedLongTypesig;
        
        BlockInfo myBlockInfo;
        bool cachedBlocklist;
        
        MustTypemapType myTypemap;
        bool cachedTypemap;
        
        int maxNoSelfoverlap;
        int minSelfoverlap;

        bool myIsNull;
        bool myIsPredefined;
        bool myIsCommited;
        bool myIsC;
        bool myIsFortran;

        bool myHasExplicitLb;
        bool myHasExplicitUb;

        MustAddressType lb;         // Lower bound of the Datatype, derived from parents
        MustAddressType extent;     // Extent of the Datatype, derived from parents
        MustAddressType true_lb;         // Lower bound of the Datatype, derived from parents
        MustAddressType true_extent;     // Extent of the Datatype, derived from parents
        MustAddressType size;       // Size of the Datatype, derived from parents
        int alignment;              // max(Alignment) of used Basetypes
        int epsilon;                // the epsilon that is added to fill the datatype to alignment

        MustParallelId creationPId;
        MustLocationId creationLId;

        MustParallelId commitPId;
        MustLocationId commitLId;

        std::vector<Datatype *> parentInfos;

        std::string getResourceName (void); /**< @see HandleInfoBase::getResourceName.*/
        virtual const char* kindName(void)=0;
        virtual std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)=0;
        std::vector<struct posInfo> posToPath(MustAddressType& errorpos);

    public:

        virtual ~Datatype(void); /**< Destructor .*/
        Datatype(
                MustParallelId creationPId,
                MustLocationId creationLId,
                std::vector<Datatype *> oldInfo,
                DatatypeTrack * track);
        Datatype();

        void commit(MustParallelId pId, MustLocationId lId);

        MustAddressType getLb(void) const;
        MustAddressType getUb(void) const;
        MustAddressType getExtent(void) const;
        MustAddressType getTrueLb(void) const;
        MustAddressType getTrueUb(void) const;
        MustAddressType getTrueExtent(void) const;
        MustAddressType getSize(void) const;
        int getEpsilon(void) const;
        int getAlignment(void) const;
        
        virtual MustAddressType checkAlignment(void) const {return 0;}

       /**
        * Basic information for a datatype handle.
        */
        bool isNull(void) const; /**< True if this is MPI_DATATYPE_NULL, isKnown=true in that case, the other pieces of information are not set. */

        //(Only!) For predefined handles
        bool isPredefined(void) const; /**< True if this type is predefined and not MPI_DATATYPE_NULL.*/
        bool isOptional(void) const; /**< True if this is a predefined optional type.*/
        bool isForReduction(void) const; /**< True if this type is only for special reductions (MPI_MINLOC, MPI_MAXLOC).*/
        bool isBoundMarker(void) const; /**< True if this is an MPI_UB or MPI_LB marker.*/

        //(Only!) For user handles (isNull==false && isPredefined==false)
        bool isCommited(void) const; /**< True if the type is committed.*/

        //For both predefined and user handles (isNull == false)
        bool isC(void) const; /**< True if this is a C type, note some predefined types are both for C and Fortran!.*/
        bool isFortran(void) const; /**< True if this is a Fortran type.*/

        bool hasExplicitLb(void) const; /**< True if typemap contains MPI_LB.*/
        bool hasExplicitUb(void) const; /**< True if typemap contains MPI_UB.*/

        /**
        * Information on where a derived datatype was created and commited.
        *
        * Used to print details for the locations that created or commited a
        * derived datatype.
        */
        MustParallelId getCreationPId(void) const; /**< Information for call that created the datatype.*/
        MustLocationId getCreationLId(void) const; /**< Information for call that created the datatype.*/

        MustParallelId getCommitPId(void) const; /**< Information for call that commited the datatype.*/
        MustLocationId getCommitLId(void) const; /**< Information for call that commited the datatype.*/

        /**
         * @see I_Datatype::getPredefinedInfo
         */
        MustMpiDatatypePredefined getPredefinedInfo (void);

        /**
         * @see I_Datatype::getPredefinedName.
         */
        virtual std::string getPredefinedName (void);

        /**
         * @see I_Datatype::getReferencedTypes
         */
        std::list<I_Datatype*> getReferencedTypes();

        /**
         * @see I_Datatype::printInfo
         */
        bool printInfo (
                std::stringstream &out,
                std::list<std::pair<MustParallelId, MustLocationId> > *pReferences);

        /**
         * @see I_Datatype::isSubsetOfB
         */
        MustMessageIdNames isSubsetOfB (
                int countA,
                I_Datatype * typeB,
                int countB,
                MustAddressType* errorpos
                );

        /**
         * @see I_Datatype::isEqualB
         */
        MustMessageIdNames isEqualB (
                int countA,
                I_Datatype * typeB,
                int countB,
                MustAddressType* errorpos
                );

        /**
         * @see I_Datatype::getBlockList
         * @param err returns errorcode if something goes wrong
         */
        BlockInfo& getBlockInfo ();

        /**
         * @see I_Datatype::getTypemap
         * @param err returns errorcode if something goes wrong
         */
        const MustTypemapType& getTypemap();
        const MustTypemapType& getTypemap(int* err);

        /**
         * @see I_Datatype::getTypesig
         * @param err returns errorcode if something goes wrong
         */
        const MustTypesigType& getTypesig();
        /** variant of Datatype::getTypesig for internal use */
        const MustTypesigType& getTypesig( const MustTypesigType * const );
        /** variant of Datatype::getTypesig for internal use */
        const MustTypesigType& getTypesig( const MustTypesigType * const , int* err);
        /** variant of Datatype::getTypesig for internal use */
        const MustTypesig& getTypesig( const MustTypesig * const );
        /** variant of Datatype::getTypesig for internal use */
        const MustTypesig& getTypesig( const MustTypesig * const , int* err);

        /**
         * @see I_Datatype::printTypemapString
         * @param maxentries gives the maximum of entries should be printed out
         */
        bool printTypemapString(std::ostream &out);
        bool printTypemapString(std::ostream &out, int maxentries);

        /**
         * @see I_Datatype::printDatatypePos
         */
        virtual bool printRealDatatypePos(std::ostream &out, MustAddressType errorpos);
        virtual bool printRealDatatypeLongPos(std::ostream &out, MustAddressType errorpos);
        bool printDatatypePos(std::ostream &out, MustAddressType errorpos);
        bool printDatatypeLongPos(std::ostream &out, MustAddressType errorpos);
        /**
         * @see I_Datatype::printDatatypeDotTree
         */
//         bool printDatatypeDotTree(std::ostream &out, MustAddressType errorpos, std::string callNode);
        bool printDatatypeDotOverlap(std::ostream &out, MustAddressType errorposA, MustAddressType addressA, std::string callNodeA, I_Datatype * typeB, MustAddressType errorposB, MustAddressType addressB, std::string callNodeB);
        bool printDatatypeDotTypemismatch(std::ostream &out, MustAddressType errorpos, std::string callNodeA, I_Datatype * typeB, std::string callNodeB);
//         virtual bool fillDotTree(DatatypeForest& f, DatatypeDotNode*& retNode, MustAddressType errorpos, int type);
        virtual bool fillOverlapTree(DatatypeForest& f, DatatypeDotNode*& retNode, MustAddressType errorpos,  MustAddressType address, int& level, int type);
        virtual bool fillTypemismatchTree(DatatypeForest& f, DatatypeDotNode*& retNode, MustAddressType errorpos, MustAddressType pos,   int& level, int type);
        bool fillUpLeafs( DatatypeForest& f, DatatypeDotNode* pNode, MustAddressType errorpos, MustAddressType pos, int& leafs,    int type);

        
        std::pair<int,int> getSelfoverlapCache(void);
        void setMaxNoOverlap(int nooverlap);
        void setMinOverlap(int overlap);

        /**
         * Passes the datatype to the given tool node on this TBON layer.
         * Important: all base resources of the datatype (dependent types,
         * and creation/commit location) need to be passed to the target
         * node before calling this function!
         * @param rank of resource owner.
         * @param hasHandle true if this datatype still has an associated handle.
         * @param handle associated with this datatype (superfluous if none present).
         * @param toPlaceId node id of target node on this tool layer.
         * @return true iff successful.
         */
        virtual bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId) = 0;

    protected:
        /**
         * Fixes the alignment of the datatype by adding epsilon to extent (see mpi-standard)
         * 
         * @return None
         */
        void epsilonMagic();

        /**
         * strips bound markers from typemap
         * @param typemap to be stripped
         * @param outLb returns position of lowest LB-marker
         * @param outUb returns position of highest UB-marker
         * @return None
         */
        void stripBoundmarkersFromTypemap(MustTypemapType &typemap, MustAddressType &outLb, MustAddressType &outUb);

        /**
        * Returns a list of <*Start and End*> of typeblocks
        * @return blocklist, single entry with Location=0 if this is a base type.
        */
        virtual void getRealBlockInfo()=0;

        /**
        * Returns a list of <*int, basetype*>
        * @return typesig, single entry with length=1 if this is a base type.
        */
//         virtual MustTypesigType getRealTypesig(int* err)=0;
        virtual void getRealTypesig(MustTypesig * typesig, int * err)=0;
        virtual void getRealTypesig(MustTypesigType * typesig, int * err)=0;

        /**
        * Returns a list of <*Locations and Basetypes*>
        * Can have multiple bound markers
        * @param track pointer to the datatypetrack, to get info for parenttypes
        * @param err returns errorcode if something goes wrong
        * @return typemap, single entry with Location=0 if this is a base type.
        */
        virtual MustTypemapType getFullTypemap(int* err)=0;

        /**
         * Helper for Typematching, special handling for MPI_BYTE
         */
        MustMessageIdNames handleMpiByte (
                const MustTypesig& typesigA,
                MustTypesig::const_iterator& iterA,
                int& iA,
                int countA,
                const MustTypesig& typesigB,
                MustTypesig::const_iterator& iterB,
                int& iB,
                int countB,
                MustAddressType* errorpos
                );

        /**
         * Helper for Typematching, implements typematching
         */
        MustMessageIdNames checkWhetherSubsetOfB (
                int countA,
                I_Datatype* typeB,
                int countB,
                MustAddressType* errorpos
                );

    }; /*class Datatype */

    /**
     * Information for a basic type, e.g. MPI_INT.
     */
    class FullBaseTypeInfo : public Datatype
    {
        protected:
            void getRealBlockInfo();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
            const char* kindName(void){return "NATIVE";}
            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);

        public:
            bool isOptional;
            bool isForReduction;
            bool isBoundMarker;
            MustMpiDatatypePredefined predefValue; /**< The enum value describing which predefined it is.*/
            std::string predefName; /**< Name of the predefined. */
            passDatatypePredefinedAcrossP myPassAcrossFunc;

            FullBaseTypeInfo(
                    DatatypeTrack *track,
                    bool isOptional,
                    bool isForReduction,
                    bool isBoundMarker,
                    bool isNull,
                    bool isC,
                    bool isFortran,
                    bool hasExplicitLb,
                    bool hasExplicitUb,
                    MustMpiDatatypePredefined predefValue,
                    const char* predefName,
                    passDatatypePredefinedAcrossP passAcrossFunc);
            MustTypemapType getFullTypemap(int* err);
            void setSizes(MustAddressType extent, int alignment);
            std::string getPredefinedName();
            bool printRealDatatypePos(std::ostream &out, MustAddressType errorpos);
            bool printRealDatatypeLongPos(std::ostream &out, MustAddressType errorpos);
            bool fillOverlapTree(
                DatatypeForest& f, 
                DatatypeDotNode*& retNode, 
                MustAddressType errorpos, 
                MustAddressType address, 
                int& level,
                int type
            );

            bool fillTypemismatchTree(
                DatatypeForest& f, 
                DatatypeDotNode*& retNode, 
                MustAddressType errorpos, 
                MustAddressType pos,
                int& level,
                int type
            );

            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/
    };

} /*namespace MUST*/

#endif /*DATATYPE_H*/
