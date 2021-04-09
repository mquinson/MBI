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
 * @file BaseConstants.h
 *       @see MUST::BaseConstants.
 *
 *  @date 12.04.2011
 *  @author Mathias Korepkat
 */

#include "ModuleBase.h"
#include "I_BaseConstants.h"

#include <map>

#ifndef BASECONSTANTS_H
#define BASECONSTANTS_H

using namespace gti;

namespace must
{
	/**
	 * Implementation of I_BaseConstants.
	 */
	class BaseConstants : public gti::ModuleBase<BaseConstants, I_BaseConstants>
	{
	public:
		/**
		 * Constructor.
		 * @param instanceName name of this module instance.
		 */
		BaseConstants (const char* instanceName);

		/**
		 * Destructor.
		 */
		~BaseConstants (void);

		/**
		 * @see I_BaseConstants::addConstants.
		 */
		GTI_ANALYSIS_RETURN addConstants (
				int mpiProcNull,
				int mpiAnySource,
				int mpiAnyTag,
				int mpiUndefined,
				int mpiBsendOverhead,
				int mpiTagUb,
				int mpiVersion,
				int mpiSubversion,
                int mpiDistributeBlock,
                int mpiDistributeCyclic,
                int mpiDistributeNone,
                int mpiDistributeDfltDarg,
                int mpiOrderC,
                int mpiOrderFortran,
				void* mpiBottom
				);

		 /**
		 * @see I_BaseConstants::isProcNull.
		 */
		 bool isProcNull (int val);

		 /**
		 * @see I_BaseConstants::getProcNull.
		 */
		 int getProcNull (void);

		 /**
		 * @see I_BaseConstants::isAnySource
		 */
		 bool isAnySource (int val);

		 /**
		 * @see I_BaseConstants::getAnySource.
		 */
		 int getAnySource (void);

		 /**
		 * @see I_BaseConstants::isAnyTag.
		 */
		 bool isAnyTag (int val);

		 /**
		 * @see I_BaseConstants::getAnyTag.
		 */
		 int getAnyTag (void);

		 /**
		 * @see I_BaseConstants::isUndefined.
		 */
		 bool isUndefined (int val);

		 /**
		 * @see I_BaseConstants::getUndefined.
		 */
		 int getUndefined (void);

		 /**
		 * @see I_BaseConstants::isBsendOverhead.
		 */
		 bool isBsendOverhead (int val);

		 /**
		 * @see I_BaseConstants::getBsendOverhead.
		 */
		 int getBsendOverhead (void);

		 /**
		 * @see I_BaseConstants::isTagUb.
		 */
		 bool isTagUb (int val);

		 /**
		 * @see I_BaseConstants::getTagUb.
		 */
		 int getTagUb (void);

		 /**
		 * @see I_BaseConstants::isVersion.
		 */
		 bool isVersion (int val);

		 /**
		 * @see I_BaseConstants::getVersion.
		 */
		 int getVersion (void);

		 /**
		 * @see I_BaseConstants::isSubversion.
		 */
		 bool isSubversion (int val);

		 /**
		 * @see I_BaseConstants::getSubversion.
		 */
		 int getSubversion (void);

        /**
         * @see I_BaseConstants::isDistributeBlock.
         */
        bool isDistributeBlock (int val);

        /**
         * @see I_BaseConstants::getDistributeBlock.
         */
        int getDistributeBlock (void);

        /**
         * @see I_BaseConstants::isDistributeCyclic.
         */
        bool isDistributeCyclic (int val);

        /**
         * @see I_BaseConstants::getDistributeCyclic.
         */
        int getDistributeCyclic (void);

        /**
         * @see I_BaseConstants::isDistributeNone.
         */
        bool isDistributeNone (int val);

        /**
         * @see I_BaseConstants::getDistributeNone.
         */
        int getDistributeNone (void);

        /**
         * @see I_BaseConstants::isDistributeDfltDarg.
         */
        bool isDistributeDfltDarg (int val);

        /**
         * @see I_BaseConstants::getDistributeDfltDarg.
         */
        int getDistributeDfltDarg (void);

        /**
         * @see I_BaseConstants::isOrderC.
         */
        bool isOrderC (int val);

        /**
         * @see I_BaseConstants::getOrderC.
         */
        int getOrderC (void);

        /**
         * @see I_BaseConstants::isOrderFortran.
         */
        bool isOrderFortran (int val);

        /**
         * @see I_BaseConstants::getOrderFortran.
         */
        int getOrderFortran (void);

        /**
		 * @see I_BaseConstants::isBottom.
		 */
		bool isBottom (void* val);

		/**
		 * @see I_BaseConstants::getBottom.
		 */
		void* getBottom (void);

	protected:

		 int myMpiProcNull,
		 	 myMpiAnySource,
		 	 myMpiAnyTag,
		 	 myMpiUndefined,
		 	 myMpiBsendOverhead,
		 	 myMpiTagUb,
		 	 myMpiVersion,
		 	 myMpiSubversion,
             myMpiDistributeBlock,
             myMpiDistributeCyclic,
             myMpiDistributeNone,
             myMpiDistributeDfltDarg,
             myMpiOrderC,
             myMpiOrderFortran;
		 void* myMpiBottom;
	};

} //end namespace MUST

#endif /*BASECONSTANTS_H*/
