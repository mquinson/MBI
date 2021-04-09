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
 * @file ThreadChannel.h
 * 
 *
 * @author Felix Münchhalfen
 * @date 26.02.2015
 *
 */

#include "VectorBuffer.h"

#ifndef THREADCHANNEL_H
#define	THREADCHANNEL_H

typedef VectorBuffer IOBuffer;

class ThreadChannel {
public:
    ThreadChannel();
    ThreadChannel(const ThreadChannel& orig);
    virtual ~ThreadChannel();
    
    BIOBuffer *getSendBuffer() { return m_SendBuf; }
    BIOBuffer *getRecvBuffer() { return m_RecvBuf; }
private:
    BIOBuffer *m_RecvBuf;
    BIOBuffer *m_SendBuf;
};

#endif	/* THREADCHANNEL_H */

