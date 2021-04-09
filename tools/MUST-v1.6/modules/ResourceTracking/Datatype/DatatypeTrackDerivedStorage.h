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
 * @file DatatypeTrackDerivedStorage.h
 *       @see MUST::DatatypeTrack.
 *
 *  @date 10.02.2011
 *  @author Tobias Hilbrich, Joachim Protze
 */

#include "ModuleBase.h"
#include "I_DatatypeTrack.h"
#include "TrackBase.h"
#include "Datatype.h"

#include <map>
#include <string.h>
#include <utility>
#include <algorithm>

#ifndef DATATYPETRACKDERIVEDSTORAGE_H
#define DATATYPETRACKDERIVEDSTORAGE_H

#define MUST_ADDR_INFTY (((MustAddressType)1 << (sizeof(MustAddressType)*8-1)) ^ -1)
#define MUST_ADDR_NEG_INFTY (((MustAddressType)1 << (sizeof(MustAddressType)*8-1)))

using namespace gti;

namespace must
{
    /**
     * Information for a contiguous type.
     */
    class FullContiguousTypeInfo : public Datatype
    {
        public:
            int count;

            FullContiguousTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int count,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeContiguousAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/

        protected:
            passDatatypeContiguousAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "contiguous";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a vector type.
     */
    class FullVectorTypeInfo : public Datatype
    {
        public:
            int count;
            int blocklength;
            int stride;
            FullVectorTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int count,
                    int blocklength,
                    int stride,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeVectorAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/

        protected:
            passDatatypeVectorAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "vector";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a hvector type.
     */
    class FullHVectorTypeInfo : public Datatype
    {
        public:
            int count;
            int blocklength;
            MustAddressType stride;
            FullHVectorTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int count,
                    int blocklength,
                    MustAddressType stride,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeHvectorAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/
            MustAddressType checkAlignment(void) const;

        protected:
            passDatatypeHvectorAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "hvector";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
            
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a indexed type.
     */
    class FullIndexedTypeInfo : public Datatype
    {
        public:
            int count;
            int* arrayOfBlocklengths;
            int* arrayOfDisplacements;
            ~FullIndexedTypeInfo(void); /**< Destructor.*/
            FullIndexedTypeInfo(void); /**< Constructor.*/
            FullIndexedTypeInfo(FullIndexedTypeInfo& info);  /**< Copy constructor.*/
            FullIndexedTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int count,
                    const int* blocklengths,
                    const int* displacements,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeIndexedAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/

        protected:
            passDatatypeIndexedAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "indexed";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a hindexed type.
     */
    class FullHIndexedTypeInfo : public Datatype
    {
        public:
            int count;
            int* arrayOfBlocklengths;
            MustAddressType* arrayOfDisplacements;
            ~FullHIndexedTypeInfo(void); /**< Destructor.*/
            FullHIndexedTypeInfo(void); /**< Constructor.*/
            FullHIndexedTypeInfo(FullHIndexedTypeInfo& info);  /**< Copy constructor.*/
            FullHIndexedTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int count,
                    const int* blocklengths,
                    const MustAddressType* displacements,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeHindexedAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/
            MustAddressType checkAlignment(void) const;

        protected:
            passDatatypeHindexedAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "hindexed";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a struct type.
     */
    class FullStructTypeInfo : public Datatype
    {
        public:
            int count;
            int* arrayOfBlocklengths;
            MustAddressType* arrayOfDisplacements;
            ~FullStructTypeInfo(void); /**< Destructor.*/
            FullStructTypeInfo(void); /**< Constructor.*/
            FullStructTypeInfo(FullStructTypeInfo& info); /**< Copy constructor.*/
            FullStructTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int count,
                    const int* blocklengths,
                    const MustAddressType* displacements,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeStructAcrossP passAcross
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/
            MustAddressType checkAlignment(void) const;

        protected:
            passDatatypeStructAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "struct";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a indexed-block type.
     */
    class FullIndexedBlockTypeInfo : public Datatype
    {
        public:
            int count;
            int blocklength;
            int* arrayOfDisplacements;
            ~FullIndexedBlockTypeInfo(void); /**< Destructor.*/
            FullIndexedBlockTypeInfo(void); /**< Constructor.*/
            FullIndexedBlockTypeInfo(FullIndexedBlockTypeInfo& info); /**< Copy Constructor.*/
            FullIndexedBlockTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int count,
                    int blocklength,
                    const int* displacements,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeIndexedBlockAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/

        protected:
            passDatatypeIndexedBlockAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "indexed_block";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a reduced type.
     */
    class FullResizedTypeInfo : public Datatype
    {
        public:
            FullResizedTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    MustAddressType lb,
                    MustAddressType extent,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeResizedAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/

        protected:
            passDatatypeResizedAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "resized";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a subarray type.
     */
    class FullSubarrayTypeInfo : public Datatype
    {
        public:
            int ndims;
            int* arrayOfSizes;
            int* arrayOfSubsizes;
            int* arrayOfStarts;
            int order;
            ~FullSubarrayTypeInfo(void); /**< Destructor.*/
            FullSubarrayTypeInfo(void); /**< Constructor.*/
            FullSubarrayTypeInfo(FullSubarrayTypeInfo& info); /**< Copy Constructor.*/
            FullSubarrayTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int ndims,
                    const int* sizes,
                    const int* subsizes,
                    const int* starts,
                    int order,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeSubarrayAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/

        protected:
            passDatatypeSubarrayAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "subarray";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

    /**
     * Information for a darray type.
     */
    class FullDarrayTypeInfo : public Datatype
    {
        public:
            int commSize;
            int rank;
            int ndims;
            int* arrayOfGsizes;
            int* arrayOfDistribs;
            int* arrayOfDargs;
            int* arrayOfPsizes;
            int order;

            ~FullDarrayTypeInfo(void); /**< Destructor.*/
            FullDarrayTypeInfo(void); /**< Constructor.*/
            FullDarrayTypeInfo(FullDarrayTypeInfo& info); /**< Copy Constructor.*/
            FullDarrayTypeInfo(
                    DatatypeTrack *track,
                    MustParallelId pId,
                    MustLocationId lId,
                    int commSize,
                    int rank,
                    int ndims,
                    const int* gsizes,
                    const int* distribs,
                    const int* dargs,
                    const int* psizes,
                    int order,
                    std::vector<Datatype *> oldInfos,
                    passDatatypeDarrayAcrossP passAcrossFunc
            );
            bool passAcross (int rank, bool hasHandle, MustDatatypeType handle, int toPlaceId); /**< @see I_Datatype::passAcross.*/

        protected:
            passDatatypeDarrayAcrossP myPassAcrossFunc;

            std::vector<struct posInfo> posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add);
            const char* kindName(void){return "darray";}
            MustTypemapType getFullTypemap(int* err);
            void getRealBlockInfo ();
            void getRealTypesig(MustTypesig * typesig, int * err);
            void getRealTypesig(MustTypesigType * typesig, int * err);
/*            template<class T>
            void getRealTypesig(T* typesig, int* err);*/
    };

//     #include "DatatypeTrackDerivedStorage.hpp"

} /*namespace MUST*/

#endif /*DATATYPETRACKDERIVEDSTORAGE_H*/
