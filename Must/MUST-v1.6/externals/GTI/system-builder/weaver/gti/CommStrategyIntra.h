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
 * @file CommStrategyIntra.h
 *      @see gti::weaver::CommStrategyIntra
 *
 * @author Tobias Hilbrich
 * @date 18.01.2012
 */

#ifndef COMMSTRATEGYINTRA_H
#define COMMSTRATEGYINTRA_H

#include <string>
#include <vector>

#include "Module.h"
#include "Configurable.h"
#include "Printable.h"

namespace gti
{
    namespace weaver
    {
        namespace modules
        {
            /**
             * Holds information for a intra communication
             * strategy of the GTI.
             * Intra referes to a communication within a tool layer
             * as opposed to between tool layers.
             */
            class CommStrategyIntra : public Module, public Configurable, public virtual Printable
            {
            public:
                /**
                 * Empty Constructor
                 */
                CommStrategyIntra ( );

                /**
                 * Constructor with arguments.
                 * @param moduleName Name of the up module.
                 * @param configName P^nMPI config name of the up module.
                 * @param settings list of SettingsDescriptions which lists the options for this strategy.
                 */
                CommStrategyIntra (
                        std::string moduleName,
                        std::string configName,
                        std::list<SettingsDescription*> settings);

                /**
                 * Constructor with arguments.
                 * @param moduleName Name of the up module.
                 * @param configName P^nMPI config name of the up module.
                 * @param instanceType datatype of a instance of this module.
                 * @param headerName header for this module.
                 * @param incDir include directory for this header.
                 * @param settings list of SettingsDescriptions which lists the options for this strategy.
                 */
                CommStrategyIntra (
                        std::string moduleName,
                        std::string configName,
                        std::string instanceType,
                        std::string headerName,
                        std::string incDir,
                        std::list<SettingsDescription*> settings);

                /**
                 * Empty Destructor
                 */
                virtual ~CommStrategyIntra ( );

                /**
                 * Hook method for printing,
                 * in order to enable the "<<" operator.
                 * @param out ostream to use.
                 * @return ostream after printing.
                 */
                virtual std::ostream& print (std::ostream& out) const;

            private:

            }; /*CommStrategyIntra*/
        } /*namespace modules*/
    } /*namespace weaver*/
} /*namepsace gti*/

#endif // COMMSTRATEGYINTRA_H
