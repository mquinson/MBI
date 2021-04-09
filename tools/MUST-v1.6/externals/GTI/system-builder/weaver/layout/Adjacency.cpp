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
 * @file Adjacency.cpp
 * 		@see gti::weaver::Adjacency
 *
 * @author Tobias Hilbrich
 * @date 13.01.2010
 */

#include "Adjacency.h"

using namespace gti::weaver::layout;

//=============================
// Adjacency
//=============================
Adjacency::Adjacency (
        Level* target,
        Communication* comm,
        DistributionType distrib,
        int blocksize)
:  target (target),
   comm (comm),
   distrib (distrib),
   blocksize (blocksize)
{

}

//=============================
// Adjacency
//=============================
Adjacency::Adjacency ()
: target (NULL),
  comm (NULL),
  distrib (DISTRIBUTION_UNIFORM),
  blocksize (0)
{

}

//=============================
// ~Adjacency
//=============================
Adjacency::~Adjacency ()
{
	/*Do not free the pointers,
	 *their storage is managed by
	 *their respective singletons.
	 */
	target = NULL;
	comm = NULL;
}

//=============================
// getTarget
//=============================
Level* Adjacency::getTarget (void)
{
	return target;
}

//=============================
// getComm
//=============================
Communication* Adjacency::getComm (void)
{
	return comm;
}

//=============================
// print
//=============================
std::ostream& Adjacency::print (std::ostream& out) const
{
	if (!target || !comm)
	{
		out << "invalidAdjacency";
		return out;
	}

	out
		<< "adjacency={targetLevel="
		<< target->getOrder()
		<< ", "
		<< *comm
		<< "}";

	return out;
}

//=============================
// getDistribution
//=============================
DistributionType Adjacency::getDistribution (void)
{
    return distrib;
}

//=============================
// getBlocksize
//=============================
int Adjacency::getBlocksize (void)
{
    return blocksize;
}

/*EOF*/
