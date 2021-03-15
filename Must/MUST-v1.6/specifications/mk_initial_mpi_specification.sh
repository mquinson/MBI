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

if [ -f ./gen_api_mpi_standard_3_nbc.h ]
then
    src=./gen_api_mpi_standard_3_nbc.h
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

###### HEADER
cat <<End-of-File >$tmp.xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE api-specification SYSTEM "@MUST_SPECIFICATION_DTD_PATH@/api-specification.dtd">
<api-specification unique-name="MPI">
	<api-headers><header>mpi.h</header></api-headers>
    <functions>
End-of-File

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
  
  printf "\t\t<function name=\"%s\" return-type=\"%s\">\n", callname, retval
  
  #####The arguments
  printf "\t\t\t<function-arguments>\n"
  for (i=1; i <= num_paras; i++)
  {
    ###Array or not ?
    nodeName="function-argument"
    if (index(extra[i],"ARRAY") != 0)
    	nodeName="function-array-argument"
    	
    ###What intent
    intent="in"
    if ((index(extra[i],"SINGLE_OUT") != 0) ||
        (index(extra[i],"ARRAY_OUT") != 0))
    	intent="out"
    if ((index(extra[i],"SINGLE_IO") != 0) ||
        (index(extra[i],"ARRAY_IO") != 0))
    	intent="inout"
    	
    ###print function node start
	if (index(type[i],"const_") != 0)
	{
		sub(/_/, " ", type[i])
	}
	
	if (index(type[i],"unsigned_") != 0)
	{
		sub(/_/, " ", type[i])
	}

	
    printf "\t\t\t\t<%s name=\"%s\" type=\"%s\" intent=\"%s\" order=\"%d\">", nodeName,  para[i], type[i], intent, i-1  
    
    ###print length argument
    if (nodeName == "function-array-argument")
    {
    	lenType="argument"
    	if (index(extra[i],"|OP:") != 0)
    		lenType="operation"
        
    	printf "\t\t\t\t\t<length-argument type=\"%s\">\n", lenType
    	
    	if (lenType=="argument")
    	{
    		//It is an argument length arg
    		split(extra[i],lentemp,"|ARG:")
	        arg=substr(lentemp[2], 1, length(lentemp[2])-1)
	        
	        printf "\t\t\t\t\t\t<call-arg-name>%s</call-arg-name>\n", arg    		
    	}
    	else
    	{
    		//It is an operation length arg
    		split(extra[i],lentemp,"|OP:")
    		num=split(lentemp[2],lentemp2,":")
	        lentemp2[num]=substr(lentemp2[num], 1, length(lentemp2[num])-1)
	        
	        id=getOpId(num_paras, extra, extra[i])
	        
	        printf "\t\t\t\t\t\t<op-name group=\"MPI_Base\" id=\"%d\">%s</op-name>\n", id, lentemp2[1]
	        
	        ##Debug
	        #printf "OP: %s Num:%d ", lentemp2[1], num
	        #for (k=2;k<=num;k++)
	        #   printf " Arg: %s", lentemp2[k]
	        #printf "\n"
    		
    	}
    	
    	printf "\t\t\t\t\t</length-argument>\n"
    }
    
    ###print function node end
    printf "</%s>\n", nodeName 
  }
  printf "\t\t\t</function-arguments>\n"
  
  ####The analyses
  printf "\t\t\t<analyses>\n"
  ##Add default analyses
  printf "\t\t\t\t<analysis order=\"pre\" name=\"InitParallelId:init\" group=\"MUST_Base\">\n"
  printf "\t\t\t\t\t<analysis-arguments>\n"
  printf "\t\t\t\t\t\t<analysis-argument type=\"operation\" analysis-arg-order=\"0\">\n"
  printf "\t\t\t\t\t\t\t<op-name group=\"MUST_Base\" id=\"1\">provideParallelIdStorage</op-name>\n"
  printf "\t\t\t\t\t\t</analysis-argument>\n"
  printf "\t\t\t\t\t</analysis-arguments>\n"
  printf "\t\t\t\t</analysis>\n"
  printf "\t\t\t\t<analysis order=\"pre\" name=\"InitLocationId:init\" group=\"MUST_Base\">\n"
  printf "\t\t\t\t\t<analysis-arguments>\n"
  printf "\t\t\t\t\t\t<analysis-argument type=\"operation\" analysis-arg-order=\"0\">\n"
  printf "\t\t\t\t\t\t\t<op-name group=\"MUST_Base\" id=\"1\">provideLocationIdStorage</op-name>\n"
  printf "\t\t\t\t\t\t</analysis-argument>\n"
  printf "\t\t\t\t\t\t<analysis-argument type=\"mapped\" analysis-arg-order=\"1\">\n"
  printf "\t\t\t\t\t\t\t<call-arg-name>USE_CALLNAME</call-arg-name>\n"
  printf "\t\t\t\t\t\t</analysis-argument>\n"
  printf "\t\t\t\t\t\t<analysis-argument type=\"mapped\" analysis-arg-order=\"2\">\n"
  printf "\t\t\t\t\t\t\t<call-arg-name>USE_CALLID</call-arg-name>\n"
  printf "\t\t\t\t\t\t</analysis-argument>\n"
  printf "\t\t\t\t\t</analysis-arguments>\n"
  printf "\t\t\t\t</analysis>\n"
  printf "\t\t\t\t<analysis order=\"pre\" name=\"MessageReproducer:testForMatch\" group=\"MUST_Base\">\n"
  printf "\t\t\t\t\t<analysis-arguments>\n"
  printf "\t\t\t\t\t\t<analysis-argument type=\"operation-len\" analysis-arg-order=\"0\">\n"
  printf "\t\t\t\t\t\t\t<op-name group=\"MUST_Base\" id=\"1\">provideParallelIdStorage</op-name>\n"
  printf "\t\t\t\t\t\t</analysis-argument>\n"
  printf "\t\t\t\t\t\t<analysis-argument type=\"operation-len\" analysis-arg-order=\"1\">\n"
  printf "\t\t\t\t\t\t\t<op-name group=\"MUST_Base\" id=\"1\">provideLocationIdStorage</op-name>\n"
  printf "\t\t\t\t\t\t</analysis-argument>\n"
  printf "\t\t\t\t\t</analysis-arguments>\n"
  printf "\t\t\t\t</analysis>\n"
  printf "\t\t\t</analyses>\n"
  
  #####The operations
  printf "\t\t\t<operations>\n"
  n=getKnowns(num_paras, extra)
  
  ###First the ops for array length arguments
  for (v=1;v<=n;v++)
  {
  	num=split(known[v],lentemp,":")
  	
  	order="pre"
  	
  	#Is it post order ?
  	for (k=2; k<=num; k++)
  	{
  		 #search function argument for op input
  		 found=0
  		 for (i=1; i <= num_paras; i++)
  		 {
  		 	if (para[i] == lentemp[k])
  		 	{
  		 		if ((index(extra[i],"SINGLE_OUT") != 0) ||
        			(index(extra[i],"ARRAY_OUT") != 0))
    				order="post"
    			if (((index(extra[i],"SINGLE_IO") != 0) ||
        			 (index(extra[i],"ARRAY_IO") != 0))    &&
        			(order=="pre"))
    				order="UNKNOWN"
				found=1  		 		
  		 	}
  		 }
  		 
  		 if (found==0)
  		 	printf "ERROR!\n" 
  	}
  	
  	printf "\t\t\t\t<operation group=\"MPI_Base\" name=\"%s\" order=\"%s\" id=\"%d\">\n", lentemp[1], order, v 
  	printf "\t\t\t\t\t<operation-arguments>\n"
  	
  	for (k=2; k<=num; k++)
  	{
  		printf "\t\t\t\t\t\t<operation-argument op-arg-order=\"%d\">\n", k-2
  		printf "\t\t\t\t\t\t\t<call-arg-name>%s</call-arg-name>\n", lentemp[k]  		
  		printf "\t\t\t\t\t\t</operation-argument>\n"
  	}
  	
  	printf "\t\t\t\t\t</operation-arguments>\n"
  	printf "\t\t\t\t</operation>\n"
  }
  
  ###Second the ops for MPI-Handle to int conversion
  for (i=1; i <= num_paras; i++)
  {
    ###Test for all MPI handles    
    handleName=""
    plural="s"    

    if (index(type[i],"MPI_Comm") != 0)
    	handleName="MPI_Comm"
	
	if (index(type[i],"MPI_Datatype") != 0)
    	handleName="MPI_Datatype"
	
	if (index(type[i],"MPI_Request") != 0)
    	handleName="MPI_Request"
    	
    if (index(type[i],"MPI_Status") != 0)
    {
    	handleName="MPI_Status"
    	plural="es"
    }
	
	if (index(type[i],"MPI_Op") != 0)
    	handleName="MPI_Op"
	
	if (index(type[i],"MPI_Errhandler") != 0)
    	handleName="MPI_Errhandler"
	
	if (index(type[i],"MPI_Group") != 0)
    	handleName="MPI_Group"
    	
    if (handleName != "")
    {
        ###Build a list of extra input arguments besides this argument (i.e. length arguments, or arguments from length ops)
        numExtraInput=0
        
        if (index (extra[i], "ARRAY") != 0)
    	{
    		lenType="argument"
    		if (index(extra[i],"|OP:") != 0)
    			lenType="operation"
    			
    		if (lenType=="argument")
    		{
    			//It is an argument length arg
    			split(extra[i],lentemp,"|ARG:")
		        arg=substr(lentemp[2], 1, length(lentemp[2])-1)
		        extraInput[1]=arg
		        numExtraInput=1
			}
    		else
    		{
    			//It is an operation length arg
    			split(extra[i],lentemp,"|OP:")
    			num=split(lentemp[2],lentemp2,":")
	        	lentemp2[num]=substr(lentemp2[num], 1, length(lentemp2[num])-1)
	        
	        	for (x=2; x <= num; x++)
	        	{
	        		numExtraInput++
	        		extraInput[numExtraInput]=lentemp2[x]
	        	}
	        }
    	}
    
    	###Determine Operation name
    	split(handleName, temp, "MPI_")
    	opName=temp[2]
    	if (index(extra[i], "ARRAY") != 0)
        	opName="convert"opName plural "2int"
    	else
    		opName="convert"opName"2int"
    	
    	if ( (index(type[i], "*") != 0) &&
    	     (index(extra[i], "ARRAY") == 0) )
    	    opName="deref_"opName
    	
    	#Special case for array of statuses with deref 
    	if (index(extra[i], "|OP:deref") != 0)
    	    opName="deref_"opName
    	    
    	###Determine Order
    	order="pre"
    	
    	#IMPORTANT: we quietly assume that we only care about "pre"
    	#           for IO arguments, which should hold pretty well
    	#           for common MPI checks ...
    	if ( (index (extra[i], "SINGLE_OUT") != 0) ||
    	     (index (extra[i], "ARRAY_OUT") != 0)    )
    	     order="post"
    	    
    	for (x = 1; x <= numExtraInput; x++)
    	{
    		for (j = 1; j <= num_paras; j++)
    		{
    			if (para[j] == extraInput[x])
    			{
	    			if ( (index (extra[j], "SINGLE_OUT") != 0) ||
	    	     		 (index (extra[j], "ARRAY_OUT") != 0)    )
	    	     		order="post"
	    	    }
    		} 
    	} 
   	
    	###print XML
    	numSuffixes=1
    	suffixes[1] = ""
    	
    	if (handleName == "MPI_Status")
    	{
    		numSuffixes=3
    		suffixes[1] = "Source"
    		suffixes[2] = "Tag"
    		suffixes[3] = "Error"
    	}
    	
    	for (suf = 1; suf <= numSuffixes; suf++)
    	{
    		printf "\t\t\t\t<operation group=\"MPI_Base\" name=\"%s%s\" order=\"%s\" id=\"%d\">\n", opName, suffixes[suf], order, i
    	
    		printf "\t\t\t\t\t<operation-arguments>\n"
    	
    		#the argument to convert
    		printf "\t\t\t\t\t<operation-argument op-arg-order=\"0\">\n"
    		printf "\t\t\t\t\t\t<call-arg-name>%s</call-arg-name>\n", para[i]
    		printf "\t\t\t\t\t</operation-argument>\n"
    	
    		#further arguments
    		for (x = 1; x <= numExtraInput; x++)
    		{
    			printf "\t\t\t\t\t<operation-argument op-arg-order=\"%d\">\n", x
    			printf "\t\t\t\t\t\t<call-arg-name>%s</call-arg-name>\n", extraInput[x]
    			printf "\t\t\t\t\t</operation-argument>\n"	 
    		} 
    	
    		printf "\t\t\t\t\t</operation-arguments>\n"    	
    		printf "\t\t\t\t</operation>\n"    
		}
    }
  }
  
  ###Third default ops for parallel ID and source location
  printf "\t\t\t\t<operation group=\"MUST_Base\" name=\"provideLocationIdStorage\" order=\"pre\" id=\"1\">\n"
  printf "\t\t\t\t\t<operation-arguments>\n"
  printf "\t\t\t\t\t\t<operation-argument op-arg-order=\"0\">\n"
  printf "\t\t\t\t\t\t\t<call-arg-name>USE_CALLNAME</call-arg-name>\n"
  printf "\t\t\t\t\t\t</operation-argument>\n"
  printf "\t\t\t\t\t\t<operation-argument op-arg-order=\"1\">\n"
  printf "\t\t\t\t\t\t\t<call-arg-name>USE_CALLID</call-arg-name>\n"
  printf "\t\t\t\t\t\t</operation-argument>\n"
  printf "\t\t\t\t\t</operation-arguments>\n"
  printf "\t\t\t\t</operation>\n"
  printf "\t\t\t\t<operation group=\"MUST_Base\" name=\"provideParallelIdStorage\" order=\"pre\" id=\"1\">\n"
  printf "\t\t\t\t\t<operation-arguments />\n"
  printf "\t\t\t\t</operation>\n"
    
  ###Finaly the operations for calculating argument ids
  for (i=1; i <= num_paras; i++)
  {
	printf "\t\t\t\t<operation group=\"MUST_Base\" name=\"buildArgumentId_%d_%s\" order=\"pre\" id=\"1\">\n", i, para[i]
	printf "\t\t\t\t\t<operation-arguments></operation-arguments>\n"    	
	printf "\t\t\t\t</operation>\n"		
  }
  
  #DEBUG
  #printf "KNOWN: "
  #for (v=1;v<=n;v++)
  # 	printf "Known: %s", known[v]
  #printf "\n"
    
  printf "\t\t\t</operations>\n"
  
  printf "\t\t</function>\n" 
}

# /*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/
# getOpId
# 
# Returns the "right" id for an op
# /*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/
function getOpId(num_paras, extra, op)
{
    #Create array with one entry for each DISTINCT op
    num_known=0
	for (z=1; z <= num_paras; z++)
	{
		x=split(extra[z],lentemp,"|OP:")
		
		if (x >= 2)
		{
		    opStr=substr(lentemp[2], 1, length(lentemp[2])-1)
		    found=0
			for (w=1; w <= num_known;w++)
			{
				if (known[w] == opStr)
					found=1
			}
			
			if (found==0)
			{
				#its new add it to known
				num_known++
				known[num_known]=opStr
			}
			
		}
	}
	
	#Loop over knowns and return the id of the known
	x=split(op,lentemp,"|OP:")
	opStr=substr(lentemp[2], 1, length(lentemp[2])-1)
	
	for (w=1; w <= num_known;w++)
	{
		if (known[w] == opStr)
			return w;
	}
	
	##DEBUG
	#printf "Known== num:%d ", num_known
	#for (z=1; z <= num_known; z++)
	#	printf " known: %s", known[z]
		
	return "ERROR"
}

# /*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/
# getKnowns
# 
# Returns a list of all unique operations for a
# list or argument extra specifiers
# Stored in "known"
# /*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/*/
function getKnowns(num_paras, extra)
{
    #Create array with one entry for each DISTINCT op
    num_known=0
	for (z=1; z <= num_paras; z++)
	{
		x=split(extra[z],lentemp,"|OP:")
		
		if (x >= 2)
		{
		    opStr=substr(lentemp[2], 1, length(lentemp[2])-1)
			for (w=1; w <= num_known;w++)
			{
				if (known[w] == opStr)
					break;
			}
			
			if (w == num_known + 1)
			{
				#its new add it to known
				num_known++
				known[num_known]=opStr
			}
			
		}
	}
	
	return num_known
}
'
cat <<End-of-File >>$tmp.xml
	</functions>
</api-specification>
End-of-File

gawk -f $tmp.nawk $tmp.xml > initial_mpi_specification.xml

exit
