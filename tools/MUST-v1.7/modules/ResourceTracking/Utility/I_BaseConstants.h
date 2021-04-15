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
 * @file I_BaseConstants.h
 *       @see I_BaseConstants.
 *
 *  @date 12.04.2011
 *  @author Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_BASECONSTANTS_H
#define I_BASECONSTANTS_H

/**
 * Interface for providing basic MPI constants.
 * It is used to save values of MPI constants.
 * It should be invoked after (post) MPI_Init.
 *
 * Usage scenarios include comparing values -- such as
 * tags and ranks -- against MPI constants, in order to
 * determine their correctness. Usually these constants
 * are negative values, and must thus be considered when
 * determining whether some value is really invalid.
 */
class I_BaseConstants : public gti::I_Module
{
public:
	/**
	 * Gets the given constants and save them.
	 * @param mpiProcNull value of MPI_PROC_NULL.
	 * @param mpiAnySource value of MPI_ANY_SOURCE.
	 * @param mpiAnyTag value of MPI_ANY_TAG.
	 * @param mpiUndefined value of MPI_UNDEFINED.
	 * @param mpiBsendOverhead value of MPI_BSEND_OVERHEAD.
	 * @param mpiTagUb upper bound for tags, querried as property.
	 * @param mpiVersion value of major version from MPI_Get_version.
     * @param mpiSubversion value of minor version from MPI_Get_version.
     * @param mpiDistributeBlock value of MPI_DISTRIBUTE_BLOCK.
     * @param mpiDistributeCyclic value of MPI_DISTRIBUTE_CYCLIC.
     * @param mpiDistributeNone value of MPI_DISTRIBUTE_NONE.
     * @param mpiDistributeDefautDarg value of MPI_DISTRIBUTE_NONE.
     * @param mpiOrderC value of MPI_ORDER_C.
     * @param mpiOrderFortran value of MPI_ORDER_FORTRAN.
	 * @param mpiBottom address of MPI_BOTTOM
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
     virtual gti::GTI_ANALYSIS_RETURN addConstants (
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
             int mpiDistributeDefautDarg,
             int mpiOrderC,
             int mpiOrderFortran,
    		 void* mpiBottom) = 0;

	/**
	 * Checks if an argument is equal to MPI_PROC_NULL
	 * @param val value that is checked
	 * @return true if val is equal to MPI_PROC_NULL
	 */
     virtual bool isProcNull (int val) = 0;

     /**
      * Returns the constant for MPI_PROC_NULL.
      * @return value of MPI_PROC_NULL.
      */
     virtual int getProcNull (void) = 0;

     /**
      * Checks if an argument is equal to MPI_ANY_SOURCE.
      * @param val value that is checked
      * @return true if val is equal to MPI_ANY_SOURCE
      */
     virtual bool isAnySource (int val) = 0;

     /**
      * Returns the constant for MPI_ANY_SOURCE.
      * @return value for MPI_ANY_SOURCE.
      */
     virtual int getAnySource (void) = 0;

     /**
      * Checks if an argument is equal to MPI_ANY_TAG.
      * @param val value that is checked
      * @return true if val is equal to MPI_ANY_TAG
      */
     virtual bool isAnyTag (int val) = 0;

     /**
      * Returns the constant for MPI_ANY_TAG.
      * @return value of MPI_ANY_TAG.
      */
     virtual int getAnyTag (void) = 0;

     /**
      * Checks if an argument is equal to MPI_UNDEFINED.
      * @param val value that is checked
      * @return true if val is equal to MPI_UNDEFINED
      */
     virtual bool isUndefined (int val) = 0;

     /**
      * Returns the value of MPI_UNDEFINED.
      * @return value of MPI_UNDEFINED.
      */
     virtual int getUndefined (void) = 0;

     /**
      * Checks if an argument is equal to MPI_BSEND_OVERHEAD.
      * @param val value that is checked
      * @return true if val is equal to MPI_BSEND_OVERHEAD
      */
     virtual bool isBsendOverhead (int val) = 0;

     /**
      * Returns the value of MPI_BSEND_OVERHEAD.
      * @return value of MPI_BSEND_OVERHEAD.
      */
     virtual int getBsendOverhead (void) = 0;

     /**
      * Checks if an argument is equal to MPI_TAG_UB.
      * @param val value that is checked
      * @return true if val is equal to MPI_TAG_UB
      */
     virtual bool isTagUb (int val) = 0;

     /**
      * Returns the value of MPI_TAG_UB.
      * @return value of MPI_TAG_UB.
      */
     virtual int getTagUb (void) = 0;

     /**
      * Checks if an argument is equal to the MPI Version.
      * @param val value that is checked
      * @return true if val is equal to the major version of used MPI
      */
     virtual bool isVersion (int val) = 0;

     /**
      * Returns the value of the major version of MPI.
      * @return value of the major version of used MPI.
      */
     virtual int getVersion (void) = 0;

     /**
      * Checks if an argument is equal to the MPI Subversion.
      * @param val value that is checked
      * @return true if val is equal to the minor version of used MPI
      */
     virtual bool isSubversion (int val) = 0;

     /**
      * Returns the value of the minor version of MPI.
      * @return value of the minor version of used MPI.
      */
     virtual int getSubversion (void) = 0;

     /**
      * Checks if an argument is equal to MPI_DISTRIBUTE_BLOCK.
      * @param val value that is checked
      * @return true if val is equal MPI_DISTRIBUTE_BLOCK
      */
     virtual bool isDistributeBlock (int val) = 0;

     /**
      * Returns the value of MPI_DISTRIBUTE_BLOCK.
      * @return value of the MPI_DISTRIBUTE_BLOCK.
      */
     virtual int getDistributeBlock (void) = 0;

     /**
      * Checks if an argument is equal to MPI_DISTRIBUTE_CYCLIC.
      * @param val value that is checked
      * @return true if val is equal MPI_DISTRIBUTE_CYCLIC
      */
     virtual bool isDistributeCyclic (int val) = 0;

     /**
      * Returns the value of MPI_DISTRIBUTE_CYCLIC.
      * @return value of the MPI_DISTRIBUTE_CYCLIC.
      */
     virtual int getDistributeCyclic (void) = 0;

     /**
      * Checks if an argument is equal to MPI_DISTRIBUTE_NONE.
      * @param val value that is checked
      * @return true if val is equal MPI_DISTRIBUTE_NONE
      */
     virtual bool isDistributeNone (int val) = 0;

     /**
      * Returns the value of MPI_DISTRIBUTE_NONE.
      * @return value of the MPI_DISTRIBUTE_NONE.
      */
     virtual int getDistributeNone (void) = 0;

     /**
      * Checks if an argument is equal to MPI_DISTRIBUTE_DFLT_DARG.
      * @param val value that is checked
      * @return true if val is equal MPI_DISTRIBUTE_DFLT_DARG
      */
     virtual bool isDistributeDfltDarg (int val) = 0;

     /**
      * Returns the value of MPI_DISTRIBUTE_DFLT_DARG.
      * @return value of the MPI_DISTRIBUTE_DFLT_DARG.
      */
     virtual int getDistributeDfltDarg (void) = 0;

     /**
      * Checks if an argument is equal to MPI_ORDER_C.
      * @param val value that is checked
      * @return true if val is equal MPI_ORDER_C
      */
     virtual bool isOrderC (int val) = 0;

     /**
      * Returns the value of MPI_ORDER_C.
      * @return value of the MPI_ORDER_C.
      */
     virtual int getOrderC (void) = 0;

     /**
      * Checks if an argument is equal to MPI_ORDER_FORTRAN.
      * @param val value that is checked
      * @return true if val is equal MPI_ORDER_FORTRAN
      */
     virtual bool isOrderFortran (int val) = 0;

     /**
      * Returns the value of MPI_ORDER_FORTRAN.
      * @return value of the MPI_ORDER_FORTRAN.
      */
     virtual int getOrderFortran (void) = 0;

     /**
      * Checks if an argument is equal to the MPI_BOTTOM.
      * @param val value that is checked
      * @return true if val is equal to MPI_BOTTOM otherwise false
      */
     virtual bool isBottom (void* val) = 0;

     /**
      * Returns the value of MPI_BOTTOM.
      * @return value of MPI_BOTTOM.
      */
     virtual void* getBottom (void) = 0;

}; /*class I_BaseConstants*/

#endif /*I_BASECONSTANTS_H*/
