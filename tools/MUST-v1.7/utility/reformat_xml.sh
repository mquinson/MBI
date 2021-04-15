#!/bin/bash

# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details


for i in ../specifications/*.xml
do
    if cat $i | grep -e '[^ ]' | sed -e 's/\t/ /g' -e 's/[ ][ ]*/ /g' | sed -e 's/> *</>\n</g' -e 's/[ ]*$//' | sed -e '{:q;N;s/\([^>]\)\n[ ]*/\1 /g;s/\([^>]\)$/\1/g;t q}' | xmlindent -i 4 -l 160 > $i.tmp ; then
        if diff $i $i.tmp > /dev/null; then
            rm $i.tmp
        else
            mv $i.tmp $i
            echo $i reformated
        fi
    fi
done
