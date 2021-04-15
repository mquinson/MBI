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
 * @file GtiTLS.h
 *       Header file for GTI thread safety.
 *
 * @author Joachim Protze
 * @date 01.06.2017
 */

#ifndef GTI_TLS_H
#define GTI_TLS_H


#include "gtiConfig.h"

#ifndef GTI_THREAD_SAFETY

#define thread_local

#else

extern "C" int getGtiTid();
/*#ifdef PROVIDE_TID_IMPLEMENTATION
    #include <mutex>
    #include <list>

int getGtiTid() {
    static __thread int tid=-1;
    static std::mutex _m;
    static int new_tid = 0;
    if (tid<0){
        std::lock_guard<std::mutex> lock(_m);
        tid=new_tid++;
    }
    return tid;
}
#endif*/

#endif




#endif /*GTI_TLS_H*/
