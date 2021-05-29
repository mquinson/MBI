# The MPI Bugs Initiative

The MPI Bugs Initiative is a collection of MPI codes that aims at assessing the status of MPI verification tools.


## Quick Start

Docker is used to facilitate the benchmark modifications and the installation of the tools we have selected.
You can create a docker image containing all dependencies needed to evaluate the tools using the provided Dockerfile.
The following commands will create and run the image:
```bash
docker build -f Dockerfile -t mbi:latest .
docker run -it mbi bash 
```

Once inside the docker, you will the scripts you need under /MBI/scripts. Two scripts are provided for each tool. /MBI/scripts/{tool}-build can be used to build and install the selected tool, while /MBI/scripts/{tool}-run will run that tool on all MBI code sample. The result can then be explored under /MBI/log/{tool}-{date} (a new such directory is created each time you launch a {tool}-run script)


## Feature and Errors Labels

All programs in the benchmark have a formated header with the following feature and error labels.
We define 7 feature labels. Each label is either set to Yes: use of the feature, or
Lacking: the feature is missing.


MPI Feature Label | Description 
 -----------------|--------------------
 P2P:base calls | Use of blocking point-to-point communication  
 P2P:nonblocking  | Use of nonblocking point-to-point communication 
 P2P:persistent | Use of persistent communication 
 COLL:base calls  | Use of blocking collective communication  
 COLL:nonblocking | Use of nonblocking collective communication 
 COLL:tools| Use of resource function (e.g., communicators, datatypes) 
 RMA   | Use of remote memory access (RMA)  



Nondeterminism in the semantic of MPI primitives makes some errors difficult to predict and reproduce. As a results we popose to make multiple tests out of some codes.



 Error Label |  Description
 ------------|--------------------
 Invalid Parameter | Invalid parameter in a MPI function (e.g., invalid root, communicator)
 Resource Leak | Resource leaking (e.g., communicator not freed)
 Request Lifecycle | Missing wait or start function
 Local Concurrency | Data race in a process
 Message Race | Non-determinism execution, caused by the use of ANY_SRC
 Parameter Matching | Wrong matching of parameters
 Call Matching | Call mismatch
 Global Concurrency | Data race resulting from multiplue processes
 Buffering Hazard | Error that depends on the buffering mode



## List of Programs

Our benchmark contains 708 programs.
471 programs have errors and 237 are known to be error-free.



## Tools Information

We use the MBI to compare Aislinn, CIVL, ISP, MUST, PARCOACH and McSimGrid.


Tool | Version | Compiler 
-----|---------|---------
Aislinn | v3.12 | GCC 7.4.0
CIVL | v1.20 | used with JDK-14
ISP | 0.3.1 | GCC 10.2.0
MUST | v1.7 | GCC 10.2.0
PARCOACH | v1 | LLVM 9
McSimGrid | v3.27 |  GCC 10.2.0


## Latest Tools Evaluation Results

 
Tool   	| TP | TN | FP | FN | Error | Recall | Specificity | Precision | Accurracy | F1 Score  
--------|----|----|----|----|-------|--------|-------------|-----------|-----------|---------
Aislinn | 64 | 64 | 16 | 20 | 35 | 0.7619 | 0.8000 | 0.8000 | 0.7805 | 0.7805
CIVL | 13 | 18 | 28 | 1 | 139 | 0.9286 | 0.3913 | 0.3171 | 0.5167 | 0.4727
ISP | 69 | 70 | 32 | 20 | 8  | 0. 7753 | 0.6863 | 0.6832 | 0.7277 | 0.7263 
 MUST | 83 | 79 | 10 | 15 | 12 | 0.8469 | 0.8876 | 0.8925 | 0.8663 | 0.8691
PARCOACH | 18 | 80 | 8 | 90 | 3 | 0.1667 | 0.9091 | 0.6923 |0.5000 | 0.287
McSimGrid | 77 | 76 | 16 | 17 | 13 | 0.8191 | 0.8261 | 0.8280 | 0.8226 | 0.8235 


Metrics used: 
- Recall: TP / ( TP + FN) 
- Specificity: TN / ( TN + FP)
- Precision: TP / ( TP + FP)
- Accuracy: (TP+TN) / (TP + FP + TN+ FN)
- F1 Score: 2 * (P * R) / (P + R)
- Robustness: tools ability to compile and execute codes 
- API Coverage: features coverage
