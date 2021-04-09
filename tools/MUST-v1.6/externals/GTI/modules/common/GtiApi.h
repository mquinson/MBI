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
 * @file BaseApi.h
 * 		P call definition for implicitly added GTI API.
 *
 * @author Tobias Hilbrich
 * @date 27.06.2012
 */

#include <stdint.h>

#ifndef GTI_API_H
#define GTI_API_H

/**
 * Function pointer type used to set an injected event to use a strided rank-range representation.
 */
typedef int (*gtiSetNextEventStridedP) (uint32_t offset, uint32_t stride);

/**
 * API event for notifying all layers of a shutdown
 */
inline int PgtiShutdownNotify (void) {return 0;}
typedef int (*gtiShutdownNotifyP) (void);

/**
 * API event for raising a panic.
 *
 * Description:
 * All TBON nodes can create this event in order to raise a panic, i.e.,
 * an ongoing application crash. It causes ALL TBON nodes to:
 * a) Flush outstanding communications
 * b) Deactivate potentially existent event aggregation (I.e., all successive sends will be immediate)
  */
inline int PgtiRaisePanic (void) {return 0;}
typedef int (*gtiRaisePanicP) (void);

/**
 * API event for notifying all TBON nodes of an ongoing panic.
 *
 * Description:
 * Created by GTI internaly, should not be created by the user!
 * Is triggered and broadcasted to all TBON nodes if at least one
 * gtiRaisePanic event was created on some TBON node.
 */
inline int PgtiNotifyPanic (void) {return 0;}
typedef int (*gtiNotifyPanicP) (void);

/**
 * API event for notifying all TBON nodes of a request to flush their
 * outstanding events.
 *
 * Description:
 * Can be created on any TBON node, can be called by any user module.
 * Causes the sub-tree connected to the current TBON node to flush all
 * outstanding upwards and intra-layer communication. Successive aggregation
 * after this event arrives on some node is allowed.
 */
inline int PgtiNotifyFlush (void) {return 0;}
typedef int (*gtiNotifyFlushP) (void);

/**
 * Function to request a halt of the target application. This is to allow
 * all tool places to process any outstanding events before additional
 * application events arrive.
 *
 * The use case of this API is to avoid OOM situations due to excessive
 * event queues/buffers/module-datastructures.
 */
inline int PgtiBreakRequest (void) {return 0;}
typedef int (*gtiBreakRequestP) (void);

/**
 * If a module requested a break it calls this API function once its
 * own buffer size went back to a normal limit (the limit should not
 * under any circumstances be 0, rather something like MAX_LIMIT/X
 * as an example)
 * @see gtiBreakRequest
 */
inline int PgtiBreakConsume (void) {return 0;}
typedef int (*gtiBreakConsumeP) (void);

/**
 * The function that GTI's internal break management uses to
 * enforce a break on the application processes.
 *
 * @code if code==1 then enforce a break, if code == 0 then release a previous break.
 *
 * @see gtiBreakRequest
 */
inline int PgtiBroadcastBreak (
        int code) {return 0;}
typedef int (*gtiBroadcastBreakP) (int);

#endif /*GTI_API_H*/
