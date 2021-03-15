GTI (Generic Tool Infrastructure)

GTI is an infrastructure for tools that operate in parallel systems. It 
provides a simple means of specifying and implementing a tool without
the need to develop extensive infrastructure.

Currently there is no single document with GTI documentation, rather
GTI based tools will provide install instructions for GTI.
The License file is located directly in this package, named LICENSE.txt

MUST v1.3.0 is a released GTI based tool and provides GTI      
installation instructions in its manual see http://tu-dresden.de/zih/must.

GTI uses CMake as a build system to enable compatibility to multiple
operating systems. See the CMake documentation for further details 
on how to build and install with CMake.

----------------------------
CHANGE-LOG
----------------------------
v1.4.0-rc1:
- New type of analysis ("automagic") that get mapped when all their inputs are available due to the presence of other analyses
- Improvements for admin-based installations
- Smaller improvements and bug fixes

v1.3.0:
- Increased levels of asynchrony in communication strategies
- New type of finalizing API calls allows tool triggered shutdown
- Optimizations for latency critical MPI communication modes
- Drastically increased performance for broadcasts
- A new communication strategy (CStratPRecv) that utilizes a communication protocol's ability to support partial receives
- Channel identifiers now support a "stride" representation to increase event aggregation success rates for comb-shaped MPI communicators
- Application pause mechanism allows a tool to suspend an application to finish its own event processing
-- This feature is designed to avoid OOM situations in heavy weight analysis tools
- Tuning of the channel selection heuristics that we use (FloodControl)
- MPI_Init_thread support

v1.2.0:
- Shared memory based communication mechanism (Linux message queues)
- Intra layer communication to handle cases that won't easily operate on a TBON
- Downwards broadcasting communication interfaces
- GTI internal and utility modules that may also be accessed by user modules:
-- Flood control module to avoid OOM issues at scale
-- Shutdown handling module (To incorporate shutdown requirements of intra and downwards communication)
- Extensions to the ModuleBase interface to reveal more insight on the TBON layout to modules
- Enhanced profiling mechanism to reveal imbalances and general overheads 
- Several bug fixes and portability improvements

v1.1.0:
- Extensions for tree overlay network event aggregations
- Frontend/Backend build extensions; All GTI tools are built with backend compilers and should be run on the backend
- GTI system-builder tools do not require an mpirun anymore
- Various portability enhancements
- Preparations for intra layer communication
- Removed deprecated OTF based tracing functionality
- Correction of std:: container initalization for static containers (Required for Cray systems)
- Smaller code optimizations

v1.0.1:
- Small fix in wrapper and receival generators