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
 * @file StridedBlock.h
 *       @see StridedBlock.
 *
 *  @date 17.08.2011
 *  @author Joachim Protze
 */

#include <set>
#include <list>
#include <stdint.h>
#include <cstdlib>
#include "MustTypes.h"

#ifndef STRIDEDBLOCKS_H
#define STRIDEDBLOCKS_H

namespace must
{
    class I_Datatype;

       
    struct StridedBlock
    {
        MustAddressType first; // begin
        MustAddressType second; // end
        MustAddressType pos; // byte position within the serialized datatype
        bool merged; // means the block entry was merged non contiguous (either overlapping or non-chronological)
        int repetition;
        int count;
        MustAddressType blocksize;
        MustAddressType stride;
        bool operator<(const StridedBlock& r) const;
        
        // constructors
        // copyconst
        StridedBlock(const StridedBlock& o) : first(o.first), second(o.second), pos(o.pos), merged(o.merged), repetition(o.repetition), count(o.count), blocksize(o.blocksize), stride(o.stride){}
        
        StridedBlock(const MustAddressType& l) : first(0), second(l), pos(0), merged(false), repetition(0), count(1), blocksize(l), stride(0) {}
        
        StridedBlock(const MustAddressType& b, const MustAddressType& p, const bool& m, const int& r, const int& c, const MustAddressType& block, const MustAddressType& stride) : first(b), second(b+(c-1)*stride+block), pos(p), merged(m), repetition(r), count(c), blocksize(block), stride(stride){}
        
        StridedBlock(const StridedBlock& o, const MustAddressType& offset, const int& rep) : first(o.first+offset), second(o.second+offset), pos(o.pos), merged(o.merged), repetition(rep), count(o.count), blocksize(o.blocksize), stride(o.stride){}

        StridedBlock(const StridedBlock& o, const MustAddressType& offset, const MustAddressType& posoffset, const int& rep) : first(o.first+offset), second(o.second+offset), pos(o.pos+posoffset), merged(o.merged), repetition(rep), count(o.count), blocksize(o.blocksize), stride(o.stride){}
        
        /* StridedBlock(const MustAddressType& b, const MustAddressType& e, const MustAddressType& p, const int& c, const MustAddressType& block, const MustAddressType& stride) : first(b), second(e), pos(p), merged(false), repetition(0), count(c), blocksize(block), stride(stride) {}

        StridedBlock(const MustAddressType& b, const MustAddressType& e, const MustAddressType& p, const bool& m, const int& c, const MustAddressType& block, const MustAddressType& stride) : first(b), second(e), pos(p), merged(m), repetition(0), count(c), blocksize(block), stride(stride) {}
        
        StridedBlock(const StridedBlock& o, const MustAddressType& b, const MustAddressType& e, const MustAddressType& p, const int& r) : first(b), second(e), pos(p), merged(o.merged), repetition(r), count(o.count), blocksize(o.blocksize), stride(o.stride){}*/
        bool overlapPos(MustAddressType b, MustAddressType e, MustAddressType * posB) const;
        
        bool overlaps(MustAddressType b, MustAddressType e) const;
        
        bool overlapPos(const StridedBlock& o, MustAddressType& posA, MustAddressType& posB) const;
        bool overlaps(const StridedBlock & r) const;
    };
    
    struct memInterval : StridedBlock
    {
        MustRequestType request;
        bool isSend;
        I_Datatype * type;
        MustAddressType baseAddress;
        
        // copyconst
        memInterval(const memInterval& o) : StridedBlock(o.first, o.pos, o.merged, o.repetition, o.count, o.blocksize, o.stride), request(o.request), isSend(o.isSend), type(o.type), baseAddress(o.baseAddress) {}
        
        memInterval(const StridedBlock& sBlock, const MustAddressType& offset, const MustRequestType& r, const bool& s, I_Datatype *const &t, const MustAddressType& baseAddress, const int& rep) : StridedBlock(sBlock, offset, rep), request(r), isSend(s), type(t), baseAddress(baseAddress){}
        
        memInterval(const StridedBlock& sBlock, const MustAddressType& offset, const MustAddressType& posoffset, const MustRequestType& r, const bool& s, I_Datatype *const &t, const MustAddressType& baseAddress, const int& rep) : StridedBlock(sBlock, offset, posoffset, rep), request(r), isSend(s), type(t), baseAddress(baseAddress) {}
        
    };
    
    template <typename T> 
    class multimapwrapper
    {
    protected:
        std::multiset<T> set;
    public:
        multimapwrapper():set(){};
        typedef typename std::multiset<T>::iterator iterator;
        typedef typename std::multiset<T>::const_iterator const_iterator;
        iterator begin (){return set.begin();}
        iterator end (){return set.end();}
        const_iterator begin () const{return set.begin();}
        const_iterator end () const{return set.end();}
        size_t size() const {return set.size();}
        bool empty () const {return set.empty();}
        void clear() {set.clear();}
        iterator insert ( const T& x ) {return set.insert(x);}
        iterator insert ( iterator position, const T& x ){return set.insert(position, x);}
    template <class InputIterator>
        void insert ( InputIterator first, InputIterator last ){set.insert(first, last);}
        void erase ( iterator position ){set.erase(position);}
        size_t erase ( const T& x ){return set.erase(x);}
        void erase ( iterator first, iterator last ){set.erase(first, last);}
    };
    
    struct MustMemIntervalListType : public multimapwrapper<memInterval>
    {
        bool overlapped;
        MustAddressType posA;
        MustAddressType posB;
    };


    
    /**
    * Datatype used for Blocklists.
    */
    struct MustStridedBlocklistType : public multimapwrapper<StridedBlock>
    {
        bool overlapped;
        MustAddressType posA;
        MustAddressType posB;
        void checkOverlapped();
    };

    /**
    * create a blocklist by replication
    */
    MustStridedBlocklistType buildStridedBlocklist(const MustStridedBlocklistType& blocklist, MustAddressType extent, MustAddressType size, MustAddressType offset, MustAddressType posOffset, MustAddressType blocklength, MustAddressType stride, MustAddressType count);

    MustMemIntervalListType buildMemIntervallist(const MustStridedBlocklistType& blocklist, const MustAddressType& extent, const MustAddressType& size, const MustAddressType& offset, const MustRequestType& request, const bool& isSend, I_Datatype *const& type, const int& count, const MustAddressType& baseAddress);

//     /**
//     * Blocklist Info package for return values.
//     */
//     struct BlockInfo
//     {
//         MustStridedBlocklistType stridedBlocklist;
//         bool overlapped;
//         MustAddressType posA;
//         MustAddressType posB;
//     };
    typedef MustStridedBlocklistType BlockInfo;
    // functions to check lists for overlap
    
    
//     bool isOverlapped(const MustStridedBlocklistType& blocklist, MustStridedBlocklistType::iterator& iter, MustStridedBlocklistType::iterator& nextIter);
    
    bool isOverlapped(const MustStridedBlocklistType& blocklist, MustStridedBlocklistType::iterator& iter, MustStridedBlocklistType::iterator& nextIter, MustAddressType& posA, MustAddressType& posB);

//     bool isOverlapped(const MustMemIntervalListType& blocklist, MustMemIntervalListType::iterator& iter, MustMemIntervalListType::iterator& nextIter, const bool& ignoreSend);

    bool isOverlapped(const MustMemIntervalListType& blocklist, MustMemIntervalListType::iterator& iter, MustMemIntervalListType::iterator& nextIter, MustAddressType& posA, MustAddressType& posB, const bool& ignoreSend, bool ignoreSameRequest=false);

//     bool isOverlapped(const MustMemIntervalListType& listA, const MustMemIntervalListType& listB, MustMemIntervalListType::iterator& iter, MustMemIntervalListType::iterator& nextIter, const bool& ignoreSend);

    bool isOverlapped(const MustMemIntervalListType& listA, const MustMemIntervalListType& listB, MustMemIntervalListType::iterator& iter, MustMemIntervalListType::iterator& nextIter, MustAddressType& posA, MustAddressType& posB, const bool& ignoreSend, bool ignoreSameRequest=false);
    
    
    
} // namespace must
#endif /*STRIDEDBLOCKS_H*/


