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
 * @file VectorBuffer.h
 * 
 *
 * @author Felix Münchhalfen
 * @date 26.02.2015
 *
 */

#include "BIOBuffer.h"
#include <pthread.h>
#include <deque>
#include <string.h>

#ifndef VECTORBUFFER_H
#define	VECTORBUFFER_H

    class VectorBuffer : public BIOBuffer {
    private:
                
        std::deque<BYTE> m_vector;
        pthread_mutex_t m_lock;
        
        unsigned int getSizeInternal() { return m_vector.size(); }
        int peekDataInternal(BYTE *copybuf, unsigned int length, unsigned int *remote_reqid);
        
    public:
        VectorBuffer( unsigned int size = 10000 );
        ~VectorBuffer();
        BYTE* getHeadPtr();
        int peekData(BYTE *copybuf, unsigned int length, unsigned int *remote_reqid);
        int readData(BYTE *copybuf, unsigned int length, unsigned int *remote_reqid);
        bool pushData(const BYTE *refbuf, unsigned int length, unsigned int reqid);
        bool IsEmpty() { return (getSizeInternal() == 0); }
    };

#endif	/* VECTORBUFFER_H */

