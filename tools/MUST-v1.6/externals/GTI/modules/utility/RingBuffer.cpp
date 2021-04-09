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


#include "RingBuffer.h"

RingBuffer::RingBuffer(unsigned int size) : m_BufferSize(size), m_Full(false)
{
    pthread_mutex_init (&m_lock, NULL);
    m_Buffer = (BYTE*)malloc(size);
    // Set start position 
    m_Begin = m_End = m_Buffer;
    m_Full = false;
}

RingBuffer::~RingBuffer() {
    
}

BYTE* RingBuffer::getHeadPtr()
{
    return m_Begin;
}

int RingBuffer::peekDataInternal(BYTE *copybuf, unsigned int length, unsigned int *remote_req)
{
    if( getSizeInternal() <= 0)
        return 0;
    
    unsigned int offset = m_Begin - m_Buffer;
    unsigned int len = 0;
    for( int i = 0; i < sizeof(unsigned int); i++ )
        ((BYTE*)&len)[i] = *(m_Buffer + ((offset + i) % m_BufferSize));
 
    if(len > length)
        return 0;
    
    offset += sizeof(unsigned int);
    for( int i = 0; i < sizeof(unsigned int); i++ )
        ((BYTE*)remote_req)[i] = *(m_Buffer + ((offset + i) % m_BufferSize));
    
    offset += sizeof(unsigned int);
    for( int i = 0; i < len; i++ )
    {
        copybuf[i] = *(m_Buffer + ((offset + i) % m_BufferSize));
    }
    return len;
}

int RingBuffer::peekData(BYTE *copybuf, unsigned int length, unsigned int *remote_req)
{
    pthread_mutex_lock(&m_lock);
    int ret = peekDataInternal(copybuf, length, remote_req);
    pthread_mutex_unlock(&m_lock);
    return ret;
}

int RingBuffer::readData(BYTE *copybuf, unsigned int length, unsigned int *remote_req)
{
    pthread_mutex_lock(&m_lock);
    unsigned int len = peekDataInternal(copybuf, length, remote_req);
    if( len == 0 )
    {
        pthread_mutex_unlock(&m_lock);
        return 0;
    }

    m_Begin = m_Buffer + (((m_Begin - m_Buffer) + len + sizeof(unsigned int)*2) % m_BufferSize);
    m_Full = false;
    
    pthread_mutex_unlock(&m_lock);
    return len;
}

bool RingBuffer::pushData(const BYTE *refbuf, unsigned int length, unsigned int req)
{
    pthread_mutex_lock(&m_lock);
    if ( getSizeInternal() + length + sizeof(unsigned int)> m_BufferSize)
    {
        pthread_mutex_unlock(&m_lock);
        return false;
    }
    
    unsigned int offset = m_End - m_Buffer;
    
    // Store the current buffer position if requested..
    if( buffer_position != NULL )
        *buffer_position = m_End;
    
    for( int i = 0; i < sizeof(unsigned int); i++ )
        *(m_Buffer + ((offset + i) % m_BufferSize)) = ((BYTE*)&length)[i];
    
    offset += sizeof(unsigned int);
    for( int i = 0; i < sizeof(unsigned int); i++ )
        *(m_Buffer + ((offset + i) % m_BufferSize)) = ((BYTE*)&req)[i];
        
    offset += sizeof(unsigned int);
    for( int i = 0; i < length; i++ )
        *(m_Buffer + ((offset + i) % m_BufferSize)) = refbuf[i];
    
    m_End = m_Buffer + ((offset + length) % m_BufferSize);
    if( m_End == m_Begin )
        m_Full = true;
    
    pthread_mutex_unlock(&m_lock);
    return true;
}
