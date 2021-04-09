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
 * @file RingBuffer.h
 * 
 *
 * @author Felix Münchhalfen
 * @date 26.02.2015
 *
 */

#include "BIOBuffer.h"
#include <pthread.h>

#ifndef RINGBUFFER_H
#define	RINGBUFFER_H

    class RingBuffer : public BIOBuffer {
    private:
        BYTE *m_Buffer;
        BYTE *m_Begin;
        BYTE *m_End;
        bool m_Full;
        unsigned int m_BufferSize;
        pthread_mutex_t m_lock;
        
        unsigned int getSizeInternal() { if(m_Full) return m_BufferSize; return (m_End + m_BufferSize - m_Begin) % m_BufferSize; }
        int peekDataInternal(BYTE *copybuf, unsigned int length, unsigned int *remote_req);
        
    public:
        RingBuffer( unsigned int size = 10000 );
        ~RingBuffer();
        BYTE* getHeadPtr();
        int peekData(BYTE *copybuf, unsigned int length, unsigned int *remote_req);
        int readData(BYTE *copybuf, unsigned int length, unsigned int *remote_req);
        bool pushData(const BYTE *refbuf, unsigned int length, unsigned int req);
        bool IsEmpty() { return (getSizeInternal() == 0); }
    };

#endif	/* RINGBUFFER_H */

