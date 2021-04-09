/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#ifndef BIOBUFFER_H
#define	BIOBUFFER_H

#include <stdlib.h>
#include <unistd.h>

typedef unsigned char BYTE;
class BIOBuffer {
public:
    virtual BYTE* getHeadPtr() = 0;
    virtual int peekData(BYTE *copybuf, unsigned int length, unsigned int *remote_reqid) = 0;
    virtual int  readData(BYTE *copybuf, unsigned int length, unsigned int *remote_reqid) = 0;
    virtual bool pushData(const BYTE *refbuf, unsigned int length, unsigned int reqid) = 0;
    virtual bool IsEmpty() = 0;
    virtual ~BIOBuffer(void);
};

#endif	/* BIOBUFFER_H */

