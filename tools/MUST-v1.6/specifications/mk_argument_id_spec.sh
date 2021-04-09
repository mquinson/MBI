#! /bin/bash

# This file is part of MUST (Marmot Umpire Scalable Tool)
#
# Copyright (C)
#   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
#   2010-2018 Lawrence Livermore National Laboratories, United States of America
#   2013-2018 RWTH Aachen University, Federal Republic of Germany
#
# See the LICENSE file in the package base directory for details

# TODO:
#      * Many constants still need to be converted, e.g.:
#          -MPI_ANY_SOURCE, MPI_ANY_TAG, ...
#

tmp=tmp$$
trap "rm -f $tmp.*; exit" 0 1 2 3 9


if [ -f ./gen_api_mpi_standard_1_2.h ]
then
    src=./gen_api_mpi_standard_1_2.h
else
    echo "$0: Error: no standard header found!"
    exit 1
fi

cat <<End-of-File >$tmp.nawk
/\\=/ { print; next }
End-of-File
gawk </dev/null >>$tmp.nawk '
/^#/            { next }
$4 == "-"       { next }
                { print "/ " $4 "\\(/ { state=\"" $2 "\" }" }'
cat <<End-of-File >>$tmp.nawk
/MPI_PCP_OTHER/ { if (state != "") sub("OTHER", state)
                  state = ""
                }
                { print }
End-of-File

grep -e ' MPI_.*(.*)' $src \
| sed >$tmp.tmp \
    -e '/typedef /d' \
    -e 's/( *void *)/()/' \
    -e 's/   */ /g' \
	-e 's/const /const_/g' \
    -e 's/unsigned /unsigned_/g' \
    -e 's/ /,/' \
    -e 's/(/,/' \
    -e 's/);//' \
    -e 's/, /,/g' \
    -e 's/,$//' \
    -e '/,MPI_Pcontrol,/d' \
    -e '/,MPI_Wtick/d' \
    -e '/,MPI_Wtime/d' \
    -e '/,MPI_Group_range_excl/d' \
    -e '/,MPI_Group_range_incl/d' \

###### HEADER
cat <<End-of-File >$tmp.xml
<!-- Todo add this to the file must_base_specification.xml -->
End-of-File

rm -f temp.temp
touch temp.temp

gawk -F, <$tmp.tmp >>$tmp.xml '
{
# C wrapper
#
# $1 is the return type
# $2 is the call name

  callname=$2
  retval=$1

  #Prepare Arguments  
  num_paras=0
  if (NF == 2)
  {
    #No arguments
  }
  else 
  {
      for (i=3; i<=NF; i++) 
      {
      	  split($i,typeandpara," ")
          type[i-2]=typeandpara[1]
          para[i-2]=typeandpara[2]
          extra[i-2]=typeandpara[3]
          num_paras++
      }
  }
  
  ####DEBUG
  #printf "==== %s %s ====\n", retval, callname
  #for (i=1; i <= num_paras; i++)
  #	printf "%s %s %s\n", type[i], para[i], extra[i]
  
  numKnowns=0;
    
  for (i=1; i <= num_paras; i++)
  {
    if (index(type[i],"const_") != 0)
	{
		sub(/_/, " ", type[i])
	}
	if (index(type[i],"unsigned_") != 0)
	{
		sub(/_/, " ", type[i])
	}
  
	x=system ("grep AAA"i"_"para[i]"AAA temp.temp >> /dev/null")
	if (x == 0)
		continue;

	system ("echo AAA"i"_"para[i]"AAA >> temp.temp")

	printf "\t<operation return-type=\"int\" name=\"buildArgumentId_%d_%s\">\n", i, para[i] 
	printf "\t\t<extra-headers>\n"
	printf "\t\t\t<header>BaseIds.h</header>\n"
	printf "\t\t</extra-headers>\n"
	printf "\t\t<operation-arguments>\n"
	printf "\t\t</operation-arguments>\n"
	printf "\t\t<source-template>\n"
	printf "int RETURN = MUST_ARGUMENT_%s + (%d &lt;&lt; 24);\n", toupper (para[i]), i
	printf "\t\t</source-template>\n"
	printf "\t</operation>\n"
  }
}'
cat <<End-of-File >>$tmp.xml
<!-- END -->
End-of-File

gawk -f $tmp.nawk $tmp.xml > gen_input_must_base_specification.xml

exit
