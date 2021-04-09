/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <stdio.h>

extern "C"
{
	int callA (int x, float y, int *z, char *w);
	int callB (int *x, int *y, int len);
	int callC (int x);
	int callD (int x);
}

int main (int argc, char ** argv)
{
    int i1[] = {1, 2};
    int i2[] = {123, 456, 789};


    callA (2, 77.77f, i1, "help!");
    callA (3, 33.33f, i2, "help me 2!");
    callB (i1, i1, 2);
    callB (i2, i2, 3);
    callC (5);
    callC (6);
    callD (123);

    return 0;
}
