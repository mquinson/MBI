/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file MsgLoggerHtml.cpp
 *       @see MUST::MsgLoggerHtml.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "mustConfig.h"

#include "MsgLoggerHtml.h"

#include <string.h>

using namespace must;

mGET_INSTANCE_FUNCTION(MsgLoggerHtml)
mFREE_INSTANCE_FUNCTION(MsgLoggerHtml)
mPNMPI_REGISTRATIONPOINT_FUNCTION(MsgLoggerHtml)

#define TRAILERSTRING     "           </table></body></html>"
#define TRAILERSTRINGLEN  31

//=============================
// Constructor
//=============================
MsgLoggerHtml::MsgLoggerHtml (const char* instanceName)
    : gti::ModuleBase<MsgLoggerHtml, I_MessageLogger> (instanceName),
      myPIdModule (NULL),
      myLIdModule (NULL),
      myOut ("MUST_Output.html"),
      myPrintedTrailer (false),
      myLineEven (true),
      myLoggedWarnError (false)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //save sub modules
    myLIdModule = (I_LocationAnalysis*) subModInstances[0];
    myPIdModule = (I_ParallelIdAnalysis*) subModInstances[1];

    //Initialize the log file
    printHeader ();
}

//=============================
// Destructor
//=============================
MsgLoggerHtml::~MsgLoggerHtml (void)
{
    if (myLIdModule)
        destroySubModuleInstance ((I_Module*) myLIdModule);
    myLIdModule = NULL;

    if (myPIdModule)
        destroySubModuleInstance ((I_Module*) myPIdModule);
    myPIdModule = NULL;

    //Close the log file
    flush ();   // both together don't look to useful, but the first one gives additional information if no
    unflush (); // issues have been detected while the latter makes space for a 'big' final trailer.
    printTrailer(true);
    myOut.close();
}

//=============================
// log
//=============================
GTI_ANALYSIS_RETURN MsgLoggerHtml::log (
          int msgId,
          int hasLocation,
          uint64_t pId,
          uint64_t lId,
          int msgType,
          char *text,
          int textLen,
          int numReferences,
          uint64_t* refPIds,
          uint64_t* refLIds)
{
    if (!hasLocation)
        return logStrided (msgId, pId, lId, 0, 0, 0, msgType, text, textLen, numReferences, refPIds, refLIds);

    return logStrided (msgId, pId, lId, myPIdModule->getInfoForId(pId).rank, 1, 1, msgType, text, textLen, numReferences, refPIds, refLIds);
}

//=============================
// log
//=============================
GTI_ANALYSIS_RETURN MsgLoggerHtml::logStrided (
                    int msgId,
                    uint64_t pId,
                    uint64_t lId,
                    int startRank,
                    int stride,
                    int count,
                    int msgTypeTemp,
                    char *text,
                    int textLen,
                    int numReferences,
                    uint64_t* refPIds,
                    uint64_t* refLIds)
{
    MustMessageType msgType = (MustMessageType) msgTypeTemp;
    bool bWasTrailerPrinted = myPrintedTrailer;

    if (bWasTrailerPrinted)
        unflush();

    //select even or odd char
    char evenOrOdd = 'e';
    if (!myLineEven)
        evenOrOdd = 'o';

    //select even or odd char
    char infoWarnError = 'i';
    std::string msgTypeStr = "Information";
    if (msgType == MustWarningMessage)
    {
        infoWarnError = 'w';
        msgTypeStr = "Warning";
        myLoggedWarnError = true;
    }
    else
    if (msgType == MustErrorMessage)
    {
        infoWarnError = 'e';
        msgTypeStr = "Error";
        myLoggedWarnError = true;
    }

    //replace all '\n' characters with "<br>" in the text
    std::string::size_type p = 0;
    std::string tempText (text); //copy the text

    do {
        p = tempText.find ('\n', p);
        if (p == std::string::npos)
            break;
        tempText.replace (p,1,"<br>");
    }while (p != std::string::npos);

    // ELPTODO: Threadsafety?
    static int ctr = 0;

    //do the html output
    myOut
            << "<tr onclick=\"showdetail(this, 'detail" << ctr << "');\" onmouseover=\"flagrow(this);\" onmouseout=\"deflagrow(this);\">"
            << "<td class="<< infoWarnError << evenOrOdd << "1>";

    if (count > 0)
    {
        //CASE1: A single rank
        if (count == 1)
        {
            // Thread id only interest us if this concerns
            // a single rank
            ParallelInfo info = myPIdModule->getInfoForId(pId);
            if (info.threadid)
                myOut << startRank << "(" << info.threadid << ")";
            else
                myOut << startRank;
        }
        //CASE2: Continous ranks
        else if (stride == 1)
        {
            myOut << startRank << "-" << startRank + (count-1);
        }
        //CASE3: Strided ranks
        else
        {
            int last = startRank;
            for (int x = 0; x < count; x++)
            {
                if (last != startRank)
                    myOut << ", ";

                myOut << last;

                last += stride;

                if (x == 2 && count > 4)
                {
                    myOut << ", ..., " << startRank + (count - 1) * stride;
                    break;
                }
            }
        }
    }
    else
    {
        myOut << "&nbsp;";
    }

    myOut
            << "</td>"
            //<< "<td class="<< infoWarnError << evenOrOdd << "2>"
            //<< "&nbsp;" //TODO currently we have no thread information
            //<< "</td>"
            << "<td class="<< infoWarnError << evenOrOdd << "2>"
            << "<b>"
            << msgTypeStr
            << "</b>"
            << "</td>"
            << "<td class="<< infoWarnError << evenOrOdd << "3>"
            << "<div class=\"shortmsg\">"
            << tempText
            << "</div>"
            << "</td>"
            << "</tr>"
            << "<tr>"
            << "<td colspan=\"3\" id=\"detail"<< ctr++ << "\" style=\"visibility:hidden; display:none\">"
            << "<div>Details:</div>"
            << "<table>"
            << "<tr>"
            << "<td align=\"center\" bgcolor=\"#9999DD\"><b>Message</b></td>"
            << "<td align=\"left\" bgcolor=\"#7777BB\"><b>From</b></td>"
            << "<td align=\"left\" bgcolor=\"#9999DD\"><b>References</b></td>"
            << "</tr>"
            << "<tr>"
            << "<td class="<< infoWarnError << evenOrOdd << "3>"
            << tempText
            << "</td>"
            << "<td class="<< infoWarnError << evenOrOdd << "4>";
    if (count > 0)
    {
        //if (count > 1)
        //Even if count == 1, it may still yield from a representative
        myOut << "Representative location:<br>" << std::endl;

        printLocation (pId, lId);
    }
    else
    {
        myOut << "&nbsp;";
    }

    myOut
            << "</td>"
            << "<td class="<< infoWarnError << evenOrOdd << "5>";

    //References heading
    if (numReferences > 0)
        myOut << "References of a representative process:<br><br>" << std::endl;

    //Print extra references
    for (int i = 0; i < numReferences; i++)
    {
        if (i != 0)
            myOut << "<br><br>";

        myOut
            << "reference "<<(i+1)<<" rank "
            << myPIdModule->getInfoForId(refPIds[i]).rank
            <<  ": ";
        printLocation (refPIds[i], refLIds[i]);
    }
    myOut
        << "&nbsp;"
        << "</td>";

    //TODO Reference to MPI standard
  //  myOut
   //     << "<td class="<< infoWarnError << evenOrOdd << "6>"
    //    << "&nbsp;"
      //  << "</td>";

    //End the row
    myOut  << "</tr></table></td></tr>" << std::endl;

    //Flush if necessary
    if ((msgType == MustErrorMessage) || bWasTrailerPrinted)
        flush();

    //invert even/odd flag
    myLineEven = !myLineEven;

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// flush
//=============================
void MsgLoggerHtml::flush (void)
{
    if (!myLoggedWarnError)
    {
        //If we flush due to a new error we set myLoggedWarnError to true before we call flush, no no issue in that case
        char temp[] = "MUST detected no MPI usage errors nor any suspicious behavior during this application run.";
        log (
                MUST_MESSAGE_NO_ERROR,
                false,
                0,
                0,
                MustInformationMessage,
                temp,
                strlen (temp),
                0,
                NULL,
                NULL);
    }

    if (!myPrintedTrailer)
        printTrailer();
    myOut.flush();
}

//=============================
// unflush
//=============================
void MsgLoggerHtml::unflush (void)
{
    //  there is no effect on changing a flushed stream
    //  -  most browsers can handle missing final tags - should we print them just on destructor?
    //  -  or manipulate the filebuffer-object
    if (myPrintedTrailer)
    {
        myOut.rdbuf()->pubseekoff(- TRAILERSTRINGLEN, std::ios_base::end);
        myOut << "<!-- was flushed here -->" << std::endl;
        myPrintedTrailer = false;
    }
}

//=============================
// printTrailer
//=============================
void MsgLoggerHtml::printTrailer (bool finalNotes)
{
    if (finalNotes) {
        //read the system time
        char buf[128];

        struct tm *ptr;
        time_t tm;
        tm = time(NULL);
        ptr = localtime(&tm);
        strftime(buf ,128 , "%c.\n",ptr);
        myOut << "</table>" << "<br>" << "<b>MUST has completed successfully</b>, end date: " << buf
                << "</body></html>";
    } else
        myOut << TRAILERSTRING << std::endl;
    myPrintedTrailer = true;
}

//=============================
// printHeader
//=============================
void MsgLoggerHtml::printHeader (void)
{
    //read the system time
    char buf[128];

    struct tm *ptr;
    time_t tm;
    tm = time(NULL);
    ptr = localtime(&tm);
    strftime(buf ,128 , "%c.\n",ptr);

    //print the header
    myOut
            << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">" << std::endl
            << "<html>" << std::endl
            << "<head>" << std::endl
            << "<title>MUST Outputfile</title>" << std::endl
            << "<style type=\"text/css\">" << std::endl
            << "td,td,table {border:thin solid black}" << std::endl
            << "td.ie1{ background-color:#DDFFDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ie2{ background-color:#EEFFEE; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ie3{ background-color:#DDFFDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ie4{ background-color:#EEFFEE; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ie5{ background-color:#DDFFDD;}" << std::endl
            << "td.ie6{ background-color:#EEFFEE; text-align:left; vertical-align:middle;}" << std::endl
            << "td.ie7{ background-color:#DDFFDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.io1{ background-color:#CCEECC; text-align:center; vertical-align:middle;}" << std::endl
            << "td.io2{ background-color:#DDEEDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.io3{ background-color:#CCEECC; text-align:center; vertical-align:middle;}" << std::endl
            << "td.io4{ background-color:#DDEEDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.io5{ background-color:#CCEECC;}" << std::endl
            << "td.io6{ background-color:#DDEEDD; text-align:left; vertical-align:middle;}" << std::endl
            << "td.io7{ background-color:#CCEECC; text-align:center; vertical-align:middle;}" << std::endl
            << "td.we1{ background-color:#FFFFDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.we2{ background-color:#FFFFEE; text-align:center; vertical-align:middle;}" << std::endl
            << "td.we3{ background-color:#FFFFDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.we4{ background-color:#FFFFEE; text-align:center; vertical-align:middle;}" << std::endl
            << "td.we5{ background-color:#FFFFDD;}" << std::endl
            << "td.we6{ background-color:#FFFFEE; text-align:left; vertical-align:middle;}" << std::endl
            << "td.we7{ background-color:#FFFFDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.wo1{ background-color:#EEEECC; text-align:center; vertical-align:middle;}" << std::endl
            << "td.wo2{ background-color:#EEEEDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.wo3{ background-color:#EEEECC; text-align:center; vertical-align:middle;}" << std::endl
            << "td.wo4{ background-color:#EEEEDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.wo5{ background-color:#EEEECC;}" << std::endl
            << "td.wo6{ background-color:#EEEEDD; text-align:left; vertical-align:middle;}" << std::endl
            << "td.wo7{ background-color:#EEEECC; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ee1{ background-color:#FFDDDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ee2{ background-color:#FFEEEE; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ee3{ background-color:#FFDDDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ee4{ background-color:#FFEEEE; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ee5{ background-color:#FFDDDD;}" << std::endl
            << "td.ee6{ background-color:#FFEEEE; text-align:left; vertical-align:middle;}" << std::endl
            << "td.ee7{ background-color:#FFDDDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.eo1{ background-color:#EECCCC; text-align:center; vertical-align:middle;}" << std::endl
            << "td.eo2{ background-color:#EEDDDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.eo3{ background-color:#EECCCC; text-align:center; vertical-align:middle;}" << std::endl
            << "td.eo4{ background-color:#EEDDDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.eo5{ background-color:#EECCCC;}" << std::endl
            << "td.eo6{ background-color:#EEDDDD; text-align:left; vertical-align:middle;}" << std::endl
            << "td.eo7{ background-color:#EECCCC; text-align:center; vertical-align:middle;}" << std::endl
            << "div.shortmsg{ max-width:100%; text-overflow:ellipsis; display: inline-block; display: inline-block; white-space: nowrap; overflow:hidden; }" << std::endl
            << "div.shortmsg br { display: none; }" << std::endl
            << "</style>" << std::endl
            << "</head>" << std::endl
            << "<body>" << std::endl
            << "<p> <b>MUST Output</b>, starting date: "
            << buf
            << "</p>" << std::endl
            << "<script type=\"text/javascript\">" << std::endl
            << "function flagrow(obj)" << std::endl
            << "{" << std::endl
            << " for( var i = 0; i < obj.cells.length; i++ )" << std::endl
            << "  obj.cells[i].style.borderColor=\"#ff0000\";" << std::endl
            << "}" << std::endl
            << std::endl
            << "function deflagrow(obj)" << std::endl
            << "{" << std::endl
            << " for( var i = 0; i < obj.cells.length; i++ )" << std::endl
            << "  obj.cells[i].style.borderColor=\"\";" << std::endl
            << "}" << std::endl
            << std::endl
            << "function showdetail(me, name)" << std::endl
            << "{" << std::endl
            <<   "var obj = document.getElementById(name);" << std::endl
            <<   "if( obj )" << std::endl
            <<   "{" << std::endl
            <<    "var style = obj.style;" << std::endl
            <<    "var visible = String(style.visibility);" << std::endl
            <<    "if( visible == 'hidden' )" << std::endl
            <<    "{" << std::endl
            <<      "style.visibility = 'visible';" << std::endl
            <<      "style.display=\"\";" << std::endl
            <<    "}" << std::endl
            <<    "else" << std::endl
            <<    "{"  << std::endl
            <<    "style.visibility = 'hidden';" << std::endl
            <<    "style.display=\"none\";"  << std::endl
            <<    "}"  << std::endl
            <<   "}" << std::endl
            <<  "}" << std::endl
            <<  "</script>" << std::endl
            << "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"table-layout: fixed; width: 100%\">" << std::endl
            << "<tr>" << std::endl
            << "<td align=\"center\" bgcolor=\"#9999DD\" width=\"10%\"><b>Rank(s)</b></td>" << std::endl
            << "<td align=\"center\" bgcolor=\"#7777BB\" width=\"10%\"><b>Type</b></td>" << std::endl
            << "<td align=\"center\" bgcolor=\"#9999DD\"><b>Message</b></td>" << std::endl
            << "</tr>" << std::endl;


            /*<< "<table border=\"0\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">" << std::endl
            << "<tr>" << std::endl
            << "<td align=\"center\" bgcolor=\"#9999DD\">"
            << "<b>Rank(s)</b>"
            << "</td>" << std::endl
            //<< "<td align=\"center\" bgcolor=\"#7777BB\">"
            //<< "<b>Thread</b>"
            //<< "</td>" << std::endl
            << "<td align=\"center\" bgcolor=\"#7777BB\">"
            << "<b>Type</b>"
            << "</td>" << std::endl
            << "<td align=\"center\" bgcolor=\"#9999DD\">"
            << "<b>Message</b>"
            << "</td>" << std::endl
            << "<td align=\"left\" bgcolor=\"#7777BB\">"
            << "<b>From</b>"
            << "</td>" << std::endl
            << "<td align=\"left\" bgcolor=\"#9999DD\">"
            << "<b>References</b>"
            << "</td>" << std::endl
            //<< "<td align=\"center\" bgcolor=\"#9999DD\">"
            //<< "<b>MPI-Standard Reference</b>"
            //<< "</td>" << std::endl
            << "</tr>" << std::endl;*/
}

//=============================
// printLocation
//=============================
void MsgLoggerHtml::printLocation (MustParallelId pId, MustLocationId lId)
{
#ifndef USE_CALLPATH
        myOut << myLIdModule->toString(pId, lId);
        printOccurenceCount(myOut, lId);
#else
        LocationInfo &ref = myLIdModule->getInfoForId (pId, lId);

        myOut << "<b>" << ref.callName << "</b>";
        printOccurenceCount(myOut, lId);
        myOut << " called from: <br>" << std::endl;

        std::list<MustStackLevelInfo>::iterator stackIter;
        int i = 0;
        for (stackIter = ref.stack.begin(); stackIter != ref.stack.end(); stackIter++, i++)
        {
            if (i != 0)
                myOut << "<br>";
            myOut << "#" << i << "  " << stackIter->symName << "@" << stackIter->fileModule << ":" << stackIter->lineOffset << std::endl;
        }
#endif
}

//=============================
// printOccurenceCount
//=============================
void MsgLoggerHtml::printOccurenceCount (std::ostream& out, MustLocationId lId)
{
    out << " ("<< myLIdModule->getOccurenceCount(lId);

    if (myLIdModule->getOccurenceCount(lId) == 1)
        out << "st";
    else if (myLIdModule->getOccurenceCount(lId) == 2)
        out << "nd";
    else if (myLIdModule->getOccurenceCount(lId) == 3)
        out << "rd";
    else
        out << "th";

    out <<" occurrence)";
}

/*EOF*/
