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
 * @file OnlyOnRootCondition.h
 *       @see MUST::OnlyOnRootCondition.
 *
 *  @date 23.08.2011
 *  @author Mathias Korepkat, Joachim Protze
 */

#include "ModuleBase.h"

#include "I_ParallelIdAnalysis.h"
#include "I_OnlyOnRootCondition.h"
#include "I_CommTrack.h"
#include "I_GroupTable.h"

#include <string>

#ifndef ONLYONROOTCONDITION_H
#define ONLYONROOTCONDITION_H

using namespace gti;

namespace must
{
	/**
     * Template for correctness checks interface implementation.
     */
    class OnlyOnRootCondition : public gti::ModuleBase<OnlyOnRootCondition, I_OnlyOnRootCondition>
    {
    public:
        /**
        * Constructor.
        * @param instanceName name of this module instance.
        */
		OnlyOnRootCondition (const char* instanceName);

    	/**
    	 * Destructor.
    	 */
    	virtual ~OnlyOnRootCondition (void);

   		/**
   		 * @see I_OnlyOnRootCondition::gather
   		 */
        GTI_ANALYSIS_RETURN gather (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType recvbuf,
                MustArgumentId recvbufArgId,
                int recvcount,
                MustArgumentId recvcountArgId,
                MustDatatypeType recvtype,
                MustArgumentId recvtypeArgId,
                int root,
                MustCommType comm
                );

            /**
             * @see I_OnlyOnRootCondition::gatherv
             */
        GTI_ANALYSIS_RETURN gatherv (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType recvbuf,
                MustArgumentId recvbufArgId,
                const int recvcounts[],
                MustArgumentId recvcountsArgId,
                const int displs[],
                MustArgumentId displsArgId,
                MustDatatypeType recvtype,
                MustArgumentId recvtypeArgId,
                int root,
                MustCommType comm
                );


            /**
             * @see I_OnlyOnRootCondition::scatter
             */
        GTI_ANALYSIS_RETURN scatter (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustArgumentId sendbufArgId,
                int sendcount,
                MustArgumentId sendcountArgId,
                MustDatatypeType sendtype,
                MustArgumentId sendtypeArgId,
                int root,
                MustCommType comm
                );

        /**
         * @see I_OnlyOnRootCondition::scatterv
         */
    GTI_ANALYSIS_RETURN scatterv (
            MustParallelId pId,
            MustLocationId lId,
            MustAddressType sendbuf,
            MustArgumentId sendbufArgId,
            const int sendcounts[],
            MustArgumentId sendcountsArgId,
            const int displs[],
            MustArgumentId displsArgId,
            MustDatatypeType sendtype,
            MustArgumentId sendtypeArgId,
            int root,
            MustCommType comm
            );

    protected:
        I_ParallelIdAnalysis* myPIdMod;
        I_CommTrack* myComMod;
        /* simplify your life */
        int pId2Rank(
                MustParallelId pId);


        MustOnRootTransferP myTransfer;
        MustOnRootTransferCountsP myTransferCounts;
    };
} /*namespace MUST*/

#endif /*ONLYONROOTCONDITION_H*/
