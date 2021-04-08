# The MPI Bug Initiative

The MPI Bug Initiative is a collection of MPI codes that aims at assessing the status of MPI verification tools.



## Authors

The MBI was created by Mathieu Laurent, Martin Quinson and Emmanuelle Saillard.


## Feature and Errors Labels

All programs in the benchmark have a formated header with the following feature and error labels.


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


 Error Label |  Description
 ------------|--------------------
  deadlock | Non-occurence of something
  numstab | Numerical instability (wrong computation outcome)
  mpierr | The MPI runtime is aborting 
  resleak | Resource leaking
  datarace | Data race
  various | Various type of error




## List of Programs

TODO


## Tools Information

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
ISP | 69 | 70 | 32 | 20 | 8  | 0. 7753 | {172 | 0.6832 | 0.7277 | 0.7263 
 MUST | 83 | 79 | 10 | 15 | 12 | 0.8469 | 168 | 0.8925 | 0.8663 | 0.8691
PARCOACH | 18 | 80 | {8 | 90 | 3 | 0.1667 | 168 | 0.6923 |0.5000 | 0.287
McSimGrid | 77 | 76 | 16 | 17 | 13 | 0.8191 | 168 | 0.8280 | 0.8226 | 0.8235 

