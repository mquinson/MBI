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
We define 12 feature labels. Each label is either set to Correct: correct usage of the feature, Incorrect: Incorrect usage of the feature or
Lacking: the feature is missing.


MPI Feature Label | Description 
 -----------------|--------------------
 P2P | Use of blocking point-to-point communication  
 iP2P  | Use of nonblocking point-to-point communication 
 PERS | Use of persistent communication 
 COLL  | Use of blocking collective communication  
 iCOLL | Use of nonblocking collective communication 
 RMA   | Use of remote memory access (RMA)  
 PROB  | Explicit use of MPI_Probe     
 OP | Use of operators 
 COM | Use of multiple communicators
 GRP | Use of groups 
 DATA | Use of datatypes 

Nondeterminism in the semantic of MPI primitives makes some errors difficult to predict and reproduce. As a results we popose to
set each error label either to never (the error never occurs), always (the error always occurs) or transient (the error sometimes occurs).



 Error Label |  Description
 ------------|--------------------
  deadlock | Non-occurence of something
  numstab | Numerical instability (wrong computation outcome)
  mpierr | The MPI runtime is aborting 
  resleak | Resource leaking
  datarace | Data race
  various | Various type of error




## List of Programs

Our benchmark contains 194 programs including 188 micro codes (with less than 100 lines of code) and 6 medium codes (with more than 100 lines of code). 112 programs have errors and 82 are known to be error-free.



## Tools Information

We use the MBI to compare Aislinn, CIVL, ISP, MUST, PARCOACH and McSimGrid.


Tool | Version | Compiler 
-----|---------|---------
Aislinn | v3.12 | GCC 7.4.0
CIVL | v1.20 | used with JDK-14
ISP | 0.3.1 | GCC 10.2.0
MUST | v1.6 | GCC 10.2.0
PARCOACH | v1 | LLVM 9
McSimGrid | v3.27 |  GCC 10.2.0


## Latest Tools Evaluation Results

 
Tool   	| TP | TN | FP | FN | Error | Recall | Specificity | Precision | Accurracy | F1 Score  
--------|----|----|----|----|-------|--------|-------------|-----------|-----------|---------
Aislinn | 64 | 64 | 16 | 20 | 35 | 0.7619 | 144 | 0.8000 | 0.7805 | 0.7805
CIVL | 13 | 18 | 28 | {1 | 139 | 0.9286 | 64 | 0.3171 | 0.5167 | 0.4727
ISP | 69 | 70 | 32 | 20 | 8  | 0. 7753 | 172 | 0.6832 | 0.7277 | 0.7263 
 MUST | 83 | 79 | 10 | 15 | 12 | 0.8469 | 168 | 0.8925 | 0.8663 | 0.8691
PARCOACH | 18 | 80 | {8 | 90 | 3 | 0.1667 | 168 | 0.6923 |0.5000 | 0.287
McSimGrid | 77 | 76 | 16 | 17 | 13 | 0.8191 | 168 | 0.8280 | 0.8226 | 0.8235 


Metrics used: 
- Recall: TP / ( TP + FN) 
- Specificity: TN / ( TN + FP)
- Precision: TP / ( TP + FP)
- Accuracy: (TP+TN) / (TP + FP + TN+ FN)
- F1 Score: 2 * (P * R) / (P + R)

