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
 * @page CTest Information on testing in MUST/GTI
 *
 * This chapter describes  how to use CTest in MUST/GTI
 *
 * CTest is a testing tool that collects test
 * results for a dashboard system. A dashboard was set up for ZIH and runs on http://141.30.75.44:8081.
 *
 * @section a Experimental submit
 * To submit a Test to the Dashboard just start the testing environment of GTI/MUST with the Command
 * @code
     ctest -D Experimental
   @endcode
 * CTest will run your Tests and send the results to the Dashboard server.
 * You can find, the results in the "Experimental" table in the Dashboard.
 * There you can read the output by click on the status of each test.
 *
 * @section CTestScript Script for Nightly Tests
 * To get a continuous overview of Tests on different machines you can find
 * a script called nightly.sh in the utility directory of must. This script
 * checks out the latest versions of GTI and MUST from their respective repositories, installs
 * them and submits the results to the dashboard server mentioned above.
 * To use this Script on your machine, you will likely have to do some adaptions in the script
 * to setup which compiler to  use and so on. Note, this will likely get less painful once we
 * have a certain amount of system inspection in the CMake files.
 * First of all, edit the nightly.sh script and change the SRC_DIR variable to the
 * Path of the directory where the script is. Then open config.sh and change the
 * variables there. Afterwards, you can start the script. This submits the results
 * to the dashboard. The test results of the nightly script are available in the nightly
 * table of the dashboard server. There is only one submit shown for each system and day for nightly tests.
 *
 * @section CTestCron Cronjobs
 * To get regularl submissions, it is recommended that the script is started by a cronjob.
 * To install a cronjob open your crontab by entering this command:
 *
 *@code
	crontab -e
  @endcode
 *
 * And add this line:
 * @code
      00 23 * * * <path_to_the_script>
   @endcode
 *
 * This starts the script at 11 pm every day. You can change the time where the script should
 * start by changing the first two numbers. The first numbers is the minute (0-59) and
 * the second one is the hour (0-23), at which the script should run.
 *
 *
 */
