#!/bin/bash

# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2011-2014 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2011-2014 Lawrence Livermore National Laboratories, United States of America
#   2013-2014 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

i=0
for j in $(grep ' size' $1| sed -e 's|.*size="\([^"]*\)".*|\1|')
do 
	i=$[$i+$j]
done
echo $i
