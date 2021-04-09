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
 * @file ThreadChannel.cpp
 * 
 *
 * @author Felix Münchhalfen
 * @date 26.02.2015
 *
 */

#include "ThreadChannel.h"

ThreadChannel::ThreadChannel() {
    m_SendBuf = new IOBuffer();
    m_RecvBuf = new IOBuffer();
}

ThreadChannel::ThreadChannel(const ThreadChannel& orig) {
}

ThreadChannel::~ThreadChannel() {
    // clean up
    delete m_RecvBuf;
    delete m_SendBuf;
}

