package Parser;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author Sriram
 */
import java.io.*;
import java.util.*;
import Data.*;

public class Parser {
    private boolean lines_ignored = false;
    private boolean fib_detected = false;
    private boolean type_mismatch_detected = false;

    public boolean isCollective(String funcname) {
        boolean value = false;
        if (funcname.equals("Barrier") || funcname.equals("Allreduce") || funcname.equals("Bcast") ||
            funcname.equals("Reduce") || funcname.equals("Reduce_scatter") || funcname.equals("Scatter") ||
            funcname.equals("Gather") || funcname.equals("Scatterv") || funcname.equals("Gatherv") ||
            funcname.equals("Allgather") || funcname.equals("Allgatherv") || funcname.equals("Alltoall") ||
            funcname.equals("Alltoallv") || funcname.equals("Scan") || funcname.equals("Comm_create") || funcname.equals("Cart_create") ||
            funcname.equals("Comm_dup") || funcname.equals("Comm_split") || funcname.equals("Comm_free") ||
            funcname.equals("AllReduce")) {
            value = true;
        }
        return value;
    }

    public boolean isp2p(String funcname) {
        boolean value = false;
        if (funcname.equals("Send") || funcname.equals("Ssend") || funcname.equals("Isend") ||
                funcname.equals("Irecv") || funcname.equals("Recv") || funcname.equals("Iprobe") ||
                funcname.equals("Probe") || funcname.equals("Rsend")) {
            value = true;
        }
        return value;
    }

    public void parse(String filename) {
        BufferedReader bReader = null;
        try {
            bReader = new BufferedReader(new FileReader(filename));
        } catch (FileNotFoundException e) {
            if (GlobalStructure.getInstance().Exception == null) {
                GlobalStructure.getInstance().Exception = "File Not Found";
            }
        }
        String sourcefile = new String();
        boolean newProcFlag = false;        
        int tP = 0;     // total process count
        try {
            String line = bReader.readLine();
            int iIndex = 0;
            int pIndex = 0;
            int intraCBcount = 0;
            while (line != null) {
                StringTokenizer sTokenizer = new StringTokenizer(line);
                if (sTokenizer.countTokens() == 1) {
                    String token = sTokenizer.nextToken();
                    
	        	    // for compatibility with newer ISP log files
		    /* == wfchiang begin == */
		    //Ganesh: 11/11/11: commented out System.out.println("wfchiang 1: " + token);
		    /*
                    if(token.equals("[FIB]") || token.equals("[TYPEMISMATCH]")) {
                        break;
                    }
		    */
		    //Ganesh: 11/11/11: Added this if {..} block
		    if(token.equals("[TYPEMISMATCH]")) {
			type_mismatch_detected = true;
                    }
		    //G:-- end of additions 11/11/11
		    if(token.equals("[FIB]")) {
			//Ganesh: 11/11/11: Added just this following line:
			fib_detected = true;
                        break;
                    }
		    /* == wfchiang end == */
                    
                    // for older versions of ISP
                    if(token.equals("DEADLOCK")) {
                            GlobalStructure.getInstance().deadlocked = true;                            
                            break;                        
                    }
		    /* == wfchiang begin == */
		    else if(token.equals("[TYPEMISMATCH]")); // do =nothing
		    /* == wfchiang end == */
                    else {
                        tP = new Integer(token).intValue();
                        GlobalStructure.getInstance().noProcess = tP;
                    }
                } 
                else {
                    Transition tTemp = new Transition();
                    while (sTokenizer.hasMoreTokens()) {
                    	// to handle older assert/leak from older versions of isp
		        	    String firstToken = sTokenizer.nextToken();
						if (firstToken.equals("ASSERT") || firstToken.equals("LEAK")) {
			                tTemp = null;
			                break;
			            }
				   /* == wfchiang begin ==*/
				   int interleaving_no = -1;
				   boolean ignore_this_line = false;
				   try {
				       //int interleaving_no = Integer.parseInt(firstToken);
				       interleaving_no = Integer.parseInt(firstToken);
				   } catch (Exception e) { 
				       ignore_this_line = true;
				   }
				   if (ignore_this_line) {
				       //Ganesh: 11/11/11: Added this line
				       lines_ignored = true;
				       //Ganesh: 11/11/11: commented out System.err.println("WARNING: Unexpect token in the log: " + firstToken);
				       //Ganesh: 11/11/11: commented out System.err.println("ISPUI will now ignore this whole line");
				       tTemp = null;
				       break;
				   }
				   /* == wfchiang end == */
                        tTemp.interleaving_no = interleaving_no;
                        if (iIndex < interleaving_no) {
                            iIndex = interleaving_no;
                            pIndex = 0;
                            newProcFlag = true;
                            GlobalStructure.getInstance().interleavings.add(new Interleavings(tP));
                            GlobalStructure.getInstance().interleavings.get(iIndex - 1).iNo = iIndex;
                        }

                        String token = sTokenizer.nextToken();
                        if (token.equals("DEADLOCK")) {
                            GlobalStructure.getInstance().deadlocked = true;
                            tTemp = null;
                            break;
                        }
                        int pID = Integer.parseInt(token);
                        tTemp.pID = pID;
                        if (pIndex < tTemp.pID) {
                            pIndex = tTemp.pID;
                            newProcFlag = true;
                        }
                        token = sTokenizer.nextToken();
                        if (token.equals("ASSERT") || token.equals("Leak") || token.equalsIgnoreCase("FILE:")) {
                            tTemp = null;
                            break;
                        }
                        int index = Integer.parseInt(token);
                        tTemp.index = index;
                        tTemp.orderID = Integer.parseInt(sTokenizer.nextToken()); // skip the order-id
                        tTemp.issueID = Integer.parseInt(sTokenizer.nextToken()); // skip the issue-id
                        String funcname = sTokenizer.nextToken();
                        tTemp.function = funcname;
                        if (isCollective(funcname)) {
                            tTemp.Communicator = sTokenizer.nextToken();
                        } else if (isp2p(funcname)) {
                            tTemp.src_or_dst = sTokenizer.nextToken();
                            tTemp.tag = sTokenizer.nextToken();
                            tTemp.Communicator = sTokenizer.nextToken();
                        }
                        String fbBegin = sTokenizer.nextToken();
                        if (fbBegin.equals("{")) {
                            while (true) {
                                token = sTokenizer.nextToken();
                                if (token.equals("}")) {
                                    break;
                                }
                                tTemp.intraCB.add(Integer.parseInt(token));
                                intraCBcount++;
                            }

                        }
                        fbBegin = sTokenizer.nextToken();
                        if (fbBegin.equals("{")) {
                            token = new String();
                            while (true) {
                                InterCBTemplate tempTuple = new InterCBTemplate();
                                token = sTokenizer.nextToken();
                                if (token.equals("}")) {
                                    break;
                                }
                                if (token.equals("[")) {
                                    String pId = sTokenizer.nextToken();
                                    String Index = sTokenizer.nextToken();
                                    tempTuple.pID = Integer.parseInt(pId);
                                    tempTuple.index = Integer.parseInt(Index);
                                    sTokenizer.nextToken();         // scans ]                                                
                                }
                                tTemp.interCB.add(tempTuple);
                            }
                        }

                        String match = sTokenizer.nextToken();
                        if (match.equals("Match:")) {
                            tTemp.match_pID = Integer.parseInt(sTokenizer.nextToken());
                            tTemp.match_index = Integer.parseInt(sTokenizer.nextToken());
                        }
                        String file = sTokenizer.nextToken();
                        if (file.equals("File:")) {
                            int fileNameLen = Integer.parseInt(sTokenizer.nextToken());
//                            tTemp.filename = "";
//                            while (sTokenizer.countTokens() > 1)
//                                tTemp.filename += sTokenizer.nextToken() + " ";
//                            tTemp.filename = tTemp.filename.trim();

                            tTemp.filename = sTokenizer.nextToken();
                            while (tTemp.filename.length() != fileNameLen) {
                                tTemp.filename += " " + sTokenizer.nextToken();
                            }
//                            if((! sourcefile.equals(tTemp.filename)) || (newProcFlag) ) {
//                                sourcefile = tTemp.filename;
//                                GlobalStructure.getInstance().addSourceFileName(sourcefile, iIndex, pID);
//                                newProcFlag = false;
//                            }
                            
                            tTemp.lineNo = Integer.parseInt(sTokenizer.nextToken());
                        }
                    }
                    if (tTemp != null) {
                        tTemp.CreateCell();
                        GlobalStructure.getInstance().interleavings.get(iIndex - 1).tList.get(pIndex).add(tTemp);
                    }
                }
                line = bReader.readLine();
            }
            GlobalStructure.getInstance().nInterleavings = iIndex;
            GlobalStructure.getInstance().initSystem();
	    //Ganesh: 11/11/11: Added these if {..} blocks
	    if (lines_ignored) {
		System.out.println("Some input lines were ignored because of log file format changes");
		lines_ignored = false;
	    }
	    if (fib_detected) {
		System.out.println("Functionally irrelevant barriers [FIB] were detected");
		fib_detected = false;
	    }
	    if (type_mismatch_detected) {
		System.out.println("Type mismatches [TYPEMISMATCH] were detected");
		type_mismatch_detected = false;
	    }
	    //G:-- end of additions 11/11/11
            System.out.println("Parsing Success : Firing up the UI");
        } catch (Exception e) {
            if (GlobalStructure.getInstance().Exception == null) {
                System.out.print(e.getMessage());
                GlobalStructure.getInstance().Exception = "Error in Parsing : Incorrect File";
            //System.out.print(e.getMessage());
            }
        }
    }
}

