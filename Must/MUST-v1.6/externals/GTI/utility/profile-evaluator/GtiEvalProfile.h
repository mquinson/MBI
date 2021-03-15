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
 * @file GtiEvalProfile.h
 *      @see GtiEvalProfile
 *
 * @author Tobias Hilbrich
 * @date 18.07.2012
 */

#include <string>
#include <map>
#include <vector>
#include <stdint.h>

#define GTI_EVAL_NUM_TOP_MODS 5
#define GTI_EVAL_TRIANGLE_DX 10
#define GTI_EVAL_TRIANGLE_DY 10
#define GTI_EVAL_MAX_SLOTS 1024
#define GTI_EVAL_LEGEND_SIZE 20
#define GTI_EVAL_LEGEND_COLORS 15
#define GTI_EVAL_TITEL_HEIGHT 20
#define GTI_EVAL_TITEL_TEXT 13

/**
 * Indices to arrays of time info values.
 */
enum ValueIndices
{
    IDX_TOTAL = 0,
    IDX_IDLE,
    IDX_BAD,
    IDX_INFRA,
    IDX_DOWN,
    IDX_UP,
    IDX_INTRA,
    IDX_TOUT,
    IDX_ANA,
    IDX_NUM_IDX
};

/**
 * Names for values.
 */
const char ValueNames[][256] = {
    "Total Runtime",
    "Idle/Application Time",
    "Flood-Control Maximum Badness",
    "Infrastructure Overhead",
    "Downwards Communication",
    "Upwards Communication",
    "Intra Communication",
    "Timeout Handling",
    "Total Analysis Time"
    };

/**
 * Type used to capture a usec, invocation pair.
 */
class TimeInfo
{
public:
    TimeInfo (void);

    uint64_t usec; //usec spent for activity
    uint64_t count; //invocation count for activity (some activities may not have that)
};

/**
 * Type used to capture timing information for a module function.
 */
class FunctionInfo
{
public:
    FunctionInfo (std::string name);
    FunctionInfo (void);

    std::string name;
    TimeInfo wrap, recvl;
};

/**
 * Type used to capture timing information for a module.
 */
class ModuleInfo
{
public:
    ModuleInfo (std::string name);
    ModuleInfo (void);

    std::string name;
    TimeInfo total; /**< Total analysis time, total invocation count.*/
    std::map<std::string, FunctionInfo> functions;
};

/**
 * Timing information for a complete layer
 */
class LayerInfo
{
public:
    /**
     * Constructor.
     */
    LayerInfo (int order, int size, bool isBlockDistribution, int blocksize);

    /**
     * Destructor.
     */
    ~LayerInfo (void);

    /**
     * Starts calculation of average values.
     */
    void calculateAverages (void);

    LayerInfo* myAncestor;
    LayerInfo* myDescendant;

    //Layout information
    int myOrder;
    int mySize;
    bool myAncestorDistribIsBlock; /**< Default (if false) is uniform.*/
    int myBlocksize;

    //Time information
    std::vector<TimeInfo> myTimes[IDX_NUM_IDX];
    TimeInfo myAvgTime[IDX_NUM_IDX];

    std::map<std::string, std::vector<ModuleInfo> > /**< Maps module name to vector of module infos.*/
        myAnalysisTimes;
    std::map<std::string, ModuleInfo>
        myAvgAnalysisTime;
};

/**
 * Class for evaluating and processing profiling data from GTI.
 */
class GtiEvalProfile
{
public:
    /**
     * Constructor, starts the processing.
     * @param directory where the profiling data resides
     * @param pOutRetVal pointer to storage for int, set to 0 if successfull, to 1 otherwise.
     */
    GtiEvalProfile (std::string directory, int *pOutRetVal);

    /**
     * Destructor.
     */
    ~GtiEvalProfile (void);

protected:
    std::vector<LayerInfo*> myLayers;

    /**
     * Reads all profiles found in the directory.
     */
    bool readProfiles (std::string directory);

    /**
     * Prints the gnuplot data and script files to be used for ploting with gnuplot.
     */
    bool printGnuplot (void);

    /**
     * Prints a heat map.
     * @param title of the heat map.
     * @param outFileName name of the output file to use.
     * @param valueIndex for the metric to print as heat.
     */
    bool printHeat (
            std::string title,
            std::string outFileName,
            ValueIndices valueIndex,
            bool isCount = false,
            bool isModAnalysis = false,
            std::string modName = "",
            std::string modAnalysis = "");

    /**
     * Helper function to retrieve a single data value for a heat map
     * Most params see printHeat.
     * @param l layer.
     * @param p process.
     * @return data value (count or time[usec])
     */
    inline uint64_t getHeatVal (
                unsigned int l,
                int p,
                ValueIndices valueIndex,
                bool isCount,
                bool isModAnalysis,
                std::string modName,
                std::string modAnalysis);

    /**
     * Reads a single profile.
     */
    LayerInfo* readProfile (std::string fileName);

};

/*EOF*/
