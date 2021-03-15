/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file BaseConstants.cpp
 *       @see MUST::BaseConstants.
 *
 *  @date 12.04.2011
 *  @author Mathias Korepkat
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "BaseConstants.h"

using namespace must;

mGET_INSTANCE_FUNCTION(BaseConstants)
mFREE_INSTANCE_FUNCTION(BaseConstants)
mPNMPI_REGISTRATIONPOINT_FUNCTION(BaseConstants)

//=============================
// Constructor
//=============================
BaseConstants::BaseConstants (const char* instanceName)
    : gti::ModuleBase<BaseConstants, I_BaseConstants> (instanceName),
      myMpiProcNull (-1),
      myMpiAnySource (-1),
      myMpiAnyTag (-1),
      myMpiUndefined (-1),
      myMpiBsendOverhead (-1),
      myMpiTagUb (-1)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    if (subModInstances.size() > 0)
    {
        for (std::vector<I_Module*>::size_type i = 0; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }
}

//=============================
// Destructor
//=============================
BaseConstants::~BaseConstants ()
{
	//Nothing to do
}

//=============================
// addConstants
//=============================
GTI_ANALYSIS_RETURN BaseConstants::addConstants (
		int mpiProcNull,
		int mpiAnySource,
		int mpiAnyTag,
		int mpiUndefined,
		int mpiBsendOverhead,
		int mpiTagUb,
		int mpiVersion,
		int mpiSubversion,
        int mpiDistributeBlock,
        int mpiDistributeCyclic,
        int mpiDistributeNone,
        int mpiDistributeDfltDarg,
        int mpiOrderC,
        int mpiOrderFortran,
		void* mpiBottom
		)
{
	myMpiProcNull = mpiProcNull;
	myMpiAnySource = mpiAnySource;
	myMpiAnyTag = mpiAnyTag;
	myMpiUndefined = mpiUndefined;
	myMpiBsendOverhead = mpiBsendOverhead;
	myMpiTagUb = mpiTagUb;
	myMpiVersion = mpiVersion;
	myMpiSubversion = mpiSubversion;
    myMpiDistributeBlock = mpiDistributeBlock,
    myMpiDistributeCyclic = mpiDistributeCyclic,
    myMpiDistributeNone = mpiDistributeNone,
    myMpiDistributeDfltDarg = mpiDistributeDfltDarg,
    myMpiOrderC = mpiOrderC,
    myMpiOrderFortran = mpiOrderFortran,
	myMpiBottom = mpiBottom;

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// isProcNull
//=============================
bool BaseConstants::isProcNull (int mpiProcNull)
{
	 if(myMpiProcNull == mpiProcNull)
		 return true;

	return false;
}

//=============================
// getProcNull
//=============================
int BaseConstants::getProcNull ()
{
	 return myMpiProcNull;
}

//=============================
// isAnySource
//=============================
bool BaseConstants::isAnySource (int mpiAnySource)
{
	 if(myMpiAnySource == mpiAnySource)
		 return true;

	return false;
}

//=============================
// getAnySource
//=============================
int BaseConstants::getAnySource ()
{
	 return myMpiAnySource;
}

//=============================
// isAnyTag
//=============================
bool BaseConstants::isAnyTag (int mpiAnyTag)
{
	 if(myMpiAnyTag == mpiAnyTag)
		 return true;

	return false;
}

//=============================
// getAnyTag
//=============================
int BaseConstants::getAnyTag ()
{
	 return myMpiAnyTag;
}

//=============================
// isUndefined
//=============================
bool BaseConstants::isUndefined (int mpiUndefined)
{
	 if(myMpiUndefined == mpiUndefined)
		 return true;

	return false;
}

//=============================
// getUndefined
//=============================
int BaseConstants::getUndefined ()
{
	 return myMpiUndefined;
}

//=============================
// isBsendOverhead
//=============================
bool BaseConstants::isBsendOverhead (int mpiBsendOverhead)
{
	 if(myMpiBsendOverhead == mpiBsendOverhead)
		 return true;

	return false;
}

//=============================
// getBsendOverhead
//=============================
int BaseConstants::getBsendOverhead ()
{
	 return myMpiBsendOverhead;
}

//=============================
// isTagUb
//=============================
bool BaseConstants::isTagUb (int mpiTagUb)
{
	 if(myMpiTagUb == mpiTagUb)
		 return true;

	return false;
}

//=============================
// getTagUb
//=============================
int BaseConstants::getTagUb ()
{
	 return myMpiTagUb;
}

//=============================
// isVersion
//=============================
bool BaseConstants::isVersion (int mpiVersion)
{
	 if(myMpiVersion == mpiVersion)
		 return true;

	return false;
}

//=============================
// getVersion
//=============================
int BaseConstants::getVersion()
{
	 return myMpiVersion;
}

//=============================
// isSubversion
//=============================
bool BaseConstants::isSubversion (int mpiSubversion)
{
	 if(myMpiSubversion == mpiSubversion)
		 return true;

	return false;
}

//=============================
// getSubversion
//=============================
int BaseConstants::getSubversion()
{
	 return myMpiSubversion;
}

//=============================
// isDistributeBlock
//=============================
bool BaseConstants::isDistributeBlock (int val)
{
     return myMpiDistributeBlock == val;
}

//=============================
// getDistributeBlock
//=============================
int BaseConstants::getDistributeBlock (void)
{
     return myMpiDistributeBlock;
}

//=============================
// isDistributeCyclic
//=============================
bool BaseConstants::isDistributeCyclic (int val)
{
     return myMpiDistributeCyclic == val;
}

//=============================
// getDistributeCyclic
//=============================
int BaseConstants::getDistributeCyclic (void)
{
     return myMpiDistributeCyclic;
}

//=============================
// isDistributeNone
//=============================
bool BaseConstants::isDistributeNone (int val)
{
     return myMpiDistributeNone == val;
}

//=============================
// getDistributeNone
//=============================
int BaseConstants::getDistributeNone (void)
{
     return myMpiDistributeNone;
}

//=============================
// isDistributeDfltDarg
//=============================
bool BaseConstants::isDistributeDfltDarg (int val)
{
     return myMpiDistributeDfltDarg == val;
}

//=============================
// getDistributeDfltDarg
//=============================
int BaseConstants::getDistributeDfltDarg (void)
{
     return myMpiDistributeDfltDarg;
}

//=============================
// isOrderC
//=============================
bool BaseConstants::isOrderC (int val)
{
     return myMpiOrderC == val;
}

//=============================
// getOrderC
//=============================
int BaseConstants::getOrderC (void)
{
     return myMpiOrderC;
}

//=============================
// isOrderFortran
//=============================
bool BaseConstants::isOrderFortran (int val)
{
     return myMpiOrderFortran == val;
}

//=============================
// getOrderFortran
//=============================
int BaseConstants::getOrderFortran (void)
{
     return myMpiOrderFortran;
}

//=============================
// isBottom
//=============================
bool BaseConstants::isBottom (void* mpiBottom)
{
	 if(myMpiBottom == mpiBottom)
		 return true;

	return false;
}

//=============================
// getBottom
//=============================
void* BaseConstants::getBottom ()
{
	 return myMpiBottom;
}
/*EOF*/
