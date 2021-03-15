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
 * @file GtiRecordGenImplementation.h
 *       Record Generation Interface implementation with a mainly generated record.
 *
 * @ref RecordGenerator - Learn more about the Record Generator
 *
 * @author Tobias Hilbrich
 * @date 11.12.2009
 */

/**
 * @page GtiRecordImplementation Trace Record Generation Interface Implementation
 * @section GtiRecordImplementationOverview Overview
 * Implements: @ref RecordGenerator -- see for details
 *
 * This is a
 * simple record that is completely generated and builds records
 * that exactly fit the given descriptions. However, will not be usable
 * for existing applications that have their own trace records. It
 * will be used as default trace record for GTI.
 *
 * The code that is returned to the user of the generator is referred
 * to as frontend code. The remaining code that is used to create
 * declarations for the records and to provide serialize and
 * de-serialize functions is referred to as backend code.
 *
 * @section GtiRecordImplementationDeveloper Developer Details
 * The implementation consists of:
 * - gti::GtiRecordGenImplementation - High level interface, manages committed descriptions
 * - gti::GtiRecordGenDesc - A new description used to form the new description
 * - gti::GtiRecordGenRecord - A record used to generate code for user
 * - gti::GtiRecordCommittedDesc - A committed description, used to generated back-end code (generated source files)
 * - gti::GtiRecordArgument - A single argument of a record
 *
 * The gti::I_RecordGenerator interface is implemented by gti::GtiRecordGenImplementation, the
 * gti::I_RecordDescription interface by gti::GtiRecordGenDesc, and the gti::I_RecordType
 * interface by gti::GtiRecordGenRecord. The class gti::GtiRecordCommittedDesc
 * represents a complete description of a record, which is managed by
 * gti::GtiRecordGenImplementation.
 * @image html GtiRecordGeneratorImplementation.png "Class diagram for record generation implementation"
 * @image latex GtiRecordGeneratorImplementation.png "Class diagram for record generation implementation" width=13cm
 *
 * The class diagram above highlights the relationship between these classes.
 */

#include <fstream>

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_RecordGenerator.h"

#ifndef GTI_RECORD_GEN_IMPL_H
#define GTI_RECORD_GEN_IMPL_H

namespace gti
{
	/**
     * A single argument (either array or single value).
     * Mainly used like a struct.
     */
    class GtiRecordArgument
    {
    public:
    	std::string name;
    	std::string type;
    	std::string fromCall;
    	std::string asArg;
    	bool 		isArray;
    	std::string lengthArgument;

    	GtiRecordArgument (
    			std::string name,
    			std::string type,
    			std::string fromCall,
    			std::string asArg
    			);
    	GtiRecordArgument (
    			std::string nameArray,
    			std::string type,
    			std::string fromCall,
    	    	std::string asArg,
    	    	std::string lengthArgument
    	    	);
    	bool operator == (GtiRecordArgument const& other) const;
    	bool operator != (GtiRecordArgument const& other) const;

    protected:
    	bool compare (
    			std::string name,
    			std::string type,
    			std::string fromCall,
    			std::string asArg,
    			bool isArray,
    			std::string lengthArgument
    			) const;
    };

    /**
     * A record description.
     * Will be used most frequently after it is
     * committed.
     */
    class GtiRecordCommittedDesc
    {
    public:
    	GtiRecordCommittedDesc ();
    	~GtiRecordCommittedDesc ();
    	bool operator == (GtiRecordCommittedDesc const& other) const;
    	GTI_RETURN addArgument (GtiRecordArgument arg);

    	std::string frontendGenGetStructName (uint64_t uid);
    	std::string frontendGenInitArgs (std::string varName);
    	std::string frontendGenFreeArgs (std::string varName);
    	std::string frontendGenWriteArgument (
    			std::string varName,
    			std::string argument,
    			std::string value);
    	std::string frontendGenWriteArrayArgument (
    			std::string varName,
    			std::string argument,
    			std::string valueArray);
    	std::string frontendGenWriteArrayArgumentIndexed (
    			std::string varName,
    			std::string argument,
    			std::string value,
    			std::string index);
    	std::string frontendGenReturnArgument (std::string varName, std::string argument);
    	std::string frontendGenReturnArrayArgument (std::string varName, std::string argument);
    	std::string frontendGenReturnArrayArgumentIndexed (
    			std::string varName,
    			std::string argument,
    			std::string index);
    	std::string frontendGenSerializeCall (
    			uint64_t uid,
    			std::string varName,
    			std::string serBufOutName,
    			std::string numBytesOutName);
    	std::string frontendGenDeserializeCall (
    			uint64_t uid,
    			std::string varNameOutRecord,
    			std::string serBufName,
    			std::string serBufLength);

    	std::string backendGenCreateHeaderDefinitions (void);
    	std::string backendGenCreateSource (void);

    	void print (void);

    protected:
    	std::list<GtiRecordArgument> myArgs;
    	bool myNeedsHeaderDefinition;
    	bool myNeedsSerialize;
    	bool myNeedsDeserialize;
    	std::map <uint64_t, std::string> myDefinitionNames;
    	std::map <uint64_t, bool> mySerializeUids;
    	std::map <uint64_t, bool> myDeserializeUids;

    	bool compare (std::list<GtiRecordArgument> args) const;
    };

    /**
     * Implementation of top level interface for record usage generation.
     *
     * @ref RecordGenerator - Learn more about the Record Generator
     *
     * @see gti::I_RecordDescription
     */
    class GtiRecordGenImplementation : public I_RecordGenerator
    {
    public:

    	/**
    	 * Constructor
    	 */
    	GtiRecordGenImplementation ();

    	/**
    	 * Destructor
    	 */
    	~GtiRecordGenImplementation ();

    	/**
    	 * Gets a description that should be committed and
    	 * adds it to the list of known descriptions (if
    	 * it is a new one) or returns a pointer to a
    	 * similar already existing description.
    	 * @param descToCheck the new description.
    	 * @return pointer to the managed description.
    	 */
    	GtiRecordCommittedDesc* getCommittedDesc (GtiRecordCommittedDesc descToCheck);

    	/**
         * @see gti::I_RecordGenerator::initBegin
         */
        GTI_RETURN initBegin (
        		std::string targetDirectory,
                std::string prefix);

        /**
         * @see gti::I_RecordGenerator::initGetIntermediateSourceFileNames
         */
        GTI_RETURN initGetIntermediateSourceFileNames (
                std::list<std::string> *pOutSourceNames
                );

        /**
         * @see gti::I_RecordGenerator::initGetStaticSourceFileNames
         */
        GTI_RETURN initGetStaticSourceFileNames (std::list<std::string> *pOutSourceNames);

        /**
         * @see gti::I_RecordGenerator::initGetLibNames
         */
        GTI_RETURN initGetLibNames (std::list<std::string> *pOutLibNames);

        /**
         * @see gti::I_RecordGenerator::initGetHeaderFileNames
         */
        GTI_RETURN initGetHeaderFileNames (std::list<std::string> *pOutHeaderNames);

        /**
         * @see gti::I_RecordGenerator::initEnd
         */
        GTI_RETURN initEnd ();

        /**
         * @see gti::I_RecordGenerator::returnUidFromSerialized
         */
        GTI_RETURN returnUidFromSerialized (std::string bufName, std::string *pOutCode);

        /**
         * @see gti::I_RecordGenerator::createRecordDescription
         */
        GTI_RETURN createRecordDescription (I_RecordDescription **pOutDesc);

    protected:
    	bool myInited;
    	bool myInInit;
    	bool myIsVerbose;
    	std::string myPrefix;
    	std::string myGenPath;
    	std::ofstream mySource;
    	std::ofstream myHeader;

    	std::list<GtiRecordCommittedDesc> myCommittedDescs;

    	void printCommittedDescs (void);
    };

    /**
     * Implementation of the record description interface.
     */
    class GtiRecordGenDesc : public I_RecordDescription
    {
    public:
    	/**
    	 * Constructor.
    	 */
    	GtiRecordGenDesc (GtiRecordGenImplementation *pMaster);

        /**
         * @see gti::I_RecordDescription::addArgument
         */
        GTI_RETURN addArgument (
                std::string name,
                std::string type,
                std::string fromCall,
                std::string asArg);

        /**
         * @see gti::I_RecordDescription::addArrayArgument
         */
        GTI_RETURN addArrayArgument (
        		std::string name,
        		std::string type,
        		std::string fromCall,
        		std::string asArg,
        		std::string lengthArgument);

        /**
         * @see gti::I_RecordDescription::createRecord
         */
        GTI_RETURN createRecord (uint64_t uid, I_RecordType **pOutRecord);

        /**
         * @see gti::I_RecordDescription::deleteObject
         */
        GTI_RETURN deleteObject (void);

    protected:
    	GtiRecordCommittedDesc myDesc;
    	GtiRecordGenImplementation *mypMaster;
    };

    /**
     * Record type interface implementation.
     */
    class GtiRecordGenRecord : public I_RecordType
    {
    public:
    	/**
    	 * Constructor.
    	 */
    	GtiRecordGenRecord (GtiRecordCommittedDesc *pDesc, uint64_t uid);

        /**
         * @see gti::I_RecordType::createInstance
         */
        GTI_RETURN createInstance (std::string varName, std::string *pOutCode);

        /**
         * @see gti::I_RecordType::createInstancePointer
         */
        GTI_RETURN createInstancePointer (std::string varName, std::string *pOutCode);

        /**
         * @see gti::I_RecordType::getPointerType
         */
        GTI_RETURN getPointerType (std::string *pOutCode);

        /**
         * @see gti::I_RecordType::allocInstance
         */
        GTI_RETURN allocInstance (std::string varName, std::string *pOutCode);

        /**
         * @see gti::I_RecordType::deallocInstance
         */
        GTI_RETURN deallocInstance (std::string varName, std::string *pOutCode);

        /**
         * @see gti::I_RecordType::initInstance
         */
        GTI_RETURN initInstance (std::string varName, std::string *pOutCode);

        /**
         * @see gti::I_RecordType::freeInstance
         */
        GTI_RETURN freeInstance (std::string varName, std::string *pOutCode);

        /**
         * @see gti::I_RecordType::writeArgument
         */
        GTI_RETURN writeArgument (
                std::string varName,
                std::string argument,
                std::string value,
                std::string *pOutCode);

        /**
         * @see gti::I_RecordType::writeArrayArgument
         */
        GTI_RETURN writeArrayArgument (
        		std::string varName,
        		std::string argument,
        		std::string valueArray,
        		std::string *pOutCode);

        /**
         * @see gti::I_RecordType::writeArrayArgumentIndexed
         */
        GTI_RETURN writeArrayArgumentIndexed (
        		std::string varName,
        		std::string argument,
        		std::string value,
        		std::string index,
        		std::string *pOutCode);

        /**
         * @see gti::I_RecordType::returnArgument
         */
        GTI_RETURN returnArgument (
                std::string varName,
                std::string argument,
                std::string *pOutCode);

        /**
         * @see gti::I_RecordType::returnArrayArgument
         */
        GTI_RETURN returnArrayArgument (
        		std::string varName,
        		std::string argument,
        		std::string *pOutCode);

        /**
         * @see gti::I_RecordType::returnArrayArgumentIndexed
         */
        GTI_RETURN returnArrayArgumentIndexed (
        		std::string varName,
        		std::string argument,
        		std::string index,
        		std::string *pOutCode);

        /**
         * @see gti::I_RecordType::serialize
         */
        GTI_RETURN serialize (
                std::string varName,
                std::string serBufOutName,
                std::string numBytesOutName,
                std::string *pOutCallCode);

        /**
         * @see gti::I_RecordType::deserialize
         */
        GTI_RETURN deserialize (
                std::string serBufName,
                std::string serBufLength,
                std::string varNameOutRecord,
                std::string *pOutCallCode);

        /**
         * @see gti::I_RecordType::deleteObject
         */
        GTI_RETURN deleteObject (void);

    protected:
    	GtiRecordCommittedDesc *mypDesc;
    	uint64_t myUid;
    };
}

#endif /*GTI_RECORD_GEN_IMPL_H*/
