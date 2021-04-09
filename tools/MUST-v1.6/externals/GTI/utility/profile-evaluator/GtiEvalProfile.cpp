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
 * @file GtiEvalProfile.cpp
 *      @see GtiEvalProfile
 *
 * @author Tobias Hilbrich
 * @date 18.07.2012
 */

/*
 * SEE CMakeLists.txt of this directory for details on HAVE_CIMG
 */
#ifdef HAVE_CIMG
#include "CImg.h"
#endif /*HAVE_CIMG*/
#include "GtiEvalProfile.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <list>
#include <cmath>
#include <cstdlib>
#include <string.h>

#ifdef HAVE_CIMG
using namespace cimg_library;
#endif

//=============================
// printUsage
//=============================
void printUsage (std::string execName, std::ostream &out)
{
    out
        << "Usage: "
        << execName
        << " <directory>" << std::endl
        << std::endl
        << "E.g.: " << execName << " ./"
        << std::endl;
}

//=============================
// colorForValue
//=============================
void colorForValue (uint64_t max, uint64_t value, unsigned char *ret)
{
    double ratio = (double)value/(double)max;

    if (ratio < 0.33)
    {
        double inRatio = ratio / 0.33;
        ret[0] = 0;
        ret[1] = 255*inRatio;
        ret[2] = 255*(1.0-inRatio);
    }
    else if (ratio < 0.66)
    {
        double inRatio = (ratio-0.33) / 0.33;
        ret[0] = 255*inRatio;
        ret[1] = 255;
        ret[2] = 0;
    }
    else
    {
        double inRatio = (ratio-0.66) / 0.33;
        if (inRatio > 1.0) inRatio = 1.0;
        ret[0] = 255;
        ret[1] = 255*(1.0-inRatio);
        ret[2] = 0;
    }
}

//=============================
// main
//=============================
int main(int argc, char** argv)
{
    //=========== 0 read input or print help ===========
    //print help if requested
    if ((argc == 2) &&
            (
                    (strncmp(argv[1], "--help", strlen("--help")) == 0 ) ||
                    (strncmp(argv[1], "-help", strlen("--help")) == 0 ) ||
                    (strncmp(argv[1], "-h", strlen("--help")) == 0 )
            )
    )
    {
        printUsage (argv[0], std::cout);
        return 0;
    }

    //enough arguments ?
    if (argc < 2)
    {
        std::cerr << "Error: Not enough arguments!" << std::endl << std::endl;
        printUsage (argv[0], std::cerr);
        return 1;
    }

    std::string directory = argv[1];
    int retVal = 0;
    GtiEvalProfile eval (directory, &retVal);

    return retVal;
}

//=============================
// TimeInfo::TimeInfo
//=============================
TimeInfo::TimeInfo (void)
 : usec (0), count (0)
{}

//=============================
// FunctionInfo::FunctionInfo
//=============================
FunctionInfo::FunctionInfo (std::string name)
 : name (name), wrap (), recvl ()
{}

//=============================
// FunctionInfo::FunctionInfo
//=============================
FunctionInfo::FunctionInfo ()
 : name (), wrap (), recvl ()
{}

//=============================
// ModuleInfo::ModuleInfo
//=============================
ModuleInfo::ModuleInfo (std::string name)
 : name (name), total (), functions ()
{}

//=============================
// ModuleInfo::ModuleInfo
//=============================
ModuleInfo::ModuleInfo ()
 : name (), total (), functions ()
{}

//=============================
// LayerInfo::LayerInfo
//=============================
LayerInfo::LayerInfo (int order, int size, bool isBlockDistribution, int blocksize)
 : myAncestor (NULL),
   myDescendant (NULL),
   myOrder (order),
   mySize (size),
   myAncestorDistribIsBlock (isBlockDistribution),
   myBlocksize (blocksize),
   myTimes (),
   myAvgTime (),
   myAnalysisTimes (),
   myAvgAnalysisTime ()
{
    for (int i = 0; i < IDX_NUM_IDX; i++)
    {
        myTimes[i].resize(size);
    }
}

//=============================
// LayerInfo::~LayerInfo
//=============================
LayerInfo::~LayerInfo (void)
{
    //Nothing to do
}

//=============================
// calculateAverages
//=============================
void LayerInfo::calculateAverages (void)
{
    //==1) Calculate the basic time averages
    for (int idx = 0; idx < IDX_NUM_IDX; idx++)
    {
        TimeInfo sum;

        for (unsigned int p = 0; p < myTimes[idx].size(); p++)
        {
            sum.usec += myTimes[idx][p].usec;
            sum.count += myTimes[idx][p].count;
        }

        myAvgTime[idx].usec = (uint64_t)((double)sum.usec / (double)myTimes[idx].size());
        myAvgTime[idx].count = (uint64_t)((double)sum.count / (double)myTimes[idx].size());

        ///*DEBUG*/ std::cout << idx << ":" << myAvgTime[idx].usec << "@" << myAvgTime[idx].count << std::endl;
    }

    //==2) Calculate the analysis time averages
    std::map<std::string, std::vector<ModuleInfo> >::iterator iter;
    std::map<std::string, FunctionInfo>::iterator fIter;
    for (iter = myAnalysisTimes.begin(); iter != myAnalysisTimes.end(); iter++)
    {
        std::string modName = iter->first;
        myAvgAnalysisTime[modName].name = modName;

        TimeInfo modSum;
        for (unsigned int p = 0; p != iter->second.size(); p++)
        {
            //Total sum
            modSum.usec += iter->second[p].total.usec;
            modSum.count += iter->second[p].total.count;

            //Function sum
            for (fIter = iter->second[p].functions.begin(); fIter != iter->second[p].functions.end(); fIter++)
            {
                if (p == 0) myAvgAnalysisTime[modName].functions[fIter->first].name = fIter->first;

                myAvgAnalysisTime[modName].functions[fIter->first].recvl.usec += fIter->second.recvl.usec;
                myAvgAnalysisTime[modName].functions[fIter->first].recvl.count += fIter->second.recvl.count;
                myAvgAnalysisTime[modName].functions[fIter->first].wrap.usec += fIter->second.wrap.usec;
                myAvgAnalysisTime[modName].functions[fIter->first].wrap.count += fIter->second.wrap.count;
            }
        }

        //Total average
        myAvgAnalysisTime[modName].total.usec =  (uint64_t)((double)modSum.usec / (double)iter->second.size());
        myAvgAnalysisTime[modName].total.count =  (uint64_t)((double)modSum.count / (double)iter->second.size());

        //Function average
        for (fIter = myAvgAnalysisTime[modName].functions.begin(); fIter != myAvgAnalysisTime[modName].functions.end(); fIter++)
        {
            fIter->second.recvl.usec = (uint64_t) ((double)fIter->second.recvl.usec / (double)mySize);
            fIter->second.recvl.count = (uint64_t) ((double)fIter->second.recvl.count / (double)mySize);
            fIter->second.wrap.usec = (uint64_t) ((double)fIter->second.wrap.usec / (double)mySize);
            fIter->second.wrap.count = (uint64_t) ((double)fIter->second.wrap.count / (double)mySize);

            ///*DEBUG*/ std::cout << modName << ":" << fIter->second.name << "=" << fIter->second.recvl.usec << ":" << fIter->second.recvl.count << "|" << fIter->second.wrap.usec << ":" << fIter->second.wrap.count << std::endl;
        }
    }
}

//=============================
// Constructor
//=============================
GtiEvalProfile::GtiEvalProfile (std::string directory, int *pOutRetVal)
 : myLayers ()
{
    if (pOutRetVal) *pOutRetVal = 1;

    //1) Read the profiles
    if (!readProfiles (directory))
        return;

    //2) Create data and script file for gnuplot summary
    if (!printGnuplot ())
        return;

    //3) Create heat maps
    if (!printHeat ("Heat Map of Total Analysis Time", "gti_profile_heat_analysis_time.tiff", IDX_ANA))
        return;
    if (!printHeat ("Heat Map of Idle Time", "gti_profile_heat_idle_time.tiff", IDX_IDLE))
        return;
    if (!printHeat ("Heat Map of Flood Control Maximum Badness", "gti_profile_heat_max_bad.tiff", IDX_BAD, true))
        return;
    if (!printHeat ("Heat Map of Upwards-Communication Time", "gti_profile_heat_up_comm_time.tiff", IDX_UP))
        return;
    if (!printHeat ("Heat Map of Downwards-Communication Time", "gti_profile_heat_down_comm_time.tiff", IDX_DOWN))
        return;
    if (!printHeat ("Heat Map of Intra-Communication Time", "gti_profile_heat_intra_comm_time.tiff", IDX_INTRA))
        return;
    if (!printHeat ("Heat Map of Infrastructure Overhead", "gti_profile_heat_infrastructure_time.tiff", IDX_INFRA))
        return;
    if (!printHeat ("Heat Map of Maximum event queue size in place driver", "gti_profile_heat_place_max_queue.tiff", IDX_NUM_IDX, true, true, "ThreadedMpiPlace", "maxEventQueue"))
        return;
    if (!printHeat ("Heat Map of Maximum number of events in DP2PMatch", "gti_profile_heat_max_dp2p_queue.tiff", IDX_NUM_IDX, true, true, "DP2PMatch", "maxEventQueue"))
        return;
    if (!printHeat ("Heat Map of Maximum trace size in DWaitState", "gti_profile_heat_max_dwaitstate_trace.tiff", IDX_NUM_IDX, true, true, "DWaitState", "maxTraceSize"))
        return;
    if (!printHeat ("Hear Map of analysis invocation counts", "gti_profile_heat_analysis_count.tiff", IDX_ANA, true, false))
        return;

    if (pOutRetVal) *pOutRetVal = 0;
}

//=============================
// Destructor
//=============================
GtiEvalProfile::~GtiEvalProfile (void)
{
    for (unsigned int i = 0; i < myLayers.size(); i++)
        if (myLayers[i]) delete myLayers[i];
    myLayers.clear();
}

//=============================
// readProfiles
//=============================
bool GtiEvalProfile::readProfiles (std::string directory)
{
    bool success = true;
    int numLayers = 0;

    //How many layers?
    do
    {
        std::stringstream profileName;
        profileName << directory << "/gti_layer_" << numLayers << ".profile";
        std::ifstream test;

        try
        {
            test.open(profileName.str().c_str());
            if (test.fail()) success = false;
            test.close();
        }
        catch (std::exception& e)
        {
            success = false;
        }

        if (success)
            numLayers++;
    } while (success);

    //Abort if no layers
    if (numLayers == 0)
        return false;

    //Read layers
    myLayers.resize(numLayers);
    for (int i = 0; i < numLayers; i++)
    {
        std::stringstream profileName;
        profileName << directory << "/gti_layer_" << i << ".profile";

        myLayers[i] = readProfile (profileName.str());
        if (!myLayers[i]) return false;

        if (i > 0)
        {
            myLayers[i-1]->myDescendant = myLayers[i];
            myLayers[i]->myAncestor = myLayers[i-1];
        }
    }

    return true;
}

//=============================
// Helper: search
//=============================
inline bool search (std::ifstream &in, std::string str)
{
    std::string res;
    in >> res;

    while (in.good())
    {
        //Evaluate value
        if (res == str)
            return true;

        //Next
        in >> res;
    }

    return false;
}

//=============================
// readProfile
//=============================
LayerInfo* GtiEvalProfile::readProfile (std::string fileName)
{
    LayerInfo* ret;
    std::string str;
    std::ifstream in (fileName.c_str());
    int index, size, blocksize = 0;
    bool isBlock = false;

    //Index of this layer
    if (!search (in, "index")) return NULL;
    in >> str;
    index = atoi (str.c_str());

    //Distribution
    if (index > 0)
    {
        std::stringstream distribNameStream;
        distribNameStream << "levelDistribution_" << index -1 << "_" << index;
        if (!search (in, distribNameStream.str())) return NULL;
        in >> str;
        if (str == "by-block")
            isBlock = true;

        if (isBlock)
        {
            std::stringstream bsizeNameStream;
            bsizeNameStream << "levelBlocksize_" << index -1 << "_" << index;
            if (!search (in, bsizeNameStream.str())) return NULL;
            in >> str;
            blocksize = atoi (str.c_str());
        }
    }

    //size of this layer
    std::stringstream sizeNameStream;
    sizeNameStream << "levelSize_" << index;
    if (!search (in, sizeNameStream.str())) return NULL;
    in >> str;
    size = atoi (str.c_str());

    //Create the layer
    ret = new LayerInfo (index, size, isBlock, blocksize);

    //Basic performance data
    std::string keys[IDX_NUM_IDX] = {"totalTime", "idleTime", "maxFloodBadness", "infrastructureTime", "downTime", "upTime", "intraTime", "timeoutTime", "analysisTime"};

    for (int i = 0; i < IDX_NUM_IDX; i++)
    {
        if (!search (in, keys[i]))
        {
            //If not present, set to 0
            for (int s = 0; s < size; s++)
            {
                ret->myTimes[i][s].usec = 0;
                ret->myTimes[i][s].count = 0;
            }
        }
        else
        {
            for (int s = 0; s < size; s++)
            {
                uint64_t usec, count;
                in >> usec;
                in >> count;

                ret->myTimes[i][s].usec = usec;
                ret->myTimes[i][s].count = count;
            }
        }
    }

    //Per module performance data
    while (search (in, "module"))
    {
        std::string modName;
        int numFunctions;

        in >> modName;
        ret->myAnalysisTimes[modName].resize(size);

        //Num functions
        if (!search (in, "numFunctions")) return NULL;
        in >> str;
        numFunctions = atoi (str.c_str());

        for (int i = 0; i < numFunctions; i++)
        {
            std::string functionName;

            //Wrapper information
            if (!search (in, "wraperFunction")) return NULL;
            in >> functionName;

            for (int s = 0; s < size; s++)
            {
                uint64_t usec, count;
                in >> usec;
                in >> count;

                ret->myAnalysisTimes[modName][s].name = modName;
                ret->myAnalysisTimes[modName][s].functions[functionName].name = functionName;

                ret->myAnalysisTimes[modName][s].total.usec += usec;
                ret->myAnalysisTimes[modName][s].total.count += count;

                ret->myAnalysisTimes[modName][s].functions[functionName].wrap.usec = usec;
                ret->myAnalysisTimes[modName][s].functions[functionName].wrap.count = count;
            }

            //Receival information
            if (!search (in, "receivalFunction")) return NULL;
            in >> functionName;

            for (int s = 0; s < size; s++)
            {
                uint64_t usec, count;
                in >> usec;
                in >> count;

                ret->myAnalysisTimes[modName][s].total.usec += usec;
                ret->myAnalysisTimes[modName][s].total.count += count;

                ret->myAnalysisTimes[modName][s].functions[functionName].recvl.usec = usec;
                ret->myAnalysisTimes[modName][s].functions[functionName].recvl.count = count;
            }
        }

    }

    //Calculate averages
    ret->calculateAverages();

    return ret;
}

//=============================
// printGnuplotData
//=============================
bool GtiEvalProfile::printGnuplot (void)
{
    /*
     * Preparation, determine top consumer modules and module functions
     */
    std::set<std::string> topModules; //set of top modules
    std::set<std::string>::iterator topModIter;
    std::set<std::pair<std::pair<std::string,std::string> , bool> > topFunctions; //set of top function module and function name pair and a bool that specifies whether its the wrapper (=true) or the receival (=false)
    std::set<std::pair<std::pair<std::string,std::string> , bool> >::iterator topFctIter;
    double maxTPerOp = 0;

    for (unsigned int i = 0; i < myLayers.size(); i++)
    {
        LayerInfo *cur = myLayers[i];

        //Determine top modules and top function: Create ordered map
        std::multimap<uint64_t, std::string> modOrder;
        std::multimap<uint64_t, std::pair<std::pair<std::string,std::string>, bool> > fctOrder;

        std::map<std::string, ModuleInfo>::iterator modIter;
        for (modIter = cur->myAvgAnalysisTime.begin(); modIter != cur->myAvgAnalysisTime.end(); modIter++)
        {
            //Add module to order
            modOrder.insert(std::make_pair (modIter->second.total.usec, modIter->first));

            //Add functions to order
            std::map<std::string, FunctionInfo>::iterator fIter;
            for (fIter = modIter->second.functions.begin(); fIter != modIter->second.functions.end(); fIter++)
            {
                fctOrder.insert(
                        std::make_pair(
                                fIter->second.wrap.usec,
                                std::make_pair (
                                        std::make_pair (
                                                modIter->first,
                                                fIter->first
                                        ),
                                        true)
                        ));
                fctOrder.insert(
                        std::make_pair(
                                fIter->second.recvl.usec,
                                std::make_pair (
                                        std::make_pair (
                                                modIter->first,
                                                fIter->first
                                        ),
                                        false)
                        ));
            }
        }

        //Determine top modules and top function: Select top functions from ordered map
        int numTops = 0;
        std::multimap<uint64_t, std::string>::reverse_iterator modOrderIter;
        std::multimap<uint64_t, std::pair<std::pair<std::string,std::string>, bool> >::reverse_iterator fctOrderIter;

        for (
                modOrderIter = modOrder.rbegin(), numTops = 0;
                (modOrderIter != modOrder.rend()) && (numTops < GTI_EVAL_NUM_TOP_MODS);
                modOrderIter++, numTops++)
        {
            topModules.insert(modOrderIter->second);
        }

        for (
                fctOrderIter = fctOrder.rbegin(), numTops = 0;
                (fctOrderIter != fctOrder.rend()) && (numTops < GTI_EVAL_NUM_TOP_MODS);
                fctOrderIter++, numTops++)
        {
            topFunctions.insert(
                    fctOrderIter->second
            );
        }
    }

    /*
     * PLOT 1: Normalized stacked bar plot
     * - One bar for each layer
     * - values are normalized basic times (i.e. percentages)
     *
     * PLOT 2: Normalized stacked bar plot
     * - One bar for each layer
     * - values are top N modules names (use top N per layer)
     *
     * PLOT 3: Normalized stacked bar plot
     * - One bar for each layer
     * - values are top N analysis functions (use top N per layer)
     *
     * PLOT 4: Bar chart, analysis time per op
     * - One bar for each layer
     */

    /*
     * Print the data file that we use in the gnuplot script
     */
    std::ofstream data ("gti_profile_plot.data");
    int
        plot1FirstCol = 2,
        plot2FirstCol,
        plot3FirstCol,
        plot4FirstCol;


    for (unsigned int i = 0; i < myLayers.size(); i++)
    {
        LayerInfo *cur = myLayers[i];
        data << i+1 << "\t";

        //Plot1: sum basic times and print fractions
        uint64_t sum = 0;
        for (int d = 1; d < IDX_NUM_IDX; d++) //Start with 1, we skipp the total time
            sum += cur->myAvgTime[d].usec;

        for (int d = 1; d < IDX_NUM_IDX; d++)
        {
            data << (double)cur->myAvgTime[d].usec / (double)sum<< "\t";
        }

        //Plot2
        plot2FirstCol = plot3FirstCol = IDX_NUM_IDX + 1;

        double dSum = 0.0;
        for (topModIter = topModules.begin(); topModIter != topModules.end(); topModIter++, plot3FirstCol++)
        {
            double fraction = (double)cur->myAvgAnalysisTime[*topModIter].total.usec / (double)cur->myAvgTime[IDX_ANA].usec;
            data << fraction << "\t";
            dSum += fraction;
        }

        //add "other" column for module fractions
        data << 1.0 - dSum << "\t";
        plot3FirstCol++;

        //Plot3
        plot4FirstCol = plot3FirstCol;
        dSum = 0;
        for (topFctIter = topFunctions.begin(); topFctIter != topFunctions.end(); topFctIter++,plot4FirstCol++)
        {
            std::string
                modName = (*topFctIter).first.first,
                fctName = (*topFctIter).first.second;
            bool isWrapper = (*topFctIter).second;

            double fraction;
            if (isWrapper)
                fraction = (double)cur->myAvgAnalysisTime[modName].functions[fctName].wrap.usec / (double)cur->myAvgTime[IDX_ANA].usec;
            else
                fraction = (double)cur->myAvgAnalysisTime[modName].functions[fctName].recvl.usec / (double)cur->myAvgTime[IDX_ANA].usec;

            dSum += fraction;
            data << fraction << "\t";
        }

        data << 1.0 - dSum << "\t";
        plot4FirstCol++;

        //Plot4
        double tPerOp = (double)cur->myAvgTime[IDX_ANA].usec / (double)cur->myAvgTime[IDX_ANA].count;
        data << tPerOp << "\t";

        if (tPerOp > maxTPerOp)
            maxTPerOp = tPerOp;

        data << std::endl;
    }

    data.close ();

    /*
     * Print the gnuplot script
     */
    std::ofstream plot ("gti_profile_script.gnuplot");

    //==1) Basic stuff
    plot
        << "set size 1.0" << std::endl
        << "set terminal jpeg size 2000, 1600" << std::endl
        << "set output \"gti_profile_plot.jpeg\"" << std::endl
        << "set boxwidth 0.75 absolute" << std::endl
        << "set key outside box" << std::endl
        << "set style fill solid 1.00 border 1" << std::endl
        << "set style histogram rowstacked" << std::endl
        << "set style data histograms" << std::endl
        << "set yrange [0:1.0]" << std::endl
        << "set ylabel \"Fraction of total runtime\"" << std::endl
        << "set xtics ( \"Application Layer\" 0";

    for (unsigned int i = 1; i < myLayers.size(); i++)
    {
        plot << ", \"GTI Layer " << i << "\" " << i;
    }

    plot
        << " )" << std::endl
        << "" << std::endl;

    //==2) Plot 1
    plot
        << "set multiplot" << std::endl
        << "set origin 0, 0.5; \\" << std::endl
        << "set size 0.5, 0.5; \\" << std::endl
        << "set title \"Layer Activity\"; \\" << std::endl
        << "plot";

    for (int i = 1; i < IDX_NUM_IDX; i++)
    {
        if (i != 1) plot << ",";
        plot << " 'gti_profile_plot.data' using " << i+plot1FirstCol-1 << " t \"" << ValueNames[i] << "\"";
    }
    plot << std::endl << std::endl;

    //==3) Plot 2
    plot
    << "set origin 0.5, 0.5; \\" << std::endl
    << "set size 0.5, 0.5; \\" << std::endl
    << "set ylabel \"Fraction of total analysis time\"" << std::endl
    << "set title \"Most Expensive Analysis Modules\"; \\" << std::endl
    << "plot";

    int column = plot2FirstCol;
    for (topModIter = topModules.begin(); topModIter != topModules.end(); topModIter++, column++)
    {
        plot << " 'gti_profile_plot.data' using " << column << " t \"" << *topModIter << "\", ";
    }
    plot << "'gti_profile_plot.data' using " << column << " t \"Other modules\"" << std::endl << std::endl;

    //==4) Plot 3
    plot
    << "set origin 0.0, 0.0; \\" << std::endl
    << "set size 0.5, 0.5; \\" << std::endl
    << "set title \"Most Expensive Analysis Functions\"; \\" << std::endl
    << "plot";

    column = plot3FirstCol;
    for (topFctIter = topFunctions.begin(); topFctIter != topFunctions.end(); topFctIter++, column++)
    {
        std::string
            modName = (*topFctIter).first.first,
            fctName = (*topFctIter).first.second;
        bool isWrapper = (*topFctIter).second;

        plot << " 'gti_profile_plot.data' using " << column << " t \"" << modName << ":" << fctName;
        if (isWrapper)
            plot << "(W)";
        else
            plot << "(R)";
        plot << "\", ";
    }
    plot << "'gti_profile_plot.data' using " << column << " t \"Other functions\"" << std::endl << std::endl;

    //==5) Plot 4
    plot
        << "set origin 0.5, 0.0; \\" << std::endl
        << "set size 0.5, 0.5; \\" << std::endl
        << "set yrange [0:" << ceil(maxTPerOp) << "]" << std::endl
        << "set ylabel \"t/op [usec]\"" << std::endl
        << "set title \"Average Analysis Time per Operation\"; \\" << std::endl
        << "plot 'gti_profile_plot.data' using " << plot4FirstCol << " t \"Avg t/op\"" << std::endl;

    plot.close();

    return 1;
}

struct Triangle
{
    int x[3], y[3]; //Coordinates
    unsigned char c; //Color
    uint64_t value;
};

//=============================
// printHeat
//=============================
bool GtiEvalProfile::printHeat (
        std::string title,
        std::string outFileName,
        ValueIndices valueIndex,
        bool isCount,
        bool isModAnalysis,
        std::string modName,
        std::string modAnalysis)
{
#ifdef HAVE_CIMG
    //How many slots do we have?
    int numSlots = myLayers[0]->mySize;
    int compression = 1;
    int height = 1, width = GTI_EVAL_TRIANGLE_DX;
    while (numSlots > GTI_EVAL_MAX_SLOTS)
    {
        compression*=2;
        numSlots = ceil((float)myLayers[0]->mySize / (float)compression);
    }

    while ((height+1)*numSlots < GTI_EVAL_MAX_SLOTS)
        height++;

    while ((width+1)*myLayers.size() < GTI_EVAL_MAX_SLOTS)
        width++;

    //Create the picture
    int
        totalHeight = height*numSlots + GTI_EVAL_TITEL_HEIGHT,
        totalWidth = width * myLayers.size() + GTI_EVAL_LEGEND_SIZE;
    CImg<unsigned char> heat;
    heat.assign(totalWidth,totalHeight,1,3, 0);

    //Print legend
    for (int i = 0; i< totalHeight-GTI_EVAL_TITEL_HEIGHT;i++)
    {
        unsigned char color[3];
        colorForValue (totalHeight-GTI_EVAL_TITEL_HEIGHT, i, color);
        heat.draw_line(0,i,GTI_EVAL_LEGEND_COLORS,i,color);
    }

    //What is the global maximum value?
    uint64_t theMax = 0;
    for (unsigned int l = 0; l < myLayers.size(); l++)
    {
        for (int p = 0; p < myLayers[l]->mySize; p++)
        {
            uint64_t thisVal = getHeatVal (l,p,valueIndex,isCount,isModAnalysis,modName,modAnalysis);

            if (thisVal > theMax)
                theMax =thisVal;
        }
    }

    //Print Title
    float maxToPrint = (float)theMax/(float)1000000;
    std::string unit = "s";
    if (isCount)
    {
        maxToPrint = (float)theMax;
        unit = "";
    }
    unsigned char textColor[] = {255,255,255}, bgColor[] = {0,0,0};
    heat.draw_text  (
            10, height*numSlots + (GTI_EVAL_TITEL_HEIGHT-GTI_EVAL_TITEL_TEXT)/2,
            "%s (Maximum value = %f%s)",
            textColor,
            bgColor,
            1,
            GTI_EVAL_TITEL_TEXT,
            title.c_str(),
            unit.c_str(),
            maxToPrint
        );

    std::map <int, std::list<int> > lastPos, newPos; //maps slot to processes that fall into it
    int x = GTI_EVAL_LEGEND_SIZE;
    for (unsigned int l = 0; l < myLayers.size(); l++, x+=width)
    {
        int p = 0;
        for (int s = 0; s < numSlots; s++)
        {
            //Values for first level
            if (l == 0)
            {
                uint64_t value, maxValue = 0;
                int c;
                for (c = 0; c < compression && p < myLayers[l]->mySize; c++, p++)
                {
                    uint64_t thisValue = getHeatVal (l,p,valueIndex,isCount,isModAnalysis,modName,modAnalysis);
                    value += thisValue;

                    if (thisValue > maxValue)
                        maxValue = thisValue;

                    newPos[s].push_back(p);
                }
                value = value / c;

                Triangle t1,t2;
                t1.x[0] = t2.x[0] = t2.x[2] = x;
                t1.x[1] = t2.x[1] = t1.x[2] = x+width;
                t1.y[0] = t1.y[1] = t2.y[0] = s*height;
                t1.y[2] = t2.y[2] = t2.y[1] = s*height+height;
                t1.value = value;
                t2.value = value;

                unsigned char color[3];
                colorForValue (theMax, maxValue, color); //TODO value/maxValue ?
                heat.draw_triangle(t1.x[0],t1.y[0], t1.x[1], t1.y[1], t1.x[2], t1.y[2], color);
                heat.draw_triangle(t2.x[0],t2.y[0], t2.x[1], t2.y[1], t2.x[2], t2.y[2], color);
            }
            else
            //Values for other layers
            {
                std::list<int> ancestorProcs = lastPos[s];
                std::list<int>::iterator iter;
                std::map<int, int> thisProcs; //maps process in this layer to its occurrence count

                for (iter = ancestorProcs.begin(); iter != ancestorProcs.end(); iter++)
                {
                    int a = *iter;
                    int p;

                    //determine which process in this layer receives information from ancestor a
                    if (myLayers[l]->myAncestorDistribIsBlock)
                    {
                        p = a / myLayers[l]->myBlocksize;
                    }
                    else
                    {
                        int num_partners = myLayers[l-1]->mySize / myLayers[l]->mySize;
                        int remaining = myLayers[l-1]->mySize - num_partners *  myLayers[l]->mySize;

                        for (int i = 1; i < myLayers[l]->mySize + 2; i++)
                        {
                            if (a < i * num_partners + std::min(remaining,i))
                            {
                                p = i - 1;
                                break;
                            }
                        }
                    }//By-Block or UNIFORM?

                    thisProcs[p] = thisProcs[p] + 1;
                }//For ancestors that fall into this slot

                std::map<int, int>::iterator thisIter;
                uint64_t value = 0, maxValue = 0;

                double totalOc = 0;
                for (thisIter = thisProcs.begin(); thisIter != thisProcs.end(); thisIter++)
                    totalOc += thisIter->second;

                for (thisIter = thisProcs.begin(); thisIter != thisProcs.end(); thisIter++)
                {
                    uint64_t thisValue = getHeatVal (l,thisIter->first,valueIndex,isCount,isModAnalysis,modName,modAnalysis);
                    if (thisValue > maxValue)
                        maxValue=thisValue;
                    value += thisValue * ((double)thisIter->second/totalOc);
                    newPos[s].push_back(thisIter->first);
                }

                Triangle t1,t2;
                t1.x[0] = t2.x[0] = t2.x[2] = x;
                t1.x[1] = t2.x[1] = t1.x[2] = x+width;
                t1.y[0] = t1.y[1] = t2.y[0] = s*height;
                t1.y[2] = t2.y[2] = t2.y[1] = s*height+height;
                t1.value = value;
                t2.value = value;

                unsigned char color[3];
                colorForValue (theMax, maxValue, color);//TODO value/maxValue/?
                heat.draw_triangle(t1.x[0],t1.y[0], t1.x[1], t1.y[1], t1.x[2], t1.y[2], color);
                heat.draw_triangle(t2.x[0],t2.y[0], t2.x[1], t2.y[1], t2.x[2], t2.y[2], color);
            }
        }

        //Update process positions
        lastPos.clear();
        lastPos = newPos;
        newPos.clear();
    }

    heat.save_tiff(outFileName.c_str());

    /*//??? Min, Max, Avg chart
    std::list<Triangle> tris;
    int numSelected = 0;
    int selected[3];
    for (int l = myLayers.size()-1; l >= 0; l--)
    {
        //If no selection done, select freely from this layer!

        for (int p = 0; p < myLayers[l]->mySize; p++)
        {

        }
    }
    */
    /*
    int
        totalHeight = GTI_EVAL_TRIANGLE_DY * myLayers[0]->mySize,
        totalWidth = GTI_EVAL_TRIANGLE_DX * myLayers.size();



    CImg<unsigned char> heat;
    heat.assign(totalWidth,totalHeight,1,1,0);

    int xStart = 0, yStart;
    for (int l = 0; l < myLayers.size(); l++, xStart+=GTI_EVAL_TRIANGLE_DX)
    {
        LayerInfo* cur = myLayers[l];

        yStart = 0;
        for (long p = 0; p < cur->mySize; p++)
        {

        }
    }

    heat.save_tiff(outFileName.c_str());
*/
#endif /*HAVE_CIMG*/
    return 1;
}

//=============================
// getHeatVal
//=============================
uint64_t GtiEvalProfile::getHeatVal (
        unsigned int l,
        int p,
        ValueIndices valueIndex,
        bool isCount,
        bool isModAnalysis,
        std::string modName,
        std::string modAnalysis)
{
    if (!isModAnalysis)
    {
        if (!isCount)
            return myLayers[l]->myTimes[valueIndex][p].usec;
        else
            return myLayers[l]->myTimes[valueIndex][p].count;
    }

    std::map<std::string, std::vector<ModuleInfo> >::iterator pos = myLayers[l]->myAnalysisTimes.find(modName);
    if (pos == myLayers[l]->myAnalysisTimes.end())
        return 0;

    std::map<std::string, FunctionInfo>::iterator pos2 = pos->second[p].functions.find(modAnalysis);
    if (pos2 == pos->second[p].functions.end())
        return 0;

    if (isCount)
        return pos2->second.wrap.count;

    return pos2->second.wrap.usec;
}

/*EOF*/
