MUST (Marmot Umpire Scalable Tool)

MUST automatically checks parallel applications for correct use of MPI. 
It operates at runtime and intercepts all MPI calls that an application 
issues to verify their correctness. It uses the Generic Tool 
Infrastructure (GTI), and PnMPI as base packages.

The MUST documentation is located at doc/manual/manual.pdf.
The License file is located directly in this package, named LICENSE.txt

MUST uses CMake as a build system to enable compatibility to multiple
operating systems. See the CMake documentation as well as the MUST 
documentation for further details on how to build and install with 
CMake.


----------------------------
CHANGE-LOG
----------------------------
v1.4.0-rc1 (November 2014):
- Capture and replay mode allow MUST to store and replay correctness errors (for deterministic applications)
- Allinea DDT integration
-- Uses DDT manual launch mode to start an application with mustrun and to connect it to a waiting instance of DDT
-- A DDT plugin automatically sets breakpoint to stop the debugger when an error is detected
-- Capture and replay enables breakpoints for correctness errors that MUST detects asynchronously
- Improved unfoldable HTML reports
- Adapted handling for MPI_Test_cancelled to widen the range of errors MUST can detect
- Smaller bug fixes and portability improvements

v1.3.0 (February 2014):
- Largely distributed deadlock detection scheme that only uses a centralized graph detectionafter a timeout (scalability ~10,000 processes)
- Optimizations and corrections for intralayer based type matching used to handle irregular MPI collective communications (e.g., MPI_Alltoallv)
- Checks for MPI communication buffer attachment (MPI_Buffer_attach and related functions)
- New partial MPI Request tracking module that only tracks persitent requests enables performance improvements for several MUST modules
- Distributed CollectiveMatch module uses the stride representations of GTI's channel identifiers to provide better performance for 
   comb-shape communicators
- Basic checks for thread level usage (MPI_Init_thread)

v1.2.0 (November 2012):
- Application crash handling to allow efficient MUST internal communication even if the target application crashs
- Distributed checks for lost message detection, point-to-point and collective type matching, and collective checking
-- The only check that is still only available as a centralized version is the actual deadlock detection
- Extended deadlock view provides details for non-blocking communications and for potential message mismatch situations
- Visualization of path expressions provide detailed insight into communication buffer overlap situations and for type mismatches
- Completely reworked mustrun command
- Numerous bug fixes and portability improvements

v1.1.0 (April 2012):
- New deadlock display (if a DOT installation is available, this comes with the graphviz package on most linux systems)
- Wait-for graphs now show message tags for blocked P2P messages
- Checkpoint & Restart functionality for deadlock detection
-- Adds higher accuracy and support for complex corner cases
-- Some corner cases may require imprecise decisions by MUST, this will be hinted upon in its output file
- Bugfix to support MPI_STATUS[ES]_IGNORE
- Various small bugfixes and portability improvements
- All remainders of the deprecated OTF tracing functionality were removed

v1.10 (November 2011):
- Initial MUST version