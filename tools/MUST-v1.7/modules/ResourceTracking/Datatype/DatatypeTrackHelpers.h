/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#ifndef DATATYPE_TRACK_HELPERS_H
#define DATATYPE_TRACK_HELPERS_H

#include <iostream>
#include <vector>
#include "I_BaseConstants.h"

class FlexCounter{
    protected:
        int * sizes;
        std::vector<int> digits;
        int ndims;
    public:
        FlexCounter(int sizes[], int ndims);
        std::vector<int> tick(int ticks=1);
};

class SubarrayWalk{

    protected:

        int* sizes;
        int* subsizes;
        int* starts;
        int* ends;
        int* digits;
        int* omap;
        int ndims;
        int value;
        int max;

    public:

        SubarrayWalk(int sizes[], int subsizes[], int starts[], int ndims, bool corder);

        virtual ~SubarrayWalk (void);

        void calc_value();

        int& operator++();

        int operator++(int);

        operator int();

        int& end();

        bool tick();
};

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

class DarrayWalk{

    protected:

        int* digits;
        int* subsizes;
        int* sizes;
        int* omap;
        int** entries;
        int ndims;
        int value;
        int max;

        void calc_value();
        void calc_entries(int distr, int darg, int start, int subsize, int psize, int* entrie);
        bool tick();
        I_BaseConstants* myBCoMod;

    public:

        DarrayWalk(int a_sizes[], int a_dargs[], int a_distribs[], int a_psizes[], int a_subsizes[], int ndims, int rank, bool corder, I_BaseConstants* BCoMod);

        virtual ~DarrayWalk (void);

        int& operator++();

        int operator++(int);

        operator int();

        int& end();
};

#endif /*DATATYPE_TRACK_HELPERS_H*/
