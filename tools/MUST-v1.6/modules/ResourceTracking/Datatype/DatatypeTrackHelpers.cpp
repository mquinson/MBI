/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <iostream>
#include "DatatypeTrackHelpers.h"

// class subarraywalk:
//     """walk over an mpi_subarray
//     """
//     def __init__(self, sizes, subsizes, starts):
//         self.sizes = sizes
//         self.subsizes = subsizes
//         self.starts = list(starts)
//         self.digits = list(starts)
//         self.ends = map(sum,zip(starts,subsizes))
//         #if order=='MPI_ORDER_C':
//         self.it = range(len(self.sizes))
//         #else:
//             #self.it = range(len(self.sizes)-1,-1,-1)
//     def tick(self, ticks=1):
//         broken = True
//         while broken and ticks==None or ticks>0:
//             broken = False
//             for i in self.it:
//                 self.digits[i] += 1
//                 if self.digits[i] >= self.ends[i]:
//                     self.digits[i] = self.starts[i] # reset and carry to next digit
//                 else:
//                     broken = True
//                     break # no carry, so stop tick
//             ticks -= 1
//         return broken   # False means carry-overflow / finished cycles
//     def allticks(self):
//         "returns iterator over all items"
//         yield self.digits
//         while self.tick(1):
//             yield self.digits
// 

//=============================
// SubarrayWalk: walk over all entries of an SubArray and return the index of the entry
//=============================

//=============================
// Constructor
//=============================
SubarrayWalk::SubarrayWalk(int sizes[], int subsizes[], int starts[], int ndims, bool corder) :
sizes(sizes), subsizes(subsizes), starts(starts), ndims(ndims)
{
    digits = new int[ndims];
    ends = new int[ndims];
    omap = new int[ndims];
    value=0;
    max=1;
    for (int i = ndims -1 ; i >= 0; i--)
    {
        if (!corder)
            omap[i]=i;
        else
            omap[i]=ndims-i-1;
        ends[i] = starts[i] + subsizes[i];
        digits[i] = starts[i];
        max *= sizes[i];
    }
    calc_value();
}

//=============================
// Destructor
//=============================
SubarrayWalk::~SubarrayWalk (void)
{
    delete [] digits;
    delete [] omap;
    delete [] ends;
}

//=============================
// Calulate the index value
//=============================
void SubarrayWalk::calc_value()
{
    value=0;
    for (int i = ndims -1 ; i >= 0; i--)
    {
        value *= sizes[omap[i]];
        value += digits[omap[i]];
    }
}

//=============================
// preincrement
//=============================
int& SubarrayWalk::operator++(){
    tick();
    return value;
}

//=============================
// postincrement
//=============================
int SubarrayWalk::operator++(int){
    int oldvalue = value;
    tick();
    return oldvalue;
}

//=============================
// cast to int, to return the index
//=============================
SubarrayWalk::operator int()
{
    return value;
}

//=============================
// value that signals end of walk
//=============================
int& SubarrayWalk::end()
{
    return max;
}

//=============================
// calculate the next entry
//=============================
bool SubarrayWalk::tick()
{
    for (int i=0; i<ndims; i++)
    {
        digits[omap[i]]++;
        if (digits[omap[i]] >= ends[omap[i]])
        {
            digits[omap[i]] = starts[omap[i]];
        }
        else if (i==0)
        {
            value++;
            return true;
        }
        else
        {
            calc_value();
            return true;
        }
    }
    value = max;
    return false;
}

//=============================
// FlexCounter: waterclock with variating ringsizes (sizes[])
// used to calculate the position of n-th rank in cartesian grid (DArray!)
//=============================

//=============================
// Constructor
//=============================
FlexCounter::FlexCounter(int sizes[], int ndims):
sizes(sizes),
ndims(ndims)
{
    digits = std::vector<int>(ndims);
}

//=============================
// count for *ticks* ticks
//=============================
std::vector<int> FlexCounter::tick(int ticks)
{
    while (ticks>0)
    {
        for(int i=0; i<ndims; i++)
        {
            digits[i]++;
            if (digits[i] >= sizes[i])
                digits[i]=0;
            else
                break;
        }
        ticks--;
    }
    return digits;
}

//=============================
// DarrayWalk: walk over all entries of a DArray and return the index of the entry
//=============================

//=============================
// calculate the index for current entry
//=============================
void DarrayWalk::calc_value()
{
    value=0;
    for (int i = ndims -1 ; i >= 0; i--)
    {
        value *= sizes[omap[i]];
        value += entries[omap[i]][digits[omap[i]]];
    }
}

// def calc_entries(distr, darg, start, subsize, psize):
//     if distr == 'CYCLIC':
//         if darg == mpipp_distribution_dflt_darg or darg == 1:
//             return [ i * subsize + start for i in range(subsize)]
//         return [ i % darg + start * darg + i / darg * psize * darg for i in range(subsize)]
//     elif distr == 'BLOCK':
//         return [ i + subsize * start for i in range(subsize)]
//     else: # 'MPI_DISTRIBUTION_NULL' !?
//         return range(subsize)
// 

//=============================
// calculate the entry map for a dimension, depending on distr and darg value
//=============================
void DarrayWalk::calc_entries(int distr, int darg, int start, int subsize, int psize, int* entrie)
{
    if (myBCoMod->isDistributeCyclic(distr))
    {
        if (myBCoMod->isDistributeDfltDarg(darg) || darg == 1)
        {
            for (int i = 0; i < subsize; i++){
                entrie[i] = i * psize + start;
            }
        }
        else
        {
            for (int i = 0; i < subsize; i++){
                entrie[i] = i % darg + start * darg + i / darg * psize * darg;
            }
        }
    }
    else if (myBCoMod->isDistributeBlock(distr))
    {
        for (int i = 0; i < subsize; i++){
            entrie[i] = i + start * subsize;
        }
    }
    else // myBCoMod->isDistributeNone(distr)
    {
        // whole dimension is spared over one process
        for (int i = 0; i < subsize; i++){
            entrie[i] = i;
        }
    }
}

//=============================
// calculate the next entry
//=============================
bool DarrayWalk::tick()
{
    for (int i=0; i<ndims; i++)
    {
        digits[omap[i]]++;
        if (digits[omap[i]] >= subsizes[omap[i]])
        {
            digits[omap[i]] = 0;
        }
        else
        {
            calc_value();
            return true;
        }
    }
    value = max;
    return false;
}

// class darraywalk:
//     def __init__(self, ndims, a_sizes, a_dargs, a_distribs, a_psizes, a_starts, a_subsizes):
//         self.ndims = ndims
//         self.sizes = a_sizes
//         self.subsizes = a_subsizes
//         self.psizes = a_psizes
//         self.distribs = a_distribs
//         self.dargs = a_dargs
//         self.entries = [calc_entries(a_distribs[i], a_dargs[i], a_starts[i], a_subsizes[i], a_psizes[i]) for i in range(ndims)]
//         self.digits = [0]*ndims
//     def tick(self, ticks=1):
//         broken = True
//         while broken and ticks==None or ticks>0:
//             broken = False
//             for i in range(self.ndims):
//                 self.digits[i] += 1
//                 if self.digits[i] >= self.subsizes[i]:
//                     self.digits[i] = 0 # reset and carry to next digit
//                 else:
//                     broken = True
//                     break # no carry, so stop tick
//             ticks -= 1
//         return broken   # False means carry-overflow / finished cycles
//     def allticks(self):
//         "returns iterator over all items"
//         yield [self.entries[i][j] for i,j in enumerate(self.digits)]
//         while self.tick(1):
//             yield [self.entries[i][j] for i,j in enumerate(self.digits)]

//=============================
// Constructor
//=============================
DarrayWalk::DarrayWalk(int a_sizes[], int a_dargs[], int a_distribs[], int a_psizes[], int a_subsizes[], int ndims, int rank, bool corder, I_BaseConstants* BCoMod):
subsizes(a_subsizes),
sizes(a_sizes),
ndims(ndims),
myBCoMod(BCoMod)
{
    std::vector<int> a_starts = FlexCounter(a_psizes, ndims).tick(rank);
    digits = new int[ndims];
    omap = new int[ndims];
    entries = new int*[ndims];
    for (int i=0; i<ndims; i++){
        if (!corder)
            omap[i]=i;
        else
            omap[i]=ndims-i-1;
        digits[i]=0;
        entries[i] = new int[a_subsizes[i]];
        calc_entries(a_distribs[i], a_dargs[i], a_psizes[i], a_starts[i], a_subsizes[i], entries[i]);
    }
}

//=============================
// Destructor
//=============================
DarrayWalk::~DarrayWalk (void)
{
    for (int i=0; i<ndims; i++){
        delete [] entries[i];
    }
    delete [] omap;
    delete [] entries;
    delete [] digits;
}

//=============================
// preincrement
//=============================
int& DarrayWalk::operator++(){
    tick();
    return value;
}

//=============================
// postincrement
//=============================
int DarrayWalk::operator++(int){
    int oldvalue = value;
    tick();
    return oldvalue;
}

//=============================
// cast to int
//=============================
DarrayWalk::operator int()
{
    return value;
}

//=============================
// value that signals end of walk
//=============================
int& DarrayWalk::end()
{
    return max;
}


// tiny usage example for SubarrayWalk:
// can be compiled standalone

// int main(int argc, char** argv)
// {
//     int sizes[] = {4,3,2};
//     int starts[] = {1,1,0};
//     int subsizes[] = {2,2,2};
//     for (SubarrayWalk saw = SubarrayWalk(sizes, subsizes, starts, 3, true); (int)saw != saw.end(); saw++)
//         std::cout << (int)saw << std::endl;
//     return 0;
// }

