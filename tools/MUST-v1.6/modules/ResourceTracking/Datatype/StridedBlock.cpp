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
 * @file StridedBlock.cpp
 *       @see MUST::Datatype.
 *
 *  @date 17.08.2011
 *  @author Joachim Protze
 */

#include "StridedBlock.h"


using namespace must;


// for sorting a blocklist
bool StridedBlock::operator<(const StridedBlock& r) const
{return first < r.first;}

bool StridedBlock::overlaps(MustAddressType b, MustAddressType e) const
{
    // block falls into the region?
    if (b >= second || first >= e)
        return false;
    // compact block?
    if (count == 1)
        return true;
    // begin of block is left of the region?
    if (b < first)
        return true;
    // begin of block lays in i-th stride-interval.
    int i = (b - first) / stride;
    // begin of block lays within the i-th strided block?
    if ( b < first + i * stride + blocksize )
        return true;
    // end of block lays in the next stride-interval?
    if ( e > first + ( 1 + i ) * stride )
        return true;
    // else
    return false;
}

bool StridedBlock::overlapPos(MustAddressType b, MustAddressType e, MustAddressType * posB) const
{
    // block falls into the region?
    if (b >= second || first >= e)
        return false;
    // compact block?
    if (count == 1)
    {
        if (b < first)
                *posB = pos;
        else
                *posB = pos + b - first;
        return true;
    }
    // begin of block is left of the region?
    if (b < first)
    {
        *posB = pos;
        return true;
    }
    // begin of block lays in i-th stride-interval.
    int i = (b - first) / stride;
    // begin of block lays within the i-th strided block?
    if ( b < first + i * stride + blocksize )
    {
        *posB = pos + i * blocksize + b - first - i * stride;
        return true;
    }
    // end of block lays in the next stride-interval?
    if ( e > first + ( 1 + i ) * stride )
    {
        *posB = pos + ( 1 + i ) * blocksize;
        return true;
    }
    // else
    return false;
}

#define max(a,b) ((a>b)?a:b)
#define min(a,b) ((a<b)?a:b)
#define dist(a,b) ((a<b)?b-a:a-b)
#define abs(a) ((a>0)?a:-a)

bool StridedBlock::overlaps(const StridedBlock& o) const
{
    if (first >= o.second || o.first >= second)
        return false;
    const StridedBlock *left, *right;
    // recognize the left one
    if (first < o.first)
        left = this, right = &o;
    else
        right = this, left = &o;
    // collision with first block of the right one?
    if (left->overlaps(right->first, right->first + right->blocksize))
        return true;
    if (stride == o.stride)
        return false;
    // abs ( stride - o.stride )
    MustAddressType deltaStride = left->stride - right->stride,
                    absDeltaStride = abs(deltaStride),
                    overlapLength = min(second, o.second) - max(first, o.first);
    int deltaCount = overlapLength / max(stride, o.stride);
    // overlap on first interception -- is interception within the overlapping region?
    if (absDeltaStride < max(blocksize, o.blocksize))
    {
        if ( absDeltaStride * overlapLength > stride * o.stride )
            return true;
        // 
        if ( deltaStride < 0 )
        {
            // first block of left type within right region
            MustAddressType firstLeftBlock = left->first + (MustAddressType)((right->first - left->first) / left->stride + 1) * left->stride;
            if ((firstLeftBlock - (right->first + right->blocksize)) / absDeltaStride < deltaCount)
                return true;
        }
        else
        {
            // last block of left type before right region
            MustAddressType lastLeftBlock = left->first + (MustAddressType)((right->first - left->first) / left->stride) * left->stride + left->blocksize;
            if ((right->first - lastLeftBlock ) / absDeltaStride <= deltaCount+1)
                return true;
        }
        return false;
    }
    // here we go O(min(m,n))
    // loop over the bigger stride
    if ( deltaStride < 0 )
    {
        for (MustAddressType blockBegin=right->first; blockBegin<min(second, o.second); blockBegin+=right->stride)
        {
            if(left->overlaps(blockBegin, blockBegin + right->blocksize))
                return true;
        }
    }
    else
    {
        for (MustAddressType blockBegin=left->first + (MustAddressType)((right->first - left->first) / left->stride + 1) * left->stride; blockBegin<min(second, o.second); blockBegin+=left->stride)
        {
            if(right->overlaps(blockBegin, blockBegin + left->blocksize))
                return true;
        }
    }
    return false;        
}

bool StridedBlock::overlapPos(const StridedBlock& o, MustAddressType& posA, MustAddressType& posB) const
{
    const StridedBlock *left, *right;
    MustAddressType *leftPos, *rightPos;
    // recognize the left one
    if (first < o.first)
        left = this, right = &o, leftPos = &posA, rightPos = &posB;
    else
        right = this, left = &o, leftPos = &posB, rightPos = &posA;
    // collision with first block of the right one?
    if (left->overlaps(right->first, right->first + right->blocksize))
    {
        *rightPos=right->pos;
        left->overlapPos(right->first, right->first + right->blocksize, leftPos);
        return true;
    }
/*    if (stride == o.stride)
        return false;*/
    // abs ( stride - o.stride )
    MustAddressType deltaStride = left->stride - right->stride,
                    absDeltaStride = abs(deltaStride),
                    overlapLength = min(second, o.second) - max(first, o.first);
    int deltaCount = overlapLength / max(stride, o.stride);
    // overlap on first interception -- is interception within the overlapping region?
    if (absDeltaStride < max(blocksize, o.blocksize))
    {
        if ( absDeltaStride * overlapLength > stride * o.stride )
        { // overlap, so calc position
            if ( deltaStride < 0 )
            {
                // first block of left type within right region
                MustAddressType firstLeftBlock = left->first + (MustAddressType)((right->first - left->first) / left->stride + 1) * left->stride;
                int count = (firstLeftBlock - (right->first + right->blocksize)) / absDeltaStride+1;
                left->overlapPos(right->first + right->stride * count, right->first + right->stride * count + right->blocksize, leftPos);
                right->overlapPos(firstLeftBlock + left->stride * count, firstLeftBlock + left->stride * count + left->blocksize, rightPos);
            }
            else
            {
                // last block of left type before right region
                MustAddressType lastLeftBlock = left->first + (MustAddressType)((right->first - left->first) / left->stride) * left->stride + left->blocksize;
                int count = (right->first - lastLeftBlock ) / absDeltaStride + 1;
                left->overlapPos(right->first + right->stride * count, right->first + right->stride * count + right->blocksize, leftPos);
                right->overlapPos(lastLeftBlock + left->stride * count, lastLeftBlock + left->stride * count + left->blocksize, rightPos);
            }
            return true;
        }
        // 
        if ( deltaStride < 0 )
        {
            // first block of left type within right region
            MustAddressType firstLeftBlock = left->first + (MustAddressType)((right->first - left->first) / left->stride + 1) * left->stride;
            if ((firstLeftBlock - (right->first + right->blocksize)) / absDeltaStride < deltaCount)
            {
                int count = (firstLeftBlock - (right->first + right->blocksize)) / absDeltaStride+1;
                left->overlapPos(right->first + right->stride * count, right->first + right->stride * count + right->blocksize, leftPos);
                right->overlapPos(firstLeftBlock + left->stride * count, firstLeftBlock + left->stride * count + left->blocksize, rightPos);
                return true;
            }
        }
        else
        {
            // last block of left type before right region
            MustAddressType lastLeftBlock = left->first + (MustAddressType)((right->first - left->first) / left->stride) * left->stride + left->blocksize;
            if ((right->first - lastLeftBlock ) / absDeltaStride <= deltaCount+1)
            {
                int count = (right->first - lastLeftBlock ) / absDeltaStride + 1;
                left->overlapPos(right->first + right->stride * count, right->first + right->stride * count + right->blocksize, leftPos);
                right->overlapPos(lastLeftBlock + left->stride * count, lastLeftBlock + left->stride * count + left->blocksize, rightPos);
                return true;
            }
        }
        return false;
    }
    // here we go O(min(m,n))
    // loop over the bigger stride
    if ( deltaStride < 0 )
    {
        *rightPos=right->pos;
        for (MustAddressType blockBegin=right->first; blockBegin<min(second, o.second); blockBegin+=right->stride, *rightPos+=right->blocksize)
        {
            if(left->overlapPos(blockBegin, blockBegin + right->blocksize, leftPos))
                return true;
        }
    }
    else
    {
        *leftPos=left->pos;
        for (MustAddressType blockBegin=left->first + (MustAddressType)((right->first - left->first) / left->stride + 1) * left->stride; blockBegin<min(second, o.second); blockBegin+=left->stride, *leftPos+=left->blocksize)
        {
            if(right->overlapPos(blockBegin, blockBegin + left->blocksize, rightPos))
                return true;
        }
    }
    return false;        
}

MustStridedBlocklistType must::buildStridedBlocklist(const MustStridedBlocklistType& blocklist, MustAddressType extent, MustAddressType size, MustAddressType offset, MustAddressType posOffset, MustAddressType blocklength, MustAddressType stride, MustAddressType count)
{
    MustStridedBlocklistType ret;
    MustStridedBlocklistType::iterator iter = blocklist.begin(),
                                       insertIter = ret.begin();
    int i,j;
    if (blocklist.size() == 1)
    {
        if(iter->count == 1 && iter->blocksize == extent)
        {
            ret.insert(StridedBlock(offset + iter->first, posOffset + iter->pos, iter->merged, 0, count, extent*blocklength, stride ));
            return ret;
        }
        if (iter->count == 1)
        {
            for (i=0; i<count; i++)
            {
            // StridedBlock(const MustAddressType& b, const MustAddressType& p, const bool& m, const int& r, const int& c, const MustAddressType& block, const MustAddressType& stride)
                //TODO: calulate position -- done
                insertIter = ret.insert(insertIter, StridedBlock(offset + iter->first + stride*i, posOffset + iter->pos + size*i, iter->merged, 0, blocklength, iter->blocksize, extent ));
            }
            return ret;
        }
    }
    for (iter = blocklist.begin(); iter != blocklist.end(); iter++)
    {
        for (i=0; i<count; i++)
        {
            for (j=0; j<blocklength; j++)
            {
                //TODO: calulate position
                insertIter = ret.insert(insertIter, StridedBlock(offset + iter->first + extent*j + stride*i, posOffset + iter->pos + size*(i*blocklength + j), iter->merged, 0, iter->count, iter->blocksize, iter->stride));
            }
        }
    }
    return ret;
}


MustMemIntervalListType must::buildMemIntervallist(const MustStridedBlocklistType& blocklist, const MustAddressType& extent, const MustAddressType& size, const MustAddressType& offset, const MustRequestType& request, const bool& isSend, I_Datatype *const& type, const int& count, const MustAddressType& baseAddress)
{
    MustMemIntervalListType ret;
    MustStridedBlocklistType::iterator iter = blocklist.begin();
	if (count==0)
		return ret;
    if (blocklist.size() == 1 && iter->count == 1 && iter->blocksize == extent)
    {
        ret.insert(memInterval( StridedBlock(iter->first, iter->pos, true, 0, 1, extent*count, 0), offset, request, isSend, type, baseAddress, 0));
    }
    else
    {
        MustMemIntervalListType::iterator insertIter = ret.begin();
        int i;
        for (iter = blocklist.begin(); iter != blocklist.end(); iter++)
        {
            for (i=0; i<count; i++)
            {
                //TODO: calulate position -- done
                insertIter = ret.insert(insertIter, memInterval(*iter, offset + i*extent, i*size, request, isSend, type, baseAddress, 0));
            }
        }
    }
    return ret;
}



// bool must::isOverlapped(const MustStridedBlocklistType& blocklist, MustStridedBlocklistType::iterator& iter, MustStridedBlocklistType::iterator& nextIter)
// {
// //     MustStridedBlocklistType::iterator iter=blocklist.begin(), nextIter;
//     for (iter=blocklist.begin();iter != blocklist.end(); iter++)
//     {
//         for (nextIter=iter, nextIter++; nextIter != blocklist.end(); nextIter++)
//         {
//             if (nextIter->first >= iter->second)
//                 break;
//             if (iter->overlaps(*nextIter))
//                 return true;
//         }
//     }
//     return false;
// }

bool must::isOverlapped(const MustStridedBlocklistType& blocklist, MustStridedBlocklistType::iterator& iter, MustStridedBlocklistType::iterator& nextIter, MustAddressType& posA, MustAddressType& posB)
{
//     MustStridedBlocklistType::iterator iter=blocklist.begin(), nextIter;
    for (iter=blocklist.begin();iter != blocklist.end(); iter++)
    {
        for (nextIter=iter, nextIter++; nextIter != blocklist.end(); nextIter++)
        {
            if (nextIter->first >= iter->second)
                break;
            if (iter->overlaps(*nextIter))
            {
                iter->overlapPos(*nextIter, posA, posB);
                return true;
            }
        }
    }
    return false;
}

// bool must::isOverlapped(const MustMemIntervalListType& blocklist, MustMemIntervalListType::iterator& iter, MustMemIntervalListType::iterator& nextIter, const bool& ignoreSend)
// {
// //     MustMemIntervalListType::iterator iter=blocklist.begin(), nextIter;
//     for (iter=blocklist.begin(); iter != blocklist.end(); iter++)
//     {
//         if (ignoreSend && iter->isSend)
//         {
//             for (nextIter=iter, nextIter++; nextIter != blocklist.end(); nextIter++)
//             {
//                 if (nextIter->first >= iter->second)
//                     break;
//                 if (!nextIter->isSend && iter->overlaps(*nextIter))
//                     return true;
//             }
//         }
//         else
//         {
//             for (nextIter=iter, nextIter++; nextIter != blocklist.end(); nextIter++)
//             {
//                 if (nextIter->first >= iter->second)
//                     break;
//                 if (iter->overlaps(*nextIter))
//                     return true;
//             }
//         }
//     }
//     return false;
// }


bool must::isOverlapped(const MustMemIntervalListType& blocklist, MustMemIntervalListType::iterator& iter, MustMemIntervalListType::iterator& nextIter, MustAddressType& posA, MustAddressType& posB, const bool& ignoreSend, bool ignoreSameRequest)
{
//     MustMemIntervalListType::iterator iter=blocklist.begin(), nextIter;
    for (iter=blocklist.begin(); iter != blocklist.end(); iter++)
    {
        if (ignoreSend && iter->isSend)
        {
            for (nextIter=iter, nextIter++; nextIter != blocklist.end(); nextIter++)
            {
                if (ignoreSameRequest && nextIter->request == iter->request)
                    continue;
                if (nextIter->first >= iter->second)
                    break;
                if (!nextIter->isSend && iter->overlaps(*nextIter))
                {
                    iter->overlapPos(*nextIter, posA, posB);
                    return true;
                }
            }
        }
        else
        {
            for (nextIter=iter, nextIter++; nextIter != blocklist.end(); nextIter++)
            {
                if (ignoreSameRequest && nextIter->request == iter->request)
                    continue;
                if (nextIter->first >= iter->second)
                    break;
                if (iter->overlaps(*nextIter))
                {
                    iter->overlapPos(*nextIter, posA, posB);
                    return true;
                }
            }
        }
    }
    return false;
}

// bool must::isOverlapped(const MustMemIntervalListType& listA, const MustMemIntervalListType& listB, MustMemIntervalListType::iterator& iterA, MustMemIntervalListType::iterator& iterB, const bool& ignoreSend)
// {
// //     MustMemIntervalListType::iterator iterA, iterB;
//     for (iterB = listB.begin();iterB != listB.end(); iterB++)
//     {
//         if (ignoreSend && iterB->isSend)
//         {
//             for (iterA = listA.begin(); iterA != listA.end(); iterA++)
//             {
//                 if (iterA->first >= iterB->second)
//                     break;
//                 if (!iterA->isSend && iterA->overlaps(*iterB))
//                     return true;
//             }
//         }
//         else
//         {
//             for (iterA = listA.begin(); iterA != listA.end(); iterA++)
//             {
//                 if (iterA->first >= iterB->second)
//                     break;
//                 if (iterA->overlaps(*iterB))
//                     return true;
//             }
//         }
//     }
//     return false;
// }

bool must::isOverlapped(const MustMemIntervalListType& listA, const MustMemIntervalListType& listB, MustMemIntervalListType::iterator& iterA, MustMemIntervalListType::iterator& iterB, MustAddressType& posA, MustAddressType& posB, const bool& ignoreSend, bool ignoreSameRequest)
{
//     MustMemIntervalListType::iterator iterA, iterB;
    for (iterB = listB.begin();iterB != listB.end(); iterB++)
    {
        if (ignoreSend && iterB->isSend)
        {
            for (iterA = listA.begin(); iterA != listA.end(); iterA++)
            {
                if (ignoreSameRequest && iterA->request == iterB->request)
                    continue;
                if (iterA->first >= iterB->second)
                    break;
                if (!iterA->isSend && iterA->overlaps(*iterB))
                {
                    iterA->overlapPos(*iterB, posA, posB);
                    return true;
                }
            }
        }
        else
        {
            for (iterA = listA.begin(); iterA != listA.end(); iterA++)
            {
                if (ignoreSameRequest && iterA->request == iterB->request)
                    continue;
                if (iterA->first >= iterB->second)
                    break;
                if (iterA->overlaps(*iterB))
                {
                    iterA->overlapPos(*iterB, posA, posB);
                    return true;
                }
            }
        }
    }
    return false;
}

void must::MustStridedBlocklistType::checkOverlapped()
{   
    MustStridedBlocklistType::iterator iter, nextIter;
    overlapped = isOverlapped(*this, iter, nextIter, posA, posB);
}

