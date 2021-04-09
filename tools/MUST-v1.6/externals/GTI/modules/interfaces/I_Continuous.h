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
 * @file I_Continuous.h
 *       @see gti::I_Continuous
 *
 * @author Tobias Hilbrich
 * @date 12.01.2015
 */

#include <stdint.h>

#ifndef I_CONTINUOUS_H
#define I_CONTINUOUS_H

namespace gti
{
	/**
	 * Interface that needs to be implemented by all modules with continuous analyses,
	 * which will be triggered regularily by the underlying placement driver.
	 */
	class I_Continuous
	{
	public:

		/**
		 * Virtual destructor to make compilers happy.
		 */
		inline virtual ~I_Continuous (void) {};

		/**
		 * Called to trigger the analysis.
		 * @param usecSincLastTrigger specifies how many microseconds passed since the last
		 *        time the analysis was triggered.
		 */
		virtual void trigger (uint64_t usecSincLastTrigger) = 0;
	}; //I_Continuous
}//namespace gti

#endif //#ifdef I_CONTINUOUS_H
