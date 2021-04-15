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
 * @file GtiRecordGenImplementation.cpp
 *       Record Generation Interface implementation with a mainly generated record.
 *
 * @ref RecordGenerator - Learn more about the Record Generator
 *
 * @author Tobias Hilbrich
 * @date 11.12.2009
 */

#include <assert.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <iostream>
#include <string>

#include "GtiRecordGenImplementation.h"

using namespace gti;

//=======================================
//          Function to get an instance from the shared library
//=======================================
extern "C" I_RecordGenerator* getImplementation (void)
{
    return new GtiRecordGenImplementation ();
}

//=======================================
//			GtiRecordGenImpl
//=======================================
//---------------------------------------
// GtiRecordArgument
//---------------------------------------
GtiRecordArgument::GtiRecordArgument (
    			std::string name,
    			std::string type,
    			std::string fromCall,
    			std::string asArg
    			)
	:	name (name),
		type (type),
		fromCall (fromCall),
		asArg (asArg),
		isArray (false),
		lengthArgument ("")
{

}

//---------------------------------------
// GtiRecordArgument
//---------------------------------------
GtiRecordArgument::GtiRecordArgument (
    			std::string nameArray,
    			std::string type,
    			std::string fromCall,
    	    	std::string asArg,
    	    	std::string lengthArgument
    	    	)
	:	name (nameArray),
		type (type),
		fromCall (fromCall),
		asArg (asArg),
		isArray (true),
		lengthArgument (lengthArgument)
{

}

//---------------------------------------
// operator ==
//---------------------------------------
bool GtiRecordArgument::operator == (GtiRecordArgument const& other) const
{
	return other.compare (name, type, fromCall, asArg, isArray, lengthArgument);
}

//---------------------------------------
// operator !=
//---------------------------------------
bool GtiRecordArgument::operator != (GtiRecordArgument const& other) const
{
	return !other.compare (name, type, fromCall, asArg, isArray, lengthArgument);
}

//---------------------------------------
// compare
//---------------------------------------
bool GtiRecordArgument::compare (
    			std::string name,
    			std::string type,
    			std::string fromCall,
    			std::string asArg,
    			bool isArray,
    			std::string lengthArgument
    			) const
{
	if (this->name != name)
		return false;
	if (this->type != type)
		return false;
	if (this->fromCall != fromCall)
		return false;
	if (this->asArg != asArg)
		return false;
	if (this->isArray != isArray)
		return false;
	if (this->lengthArgument != lengthArgument)
		return false;

	return true;
}

//=======================================
//			GtiRecordGenImpl
//=======================================
//---------------------------------------
// GtiRecordCommittedDesc
//---------------------------------------
GtiRecordCommittedDesc::GtiRecordCommittedDesc ()
	:	myArgs (),
		myNeedsHeaderDefinition (false),
		myNeedsSerialize (false),
		myNeedsDeserialize (false),
		myDefinitionNames (),
		mySerializeUids (),
		myDeserializeUids ()
{

}

//---------------------------------------
// ~GtiRecordCommittedDesc
//---------------------------------------
GtiRecordCommittedDesc::~GtiRecordCommittedDesc ()
{
	myArgs.clear ();
}

//---------------------------------------
// operator ==
//---------------------------------------
bool GtiRecordCommittedDesc::operator == (GtiRecordCommittedDesc const& other) const
{
	return other.compare (myArgs);
}

//---------------------------------------
// compare
//---------------------------------------
bool GtiRecordCommittedDesc::compare (std::list<GtiRecordArgument> args) const
{
	std::list<GtiRecordArgument>::const_iterator iter1, iter2;



	for (iter1 = args.begin (), iter2 = this->myArgs.begin();
		 iter1 != args.end () && iter2 != this->myArgs.end ();
		 iter1++, iter2++)
	{
		if (*iter1 != *iter2)
			return false;
	}

	if (iter1 != args.end () ||
		iter2 != this->myArgs.end ())
		return false;

	return true;
}

//---------------------------------------
// addArgument
//---------------------------------------
GTI_RETURN GtiRecordCommittedDesc::addArgument (GtiRecordArgument arg)
{
	std::list<GtiRecordArgument>::iterator iter;

	bool lengthArgFound = false;

	//Check for validity (uniqueness, lengthArgument exists)
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->name == arg.name)
			return GTI_ERROR;

		if (arg.isArray)
			if (iter->name == arg.lengthArgument)
				lengthArgFound = true;
	}

	if (arg.isArray && !lengthArgFound)
		return GTI_ERROR;

	//add the argument
	myArgs.push_back (arg);

	return GTI_SUCCESS;
}

//---------------------------------------
// GtiRecordGenImpl
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenGetStructName (uint64_t uid)
{
	std::map <uint64_t, std::string>::iterator iter;
	iter = myDefinitionNames.find (uid);

	if (iter == myDefinitionNames.end ())
	{
		char buf[128];
		sprintf (buf, "%" PRIu64, uid);
		std::string str ("GtiRecord_");
		str += buf;

		myDefinitionNames.insert (std::make_pair (uid, str));
		myNeedsHeaderDefinition = true;

		return str;
	}

	return iter->second;
}


//---------------------------------------
// GtiRecordGenImpl
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenInitArgs (std::string varName)
{
	std::string ret = "";
	std::list<GtiRecordArgument>::iterator iter;
	std::map<std::string, bool> initedLenArgs;

	//loop over args and initialize all array ones
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->isArray)
		{
			if (initedLenArgs.find (iter->lengthArgument) == initedLenArgs.end())
			{
				//length argument needs to be initialized
				ret += varName + "." + iter->lengthArgument + " = 0;\n";
				initedLenArgs.insert (std::make_pair(iter->lengthArgument,true));
			}

			//init array argument
			ret += varName + "." + iter->name + " = NULL;\n";
		}
	}

	return ret;
}

//---------------------------------------
// frontendGenFreeArgs
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenFreeArgs (std::string varName)
{
	std::string ret = "";
	std::list<GtiRecordArgument>::iterator iter;

	//loop over args and initialize all array ones
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->isArray)
		{
			//free array argument
			ret += "if (" + varName + "." + iter->name + ")\n" +
				   "    free (" + varName + "." + iter->name + ");\n" +
				   varName + "." + iter->name + "= NULL;\n";
		}
	}

	return ret;
}

//---------------------------------------
// frontendGenWriteArgument
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenWriteArgument (
		std::string varName,
		std::string argument,
		std::string value)
{
	std::string ret = "";
	std::list<GtiRecordArgument>::iterator iter, iter2;

	//make sure the argument exists and is valid !
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->name == argument)
		{
			//can't write an array argument!
			assert (!iter->isArray);

			//write
			ret = varName + "." + argument + " = " + value + ";\n";

			//resize all array arguments that use this argument
			// as length argument
			for (iter2 = myArgs.begin(); iter2 != myArgs.end(); iter2++)
			{
				if (iter2->isArray &&
					iter2->lengthArgument == argument)
				{
					ret += "if (" + varName + "." + iter2->name + ")\n";
					ret += "    free (" + varName + "." + iter2->name + ");\n";
					ret += "if (" + varName + "." + argument + ")\n";
					ret += "    " + varName + "." + iter2->name + " = (" + iter2->type + "*) malloc (sizeof(" + iter2->type + ")*((" + varName + "." + argument + "+7)/8)*8);\n";
					ret += "else\n";
					ret += "    " + varName + "." + iter2->name + " = NULL;\n";
				}
			}

			break;
		}
	}

	//make sure the named argument was found!
	assert (ret != "");

	return ret;
}

//---------------------------------------
// frontendGenwriteArrayArgument
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenWriteArrayArgument (
    			std::string varName,
    			std::string argument,
    			std::string valueArray)
{
	std::string ret = "";
	std::list<GtiRecordArgument>::iterator iter;

	//make sure the argument exists and is valid !
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->name == argument)
		{
			//can't write a single argument!
			assert (iter->isArray);

			//write
			ret += "for (uint64_t i = 0; i < " + varName + "." + iter->lengthArgument + "; i++)\n";
			ret += "    " + varName + "." + argument + "[i] = " + valueArray + "[i];\n";

			break;
		}
	}

	//make sure the named argument was found!
	assert (ret != "");

	return ret;
}

//---------------------------------------
// frontendGenwriteArrayArgumentIndexed
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenWriteArrayArgumentIndexed (
    			std::string varName,
    			std::string argument,
    			std::string value,
    			std::string index)
{
	std::string ret = "";
	std::list<GtiRecordArgument>::iterator iter;

	//make sure the argument exists and is valid !
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->name == argument)
		{
			//can't write a single argument!
			assert (iter->isArray);

			//write
			ret += varName + "." + argument + "[" + index + "] = " + value + ";\n";

			break;
		}
	}

	//make sure the named argument was found!
	assert (ret != "");

	return ret;
}

//---------------------------------------
// frontendGenreturnArgument
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenReturnArgument (std::string varName, std::string argument)
{
	std::string ret = "";
	std::list<GtiRecordArgument>::iterator iter;

	//make sure the argument exists and is valid !
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->name == argument)
		{
			//can't write an array argument!
			assert (!iter->isArray);

			//return value
			ret = varName + "." + argument;

			break;
		}
	}

	//make sure the named argument was found!
	assert (ret != "");

	return ret;
}

//---------------------------------------
// frontendGenreturnArrayArgument
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenReturnArrayArgument (std::string varName, std::string argument)
{
	std::string ret = "";
	std::list<GtiRecordArgument>::iterator iter;

	//make sure the argument exists and is valid !
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->name == argument)
		{
			//can't write an array argument!
			assert (iter->isArray);

			//return value
			ret = varName + "." + argument;

			break;
		}
	}

	//make sure the named argument was found!
	assert (ret != "");

	return ret;
}

//---------------------------------------
// frontendGenreturnArrayArgumentIndexed
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenReturnArrayArgumentIndexed (
    			std::string varName,
    			std::string argument,
    			std::string index)
{
	std::string ret = "";
	std::list<GtiRecordArgument>::iterator iter;

	//make sure the argument exists and is valid !
	for (iter = myArgs.begin(); iter != myArgs.end(); iter++)
	{
		if (iter->name == argument)
		{
			//can't write an array argument!
			assert (iter->isArray);

			//return value
			ret = varName + "." + argument + "[" + index + "]";

			break;
		}
	}

	//make sure the named argument was found!
	assert (ret != "");

	return ret;
}

//---------------------------------------
// frontendGenSerializeCall
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenSerializeCall (
    			uint64_t uid,
    			std::string varName,
    			std::string serBufOutName,
    			std::string numBytesOutName)
{
	std::string ret = "";

	std::map <uint64_t, bool>::iterator iter;
	iter = mySerializeUids.find (uid);
	if (iter == mySerializeUids.end())
		mySerializeUids.insert (std::make_pair(uid,true));
	myNeedsSerialize = true;

	char buf[256];
	sprintf (buf, "%" PRIu64, uid);
	ret += "GtiRecordSerialize_" + (std::string)buf + "(";
	ret += "&" + varName + ", &" + serBufOutName + ", &" + numBytesOutName + ");\n";

	return ret;
}

//---------------------------------------
// frontendGenDeserializeCall
//---------------------------------------
std::string GtiRecordCommittedDesc::frontendGenDeserializeCall (
    			uint64_t uid,
    			std::string varNameOutRecord,
    			std::string serBufName,
    			std::string serBufLength)
{
	std::string ret = "";

	std::map <uint64_t, bool>::iterator iter;
	iter = myDeserializeUids.find (uid);
	if (iter == myDeserializeUids.end())
		myDeserializeUids.insert (std::make_pair(uid,true));
	myNeedsDeserialize = true;

	char buf[256];
	sprintf (buf, "%" PRIu64, uid);
	ret += "GtiRecordDeserialize_" + (std::string)buf + "(";
	ret += "&" + varNameOutRecord + ", " + serBufName + ", " + serBufLength + ");\n";

	return ret;
}

//---------------------------------------
// backendGenCreateHeaderDefinitions
//---------------------------------------
std::string GtiRecordCommittedDesc::backendGenCreateHeaderDefinitions (void)
{
	std::string ret = "";

	//needed ?
	if (!myNeedsHeaderDefinition)
		return ret;

	//loop over all stored uid's for which we need a definition
	std::map <uint64_t, std::string>::iterator iter;
	for (iter = myDefinitionNames.begin (); iter != myDefinitionNames.end (); iter++)
	{
		//open struct definition
		ret += "struct t" + iter->second + "\n";
		ret += "{\n";

		//add all arguments to struct
		std::list<GtiRecordArgument>::iterator argIter;
		for (argIter = myArgs.begin(); argIter != myArgs.end(); argIter++)
		{
			if (argIter->isArray)
				ret += "    " + argIter->type + " *" + argIter->name + ";\n";
			else
				ret += "    " + argIter->type + " " + argIter->name + ";\n";
		}
				/*
				 * We add a dummy entry to avoid the use of empty structs, some compilers don't like them (e.g., Cray compiler)
				 */
                if (myArgs.empty()){
                        ret += "    int dummy;\n";
                }

		//close struct
		ret += "};\n\n";

		//create typedef
		ret += "typedef struct t" + iter->second + " " + iter->second + ";\n\n";
	}

	//start extern "C"
	ret += "#if defined(__cplusplus)\n";
	ret += "extern \"C\" {\n";
	ret += "#endif\n";

	//create declaration for serialize functions
	std::map <uint64_t, bool>::iterator iter2;
	for (iter2 = mySerializeUids.begin(); iter2 != mySerializeUids.end(); iter2++)
	{
		uint64_t uid = iter2->first;

		char buf[256];
		sprintf (buf, "%" PRIu64, uid);
		ret += "void GtiRecordSerialize_" + (std::string)buf + " (";
		ret += "GtiRecord_" + (std::string)buf + " *record, void** out_buf, uint64_t* out_len);\n\n";
	}

	//create declaration for deserialize functions
	for (iter2 = myDeserializeUids.begin(); iter2 != myDeserializeUids.end(); iter2++)
	{
		uint64_t uid = iter2->first;

		char buf[256];
		sprintf (buf, "%" PRIu64, uid);
		ret += "void GtiRecordDeserialize_" + (std::string)buf + " (";
		ret += "GtiRecord_" + (std::string)buf + " *out_record, void* buf, uint64_t len);\n\n";

	}

	//end extern "C"
	ret += "#if defined(__cplusplus)\n";
	ret += "}\n";
	ret += "#endif\n";

	return ret;
}

//---------------------------------------
// backendGenCreateSource
//---------------------------------------
std::string GtiRecordCommittedDesc::backendGenCreateSource (void)
{
	std::string ret = "";

	//needed ?
	if (!myNeedsSerialize && !myNeedsDeserialize)
		return ret;

	//create a buffer size calculator first
	std::map <uint64_t, bool>::iterator iter;
	for (iter = mySerializeUids.begin(); iter != mySerializeUids.end(); iter++)
	{
		uint64_t uid = iter->first;

		//start the define
		char buf[256];
		sprintf (buf, "%" PRIu64, uid);
		ret += "#define GtiRecord_size_define_" + (std::string)(buf);
		ret += " sizeof(uint64_t)"; //used to store uid!

		//add length of arguments to define
		std::list<GtiRecordArgument>::iterator argIter;
		for (argIter = myArgs.begin(); argIter != myArgs.end(); argIter++)
		{
			if(argIter->isArray)
				ret += "+ sizeof(" + argIter->type + ")*record->" + argIter->lengthArgument;
			else
				ret += "+ sizeof(" + argIter->type + ")";
		}

		ret += "\n\n";
	}

	//do the serialize functions second
	for (iter = mySerializeUids.begin(); iter != mySerializeUids.end(); iter++)
	{
		uint64_t uid = iter->first;

		//start the serialize function
		char buf[256];
		sprintf (buf, "%" PRIu64, uid);
		ret += "void GtiRecordSerialize_" + (std::string)buf + " (";
		ret += "GtiRecord_" + (std::string)buf + " *record, void** out_buf, uint64_t* out_len)\n";
		ret += "{\n";
		ret += "    uint64_t buf_len = GtiRecord_size_define_" + (std::string)buf + ";\n";
		ret += "    char *temp_buf = (char*) malloc (((buf_len+7)/8)*8);\n";
		ret += "    size_t i = 0;\n";
		ret += "    \n";
		ret += "    uint64_t uid = " + (std::string)buf + ";\n";
		ret += "    memmove (temp_buf+i, &uid, sizeof(uint64_t));\n";
		ret += "    i += sizeof (uint64_t);\n";
		ret += "    \n";

		//serialize all the args
		std::list<GtiRecordArgument>::iterator argIter;
		for (argIter = myArgs.begin(); argIter != myArgs.end(); argIter++)
		{
			if (argIter->isArray)
			{
				ret += "    if (record->" + argIter->lengthArgument + ">0) memmove (temp_buf+i, record->" + argIter->name +
             ", sizeof (" + argIter->type + ")*record->" + argIter->lengthArgument + ");\n";
				ret += "    i += sizeof(" + argIter->type + ")*record->" + argIter->lengthArgument + ";\n";
			}
			else
			{
				ret += "    memmove (temp_buf+i, &(record->" + argIter->name +
               "), sizeof (" + argIter->type + "));\n";
				ret += "    i += sizeof(" + argIter->type + ");\n";
			}
		}

		//finish the serialize function
		ret += "    assert (i == buf_len);\n";
		ret += "    *out_buf = temp_buf;\n";
		ret += "    *out_len = i;\n";
		ret += "}\n\n";
	}

	//do the deserialize function last
	for (iter = myDeserializeUids.begin(); iter != myDeserializeUids.end(); iter++)
	{
		uint64_t uid = iter->first;

		//start the deserialize function
		char buf[256];
		sprintf (buf, "%" PRIu64, uid);
		ret += "void GtiRecordDeserialize_" + (std::string)buf + " (";
		ret += "GtiRecord_" + (std::string)buf + " *out_record, void* buf, uint64_t len)\n";
		ret += "{\n";
		ret += "    uint64_t i = 0;\n";
		ret += "    \n";
		ret += "    i += sizeof (uint64_t);\n"; //no need to deserialize uid (one might make an assert from that info but nothing else...)
		ret += "    \n";

		//deserialize all the args
		std::list<GtiRecordArgument>::iterator argIter;
		for (argIter = myArgs.begin(); argIter != myArgs.end(); argIter++)
		{
			if (argIter->isArray)
			{
				ret += "    if (out_record->" + argIter->lengthArgument + "> 0)\n";
				ret += "    {\n";
				ret += "        out_record->" + argIter->name + " = (" + argIter->type + "*)" +
				       "malloc (sizeof(" + argIter->type + ")*((out_record->" + argIter->lengthArgument + "+7)/8)*8);\n";
				ret += "        memmove (out_record->" + argIter->name +
					   ", ((char*)buf)+i, sizeof (" + argIter->type + ")*out_record->" + argIter->lengthArgument + ");\n";
				ret += "        i += sizeof(" + argIter->type + ")*out_record->" + argIter->lengthArgument + ";\n";
				ret += "    }\n";
				ret += "    else\n";
				ret += "    {\n";
				ret += "        out_record->" + argIter->name + " = NULL;\n";
				ret += "    }\n";
			}
			else
			{
				ret += "    memmove (&(out_record->" + argIter->name +
					   "), ((char*)buf)+i, sizeof (" + argIter->type + "));\n";
				ret += "    i += sizeof(" + argIter->type + ");\n";
			}
		}

		//finish the deserialize function
		ret += "    assert (i <= len);\n";
		ret += "}\n\n";
	}

	return ret;
}

//---------------------------------------
// GtiRecordGenImpl
//---------------------------------------
void GtiRecordCommittedDesc::print (void)
{
	//find all uid's that use this record
	std::map<uint64_t,bool> uids;

	std::map <uint64_t, std::string>::iterator i;
	std::map <uint64_t, bool>::iterator j;

	for (i = myDefinitionNames.begin(); i != myDefinitionNames.end(); i++)
		uids.insert (std::make_pair(i->first, true));

	for (j = mySerializeUids.begin(); j != mySerializeUids.end(); j++)
		uids.insert (std::make_pair(j->first, true));
	for (j = myDeserializeUids.begin(); j != myDeserializeUids.end(); j++)
		uids.insert (std::make_pair(j->first, true));

	//print start with uids
	std::cout << "Record used by uids:";
	for (j = uids.begin();j != uids.end(); j++)
		std::cout << " " << j->first << ";";
	std::cout << std::endl;

	//print args
	std::cout << "Args:" << std::endl;
	std::list<GtiRecordArgument>::iterator argIter;
	for (argIter = myArgs.begin(); argIter != myArgs.end(); argIter++)
	{
		std::cout
			<< "{name=" << argIter->name
			<< ", type=" << argIter->type
			<< ", fromCall=" << argIter->fromCall
			<< ", asArg=" << argIter->asArg
			<< ", isArray=" << argIter->isArray
			<< ", lengthArgument=" << argIter->lengthArgument
			<< "}" << std::endl;
	}

	//needs definition
	std::cout << "Needs definition for uids:";
	for (i = myDefinitionNames.begin(); i != myDefinitionNames.end(); i++)
		std::cout << " " << i->first << ";";
	std::cout << std::endl;

	//needs serialize function
	std::cout << "Needs serialize function for uids:";
	for (j = mySerializeUids.begin(); j != mySerializeUids.end(); j++)
		std::cout << " " << j->first << ";";
	std::cout << std::endl;

	//needs de-serialize function
	std::cout << "Needs de-serialize function for uids:";
	for (j = myDeserializeUids.begin(); j != myDeserializeUids.end(); j++)
		std::cout << " " << j->first << ";";
	std::cout << std::endl;
}

//=======================================
//			GtiRecordGenImplementation
//=======================================
//---------------------------------------
// GtiRecordGenImpl
//---------------------------------------
GtiRecordGenImplementation::GtiRecordGenImplementation ()
	: myCommittedDescs()
{
	myInited = false;
	myInInit = false;

	myIsVerbose = false;
	if (getenv("GTI_VERBOSE") != NULL)
	{
		if (atoi(getenv("GTI_VERBOSE")) > 0)
			myIsVerbose = true;
	}
}

//---------------------------------------
// ~GtiRecordGenImpl
//---------------------------------------
GtiRecordGenImplementation::~GtiRecordGenImplementation ()
{
	if (myInited)
	{
		//print data to stdout if wanted
		if (myIsVerbose)
			printCommittedDescs ();

		//Start creation of backend code
		std::list<GtiRecordCommittedDesc>::iterator iter;
		std::string code;

		//create heading for header
		myHeader << "/* This file was created by the GTI record" << std::endl
			     << " * generation implementation." << std::endl
			     << " * " << std::endl
			     << " * !!! Do not edit this file !!!" << std::endl
			     << " */" << std::endl << std::endl
			     << "#if !defined(__APPLE__)" << std::endl
                 << "#    include <malloc.h>" << std::endl
                 << "#endif" << std::endl
				 << "#include <stdlib.h>" << std::endl
                 << "#include <stdint.h>" << std::endl
			     << "#include <stdio.h>" << std::endl
			     << "#include <assert.h>" << std::endl;

		//generate backend code for header
		for (iter = myCommittedDescs.begin (); iter != myCommittedDescs.end(); iter++)
		{
			code = iter->backendGenCreateHeaderDefinitions ();
			myHeader << code;
		}

		myHeader.close ();

		//create heading for source
		mySource
			<< "/* This file was created by the GTI record" << std::endl
			<< " * generation implementation." << std::endl
			<< " * " << std::endl
			<< " * !!! Do not edit this file !!!" << std::endl
			<< " */" << std::endl << std::endl
			<< "#include \"" << myGenPath << "/" << myPrefix << "GenRecord.h" << "\"" << std::endl
			<< "#include <string.h>" << std::endl
			<< std::endl;

		//generate backend code for header
		for (iter = myCommittedDescs.begin (); iter != myCommittedDescs.end(); iter++)
		{
			code = iter->backendGenCreateSource ();
			mySource << code;
		}

		mySource.close ();
	}
}

//---------------------------------------
// getCommittedDesc
//---------------------------------------
GtiRecordCommittedDesc* GtiRecordGenImplementation::getCommittedDesc (GtiRecordCommittedDesc descToCheck)
{
	if (!myInited)
		return NULL;

	std::list<GtiRecordCommittedDesc>::iterator iter;

	//A already known description ?
	for (iter = myCommittedDescs.begin(); iter != myCommittedDescs.end(); iter++)
	{
		if ((*iter) == descToCheck)
			return &(*iter);
	}

	//Its a new description
	myCommittedDescs.push_back (descToCheck);
	return &(myCommittedDescs.back());
}

//---------------------------------------
// initBegin
//---------------------------------------
GTI_RETURN GtiRecordGenImplementation::initBegin (
		std::string targetDirectory,
		std::string prefix)
{
	if (myInited || myInInit)
		return GTI_ERROR;

	myGenPath = targetDirectory;
	myPrefix = prefix;

	myInInit = true;

	return GTI_SUCCESS;
}

//---------------------------------------
// initGetIntermediateSourceFileNames
//---------------------------------------
GTI_RETURN GtiRecordGenImplementation::initGetIntermediateSourceFileNames (
		std::list<std::string> *pOutSourceNames
		)
{
	if (!myInInit)
		return GTI_ERROR;
	assert (pOutSourceNames);

	std::string genFile = myGenPath + "/" + myPrefix + "GenRecord.c";
	pOutSourceNames->push_front (genFile);

	mySource.open (genFile.c_str());
	assert (!mySource.fail ());

	return GTI_SUCCESS;
}

//---------------------------------------
// initGetStaticSourceFileNames
//---------------------------------------
GTI_RETURN GtiRecordGenImplementation::initGetStaticSourceFileNames (std::list<std::string> *pOutSourceNames)
{
	if (!myInInit)
		return GTI_ERROR;
	//uses no static source files ...
	return GTI_SUCCESS;
}

//---------------------------------------
// Init_GetLibNames
//---------------------------------------
GTI_RETURN GtiRecordGenImplementation::initGetLibNames (std::list<std::string> *pOutLibNames)
{
	if (!myInInit)
		return GTI_ERROR;
	//uses no libs
	return GTI_SUCCESS;
}

//---------------------------------------
// initGetHeaderFileNames
//---------------------------------------
GTI_RETURN GtiRecordGenImplementation::initGetHeaderFileNames (std::list<std::string> *pOutHeaderNames)
{
	if (!myInInit)
		return GTI_ERROR;
	assert (pOutHeaderNames);

	//pOutHeaderNames->push_front ("malloc.h");
	std::string genFile = myGenPath + "/" + myPrefix + "GenRecord.h";
	pOutHeaderNames->push_front (genFile);

	myHeader.open (genFile.c_str());
	assert (!myHeader.fail ());

	return GTI_SUCCESS;
}

//---------------------------------------
// initEnd
//---------------------------------------
GTI_RETURN GtiRecordGenImplementation::initEnd ()
{
	if (!myInInit)
		return GTI_ERROR;

	assert (mySource.is_open());
	assert (myHeader.is_open());

	myInInit = false;
	myInited = true;

	return GTI_SUCCESS;
}

//---------------------------------------
// returnUidFromSerialized
//---------------------------------------
GTI_RETURN GtiRecordGenImplementation::returnUidFromSerialized (std::string bufName, std::string *pOutCode)
{
	assert (pOutCode);

	*pOutCode = "((uint64_t*)" + bufName + ")[0]";

	return GTI_SUCCESS;
}

//---------------------------------------
// createRecordDescription
//---------------------------------------
GTI_RETURN GtiRecordGenImplementation::createRecordDescription (I_RecordDescription **pOutDesc)
{
	if (!myInited)
		return GTI_ERROR;
	assert (pOutDesc);
	*pOutDesc = new GtiRecordGenDesc (this);
	assert (*pOutDesc);
	return GTI_SUCCESS;
}

//---------------------------------------
// printCommittedDescs
//---------------------------------------
void GtiRecordGenImplementation::printCommittedDescs (void)
{
	std::list<GtiRecordCommittedDesc>::iterator i;
	std::cout
		<< "Printing all finished descriptions and their state:" << std::endl
		<< "========================" <<std::endl;

	for (i = myCommittedDescs.begin (); i != myCommittedDescs.end(); i++)
	{
		i->print ();
		std::cout << std::endl;
	}

	std::cout
		<< "========================" << std::endl;
}

//=======================================
//			GtiRecordGenDesc
//=======================================
//---------------------------------------
// GtiRecordGenDesc
//---------------------------------------
GtiRecordGenDesc::GtiRecordGenDesc(GtiRecordGenImplementation *pMaster)
	: mypMaster(pMaster),
	  myDesc ()
{
	assert (mypMaster);
}

//---------------------------------------
// addArgument
//---------------------------------------
GTI_RETURN GtiRecordGenDesc::addArgument (
		std::string name,
		std::string type,
		std::string fromCall,
		std::string asArg)
{
	GtiRecordArgument arg (name, type, fromCall, asArg);
	GTI_RETURN ret = myDesc.addArgument(arg);

	return ret;
}

//---------------------------------------
// addArrayArgument
//---------------------------------------
GTI_RETURN GtiRecordGenDesc::addArrayArgument (
		std::string name,
		std::string type,
		std::string fromCall,
		std::string asArg,
		std::string lengthArgument)
{
	GtiRecordArgument arg (name, type, fromCall, asArg, lengthArgument);
	GTI_RETURN ret = myDesc.addArgument(arg);

	return ret;
}

//---------------------------------------
// createRecord
//---------------------------------------
GTI_RETURN GtiRecordGenDesc::createRecord (uint64_t uid, I_RecordType **pOutRecord)
{
	GtiRecordCommittedDesc* pDesc = mypMaster->getCommittedDesc (myDesc);
	assert (pDesc);
	assert (pOutRecord);

	*pOutRecord = new GtiRecordGenRecord (pDesc, uid);
	assert (*pOutRecord);

	return GTI_SUCCESS;
}

//---------------------------------------
// deleteObject
//---------------------------------------
GTI_RETURN GtiRecordGenDesc::deleteObject (void)
{
	delete (this);
	return GTI_SUCCESS;
}


//=======================================
//			GtiRecordGenRecord
//=======================================
//---------------------------------------
// GtiRecordGenRecord
//---------------------------------------
GtiRecordGenRecord::GtiRecordGenRecord (GtiRecordCommittedDesc *pDesc, uint64_t uid)
	:	mypDesc (pDesc),
		myUid (uid)
{
	assert (pDesc);
}

//---------------------------------------
// createInstance
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::createInstance (std::string varName, std::string *pOutCode)
{
	std::string ret = mypDesc->frontendGenGetStructName(myUid);
	ret += " " + varName;

	assert (pOutCode);
	*pOutCode = ret;

	return GTI_SUCCESS;
}

//---------------------------------------
// initInstance
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::createInstancePointer (std::string varName, std::string *pOutCode)
{
	std::string ret = mypDesc->frontendGenGetStructName(myUid);
	ret += " *" + varName;

	assert (pOutCode);
	*pOutCode = ret;

	return GTI_SUCCESS;
}

//---------------------------------------
// getPointerType
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::getPointerType (std::string *pOutCode)
{
	assert (pOutCode);

	*pOutCode = mypDesc->frontendGenGetStructName(myUid) + "*";

	return GTI_SUCCESS;
}

//---------------------------------------
// initInstance
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::allocInstance (std::string varName, std::string *pOutCode)
{
	std::string temp = mypDesc->frontendGenGetStructName(myUid);

	std::string ret = varName + "= (" + temp + "*) malloc (sizeof(" + temp + "));\n";

	assert (pOutCode);
	*pOutCode = ret;

	return GTI_SUCCESS;
}

//---------------------------------------
// initInstance
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::deallocInstance (std::string varName, std::string *pOutCode)
{
	std::string ret = "free (" + varName + ");\n";
	ret += varName + "= NULL;\n";

	assert (pOutCode);
	*pOutCode = ret;

	return GTI_SUCCESS;
}

//---------------------------------------
// initInstance
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::initInstance (std::string varName, std::string *pOutCode)
{
	assert (pOutCode);

	//Used to set array pointers to NULL and arrayLengths to 0
	*pOutCode = mypDesc->frontendGenInitArgs (varName);

	return GTI_SUCCESS;
}

//---------------------------------------
// freeInstance
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::freeInstance (std::string varName, std::string *pOutCode)
{
	assert (pOutCode);

	//Used to free pointers
	*pOutCode = mypDesc->frontendGenFreeArgs (varName);

	return GTI_SUCCESS;
}

//---------------------------------------
// writeArgument
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::writeArgument (
		std::string varName,
		std::string argument,
		std::string value,
		std::string *pOutCode)
{
	assert (pOutCode);

	//pass to Description
	*pOutCode = mypDesc->frontendGenWriteArgument (varName, argument, value);

	return GTI_SUCCESS;
}

//---------------------------------------
// writeArrayArgument
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::writeArrayArgument (
		std::string varName,
		std::string argument,
		std::string valueArray,
		std::string *pOutCode)
{
	assert (pOutCode);

	//pass to Description
	*pOutCode = mypDesc->frontendGenWriteArrayArgument (varName, argument, valueArray);

	return GTI_SUCCESS;
}

//---------------------------------------
// writeArrayArgumentIndexed
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::writeArrayArgumentIndexed (
		std::string varName,
		std::string argument,
		std::string value,
		std::string index,
		std::string *pOutCode)
{
	assert (pOutCode);

	//pass to Description
	*pOutCode = mypDesc->frontendGenWriteArrayArgumentIndexed (varName, argument, value, index);

	return GTI_SUCCESS;
}

//---------------------------------------
// returnArgument
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::returnArgument (
		std::string varName,
		std::string argument,
		std::string *pOutCode)
{
	assert (pOutCode);

	//pass to Description
	*pOutCode = mypDesc->frontendGenReturnArgument (varName, argument);

	return GTI_SUCCESS;
}

//---------------------------------------
// returnArrayArgument
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::returnArrayArgument (
		std::string varName,
		std::string argument,
		std::string *pOutCode)
{
	assert (pOutCode);

	//pass to Description
	*pOutCode = mypDesc->frontendGenReturnArrayArgument (varName, argument);

	return GTI_SUCCESS;
}

//---------------------------------------
// returnArrayArgumentIndexed
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::returnArrayArgumentIndexed (
		std::string varName,
		std::string argument,
		std::string index,
		std::string *pOutCode)
{
	assert (pOutCode);

	//pass to Description
	*pOutCode = mypDesc->frontendGenReturnArrayArgumentIndexed (varName, argument, index);

	return GTI_SUCCESS;
}

//---------------------------------------
// serialize
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::serialize (
		std::string varName,
		std::string serBufOutName,
		std::string numBytesOutName,
		std::string *pOutCallCode)
{
	assert (pOutCallCode);

	//pass to Description
	*pOutCallCode = mypDesc->frontendGenSerializeCall (myUid, varName, serBufOutName, numBytesOutName);

	return GTI_SUCCESS;
}

//---------------------------------------
// deserialize
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::deserialize (
		std::string serBufName,
		std::string serBufLength,
		std::string varNameOutRecord,
		std::string *pOutCallCode)
{
	assert (pOutCallCode);

	*pOutCallCode = mypDesc->frontendGenDeserializeCall (myUid, varNameOutRecord, serBufName, serBufLength);

	return GTI_SUCCESS;
}

//---------------------------------------
// deleteObject
//---------------------------------------
GTI_RETURN GtiRecordGenRecord::deleteObject (void)
{
	delete (this);
	return GTI_SUCCESS;
}

/*EOF*/
