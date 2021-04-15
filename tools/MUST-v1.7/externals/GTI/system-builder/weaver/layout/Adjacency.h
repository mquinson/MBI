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
 * @file Adjacency.h
 * 		@see gti::weaver::Adjacency
 *
 * @author Tobias Hilbrich
 * @date 13.01.2010
 */

#ifndef ADJACENCY_H
#define ADJACENCY_H

#include <string>
#include <vector>

#include "Level.h"
#include "Communication.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace layout
		{
            enum DistributionType
            {
                DISTRIBUTION_UNIFORM = 0,
                DISTRIBUTION_BY_BLOCK,
            };

			class Level;

			/**
			  * Simple class for a communication connection between
			  * two levels. It does specify in what relationship these
			  * levels are (e.g. who is the lower level).
			  */
			class Adjacency : virtual public Printable
			{
			public:

				/**
				 * Proper constructor.
				 * @param target pointer to target level of this comm.
				 * @param comm pointer to communication used.
				 */
				Adjacency (
				        Level* target,
				        Communication* comm,
				        DistributionType distrib = DISTRIBUTION_UNIFORM,
				        int blocksize = 0);

				/**
				 * Invalid object constructor.
				 */
				Adjacency ();

				/**
				 * Destructor.
				 */
				~Adjacency ();

				/**
				 * Returns the target level.
				 * @return level.
				 */
				Level* getTarget (void);

				/**
				 * Returns the distribution type of this connection.
				 */
				DistributionType getDistribution (void);

				/**
				 * Returns the blocksize of this connection. Only
				 * meaningful if this is a by-block distribution.
				 */
				int getBlocksize (void);

				/**
				 * Returns the communication.
				 * @return communication.
				 */
				Communication* getComm (void);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

			public:
				Level* target;
				Communication* comm;
				DistributionType distrib;
				int blocksize;
			};
		} /*namespace layout*/
	} /*namespace weaver*/
} /*namespace must*/
#endif // ADJACENCY_H
