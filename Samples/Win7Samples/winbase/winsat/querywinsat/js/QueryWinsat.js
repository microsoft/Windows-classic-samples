//-----------------------------------------------------------------------
// This file is part of the Windows SDK Code Samples.
// 
// Copyright (C) Microsoft Corporation.  All rights reserved.
// 
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation.  See these other
// materials for detailed information regarding Microsoft code samples.
// 
// THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//-----------------------------------------------------------------------

var queryWinsat = new ActiveXObject("QueryWinsat");
var queryAllWinsat = new ActiveXObject("QueryAllWinsat");

var node;
var resultList = null;
var args = WScript.Arguments;

if (args.length == 0) 
{
    PrintUsageAndQuit();
}

var cmd = args(0);
var remainingCmdLine = "";
for (i = 1; i < args.length; i++) 
{
    remainingCmdLine = remainingCmdLine + " " + args(i);
}

if ( cmd == "query" ) 
{
    var assessmentInfo = queryWinsat.Info;
    var level = assessmentInfo.SystemRating;
    var stateDesc = assessmentInfo.RatingStateDesc;
    var dateTime = assessmentInfo.AssessmentDateTime;
    var state = assessmentInfo.AssessmentState;

    WScript.Echo("Rating: " + level);
    WScript.Echo("Desc: " + stateDesc);
    WScript.Echo("Date/Time: " + dateTime);
    WScript.Echo("State: " + state);

    var score, title, description;
    for (i = 0; i < 5; i++) 
    {
        var info = assessmentInfo.GetAssessmentInfo( i );  
        WScript.Echo("-------------------");
        WScript.Echo("" + info.Description + " " + info.Title);
        WScript.Echo("Score: " + info.Score);
        WScript.Echo("");
    }
} 
else if (cmd == "queryxml") 
{
      resultList = queryWinsat.XML(remainingCmdLine);
      PrintNodeList(resultList);
} 
else if (cmd == "queryallxml") 
{
      resultList = queryAllWinsat.AllXML(remainingCmdLine);
      PrintNodeList(resultList);
} 
else 
{
      PrintUsageAndQuit();
}

function PrintUsageAndQuit()
{
    WScript.Echo("Usage: QueryWinsat.js (query | queryxml| queryallxml) [xpath]");
    WScript.Quit(1);  
}

function PrintNodeList(nodeList)
{
    for (var i = 0; i < nodeList.length; i++) 
    {
        node = nodeList.nextNode();
        WScript.Echo(node.xml); 
    } 
}