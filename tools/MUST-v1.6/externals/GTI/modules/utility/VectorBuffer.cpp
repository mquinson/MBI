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


#include "VectorBuffer.h"
#include <assert.h>

VectorBuffer::VectorBuffer(unsigned int size)
{
    pthread_mutex_init (&m_lock, NULL);
}

VectorBuffer::~VectorBuffer() {
    pthread_mutex_destroy (&m_lock);
}

BYTE* VectorBuffer::getHeadPtr()
{
    return (BYTE*)(&m_vector.front());
}

int VectorBuffer::peekDataInternal(BYTE *copybuf, unsigned int length, unsigned int *remote_reqid)
{
    if( getSizeInternal() <= 0)
        return 0;
    
    // get length
    unsigned int len = 0;
    for( int x = 0; x < sizeof(unsigned int); x++ )
        ((unsigned char*)&len)[x] = m_vector[x];
        
    // get remote-reqid
    for( int x = 0; x < sizeof(unsigned int); x++ )
        ((unsigned char*)remote_reqid)[x] = m_vector[sizeof(unsigned int)+x];
    
    assert(length >= len);
    
    for( int i = 0; i < len; i++ )
        copybuf[i] = m_vector[sizeof(unsigned int)*2+i];
    
    return len;
}

int VectorBuffer::peekData(BYTE *copybuf, unsigned int length, unsigned int *remote_reqid)
{
    pthread_mutex_lock(&m_lock);
    int ret = peekDataInternal(copybuf, length, remote_reqid);
    pthread_mutex_unlock(&m_lock);
    return ret;
}

int VectorBuffer::readData(BYTE *copybuf, unsigned int length, unsigned int *remote_reqid)
{
    pthread_mutex_lock(&m_lock);
    unsigned int len = peekDataInternal(copybuf, length, remote_reqid);
    if( len == 0 )
    {
        pthread_mutex_unlock(&m_lock);
        return 0;
    }

    // remove the data when we do a real read
    for( int x = 0; x < len+sizeof(unsigned int)*2; x++ )
        m_vector.pop_front();
    //m_vector.erase(m_vector.begin(), m_vector.begin()+length);
    
    pthread_mutex_unlock(&m_lock);
    return len;
}

bool VectorBuffer::pushData(const BYTE *refbuf, unsigned int length, unsigned int reqid)
{
    pthread_mutex_lock(&m_lock);

    // store length
    for( int x = 0; x < sizeof(unsigned int); x++ )
        m_vector.push_back(((unsigned char*)&length)[x]);
    
    // store reqid
    for( int x = 0; x < sizeof(unsigned int); x++ )
        m_vector.push_back(((unsigned char*)&reqid)[x]);
    
    while ( length-- > 0 )
        m_vector.push_back(*(refbuf++));
    
    pthread_mutex_unlock(&m_lock);
    return true;
}
