/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : denynewconnectionspub.js
 Script version : 1.0
 Description    : This script denies new connections for a specified publishing point
 on the target machine. The server name should be specified. If it is not specified,
 the local machine should be the target server. The script gives the option to
 start specific publishing points on the server. If the "all" option is
 specified, all broadcast publishing points on the server should be paused.
 Command line parameters :
           [-s <Server1>] -p <pub1[,pub2,pubN|all]>
 where  -s represents target server, -p represents publishing point name(s).
 Example: denynewconnectionspub -s s4 -p pub4,pub5
 Returns  :
 1. Usage: denynewconnectionspub [-s <Server1>] -p <pub1[,pub2,pubN|all]>
 2. Server %server% is not a valid WMS Server
 3. Publishing Point %pub% etc is not a valid publishing point on server %server%
 OS Requirements       : Windows Server 2003 (all versions)
 Software requirements : WMS Server
 Scripting Engine      : Jscript
 ---------------------------------------------------------------------*/
WMS_PUBLISHING_POINT_TYPE_ON_DEMAND = 1;
WMS_PUBLISHING_POINT_TYPE_BROADCAST = 2;
WMS_PUBLISHING_POINT_TYPE_CACHE = 3;
WMS_PUBLISHING_POINT_TYPE_PROXY = 4;

var objServer = null;
var objPubPoint = null;
var dwWhichArg = 0;
var dwNumPubPoints = 0; 
var dwFlag = 0;
var szArgServer = "";
var szEachArgument = "";
var szTemp = "";
var dwEachPubIndex = 0;
var bFailed = false;
var bPubPointSpecified = false;

var PubPointArgumentList = new Array();
var objArgs = WScript.Arguments;

if(WScript.Arguments.length == 0)
{ DisplayUsage(); }

// Parse the command to seperate out the server name and publishing points.

while( dwWhichArg < WScript.Arguments.length )
{
    szEachArgument =  objArgs(dwWhichArg);
    if(szEachArgument.toLowerCase()== "-s")
    {
        dwWhichArg = dwWhichArg + 1;
        if(dwWhichArg >=WScript.Arguments.length)    //ex: denynewconnectionspub -p p1 -s
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg);
        if(szEachArgument.toLowerCase()== "-p")  //if next szEachArgument is -p, display usage : since syntax is wrong
        {
            DisplayUsage();
        }
        else
        {
            //accept only one server name
            if(szEachArgument.lastIndexOf(",")== -1)
            {
                szArgServer=szEachArgument;
            }
            else
            {
                DisplayUsage();
            }
        }
    }
    else if(szEachArgument.toLowerCase()== "-p")
    {
        bPubPointSpecified = 1;
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)    //ex: denynewconnectionspubpub -p p1 -s
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg);
        if(szEachArgument.toLowerCase()== "-s")  //if next szEachArgument is -s, display usage : since syntax is wrong
        {
            DisplayUsage();
        }
        else
        {
            //split with comma delimter
            PubPointArgumentList=szEachArgument.split(",");
        }
    }
    else  // if argument is not -p,-s  i.e. if it is an invalid argument
    {
        DisplayUsage();
    }
    dwWhichArg = dwWhichArg + 1;
}

if(!bPubPointSpecified)       //Ex:  denynewconnectionspub -s "s1"   and -p is part is missing.
{
    DisplayUsage();
}

// Connect to Server
//if server name is not mentioned, then rampdown Publishing points on localhost
if(szArgServer == "")
{
    szArgServer="LocalHost"
}
objServer = new ActiveXObject( "WMSServer.server", szArgServer );
Trace("\nDenying new connections on Publishing Points at "+ szArgServer );

//Ramp-down Publishing Points
dwFlag = 0;
dwEachPubIndex = 0;
dwNumPubPoints = objServer.PublishingPoints.Count;
if(PubPointArgumentList[0].toLowerCase() == "all")
{
    var i = 0;
    for( i = 0; i < dwNumPubPoints; i++ )
    {
        RampDownPubPoint( objServer, i );
    }
}
else
{
    while(dwEachPubIndex < PubPointArgumentList.length)
    {
        var szEachPubPointName = "";
        var i = 0;
        dwFlag = 0;
        objPubPoint = PubPointArgumentList[dwEachPubIndex];
        while( ( i < dwNumPubPoints ) && ( ! dwFlag ) )
        {
            szEachPubPointName = objServer.PublishingPoints.item( i ).name;
            if(objPubPoint.toLowerCase() == szEachPubPointName.toLowerCase())
            {
                RampDownPubPoint( objServer, i );
            }
            i = i + 1;
        }

        if(dwFlag==0)
        {
            szTemp = "Publishing Point "+objPubPoint+" does not exist ";
            Trace(szTemp);
        }
        dwEachPubIndex = dwEachPubIndex + 1;
    }
}   //end of else



function RampDownPubPoint( Server, i )
{
    objPubPoint = Server.PublishingPoints.item( i ).name;     //required for denynewconnectionspub -p all
    if((Server.PublishingPoints.item( i ).Type == WMS_PUBLISHING_POINT_TYPE_BROADCAST)||
        (Server.PublishingPoints.item( i ).Type == WMS_PUBLISHING_POINT_TYPE_ON_DEMAND) ||
        (Server.PublishingPoints.item( i ).Type == WMS_PUBLISHING_POINT_TYPE_CACHE) ||
        (Server.PublishingPoints.item( i ).Type == WMS_PUBLISHING_POINT_TYPE_PROXY) )
    {
        szTemp = "Denying new connections on " + objPubPoint;
        Server.PublishingPoints.item( i ). AllowClientsToConnect = 0
        Trace(szTemp) ;
        dwFlag= 1;
    }
    else
    {
        szTemp = objPubPoint+" is of unknown type : "+Server.PublishingPoints.item( i ).Type;
        Trace( szTemp );
        dwFlag = 2;
    }
}

function DisplayUsage()
{
    Trace("Usage: denynewconnectionspub[-s <Server1>] -p <pub1[,pub2,pubN|all]> ");
    Trace("Only one server name is accepted. ");
    Trace("If server name is not mentioned, local host is the default server ");
    WScript.Quit(1);
}

function Trace( Msg )
{
    WScript.Echo( Msg );
}
