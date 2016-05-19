/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : Add Publishing Point (addpub)
 Script version : 1.0
 Description    : This script adds a publishing point to the target
                  server.
 Command line parameters :
           [-s <Server1>] -n <pub1> -t <od|bc> -p <path>
 -s represents target server, -n represents publishing point name,
 -p represents path, -t represent type of publishing point.
 There are 4 Publishing point types.
 od = On-demand, bc = broadcast
 Example : addpub -s server1 -n odpub1 -t od -p c:/wsh1
 Returns  :
 1. Usage: addpub [-s <Server1>] -n <pub1> -t <od|bc> -p <path>
 2. Server %server% is not a valid WMS Server
 3.  %type% is not a valid publishing point type
 OS Requirements       :  Windows Server 2003 (all versions)
 Software requirements :  WMS Server
 Scripting Engine      : Jscript
 ---------------------------------------------------------------------*/
WMS_PUBLISHING_POINT_TYPE_ON_DEMAND = 1;
WMS_PUBLISHING_POINT_TYPE_BROADCAST = 2;

var objServer = null;
var dwWhichArg = 0;
var szEachArg = "";
var szTemp = "";
var szArgServer = "";
var szArgPubPoint = "";
var szArgPubPointType = "";
var szArgPubPointPath = "";
var bCheckName = false;
var bCheckPath = false;
var bCheckType = false;

var objArgs = WScript.Arguments;

if( 0 == WScript.Arguments.length )
{
    DisplayUsage(); 
}

// Parse the command to seperate out the server name and publishing points.

while( dwWhichArg < WScript.Arguments.length )
{
    szEachArg = objArgs( dwWhichArg );
    if( "-s" == szEachArg.toLowerCase() )
    {
        dwWhichArg = dwWhichArg + 1;
        if(dwWhichArg >= WScript.Arguments.length )    //ex: addpub ... -n p1 -s
        {
            DisplayUsage();
        }
        szEachArg = objArgs( dwWhichArg );
        if((szEachArg.toLowerCase()== "-n") || (szEachArg.toLowerCase()== "-p") || (szEachArg.toLowerCase()== "-t"))   //if next szEachArg is -p,-t,-n display usage : since syntax is wrong
        {
            DisplayUsage();
        }
        else
        {
            //accept only one server name
            if(szEachArg.lastIndexOf(",")== -1)
            {
                szArgServer = szEachArg;
            }
            else
            {
                DisplayUsage();
            }
        }
    }
    else if(szEachArg.toLowerCase()== "-n")
    {
        bCheckName = true;
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)    //ex: addpub ... -s s1 -p
        {
            DisplayUsage();
        }
        szEachArg = objArgs(dwWhichArg);
        if((szEachArg.toLowerCase()== "-p") || (szEachArg.toLowerCase()== "-t") || (szEachArg.toLowerCase()== "-s"))  //if next szEachArg is -s,-t,-p display usage : since syntax is wrong
        {
              DisplayUsage();
        }
        else
        {
            //accept only one publishing point name
            if(szEachArg.lastIndexOf(",")== -1)
            {
                szArgPubPoint=szEachArg;
            }
            else
            {
                DisplayUsage();
            }
        }
    }
    else if(szEachArg.toLowerCase()== "-p")
    {
        bCheckPath = true;
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)    //ex: addpub ... -s s1 -p
        {
            DisplayUsage();
        }
        szEachArg = objArgs(dwWhichArg);
        if((szEachArg.toLowerCase()== "-t") || (szEachArg.toLowerCase()== "-s") || (szEachArg.toLowerCase()== "-n"))  //display usage : since syntax is wrong
        {
            DisplayUsage();
        }
        else
        {
            //accept only one path
            if(szEachArg.lastIndexOf(",")== -1)
            {
                szArgPubPointPath=szEachArg;
            }
            else
            {
                DisplayUsage();
            }
        }
    }
    else if(szEachArg.toLowerCase()== "-t")
    {
        bCheckType = true;
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)
        {
            DisplayUsage();
        }
        szEachArg = objArgs(dwWhichArg);
        if((szEachArg.toLowerCase()== "-p") || (szEachArg.toLowerCase()== "-n") || (szEachArg.toLowerCase()== "-s"))  //display usage, since syntax is wrong
        {
            DisplayUsage();
        }
        else
        {
            //accept only one type
            if(szEachArg.lastIndexOf(",")== -1)
            {
                szArgPubPointType=szEachArg;
            }
            else
            {
                DisplayUsage();
            }
        }
    }
    else  // if argument is not -p,-n,-t,-s  i.e. if it is an invalid argument
    {
        DisplayUsage();
    }

    dwWhichArg = dwWhichArg + 1;
}

if( (!bCheckName) || (!bCheckPath) || (!bCheckType) )       //Ex:  compulsory part -p,-t or -n is missing.
{
    DisplayUsage();
}

// Connect to Server
//if server name is not mentioned, then start Publishing points on localhost
if( "" == szArgServer )
{
    szArgServer = "LocalHost";
}

var bFailed;
bFailed = false;

try
{
    objServer = new ActiveXObject( "WMSServer.server", szArgServer );
}
catch(e)
{
    bFailed = true;
    szTemp = "Server '" + szArgServer + "' is not a valid WMS Server \n";
    Trace( szTemp );
}

if( !bFailed )
{
    Trace("\nAdding Publishing Points at "+ szArgServer );

    //Add Publishing Point
    AddPublishingPoint();
}


// This function checks if the publishing point name and type are valid, and then
// adds it to server.
function AddPublishingPoint()
{
    var bFailed, nType;
    bFailed = false;

    //check if the type is a valid type
    switch(szArgPubPointType)
    {
        case 'od' :
            nType = WMS_PUBLISHING_POINT_TYPE_ON_DEMAND;
            break;
        case 'bc' :
            nType = WMS_PUBLISHING_POINT_TYPE_BROADCAST;
            break;
        default :
        {
            bFailed = true;
            szTemp = "Adding Publishing Point '" + szArgPubPoint + "' failed: Unknown type\n";
            break;
        }
    }

    if( !bFailed )
    {
        try
        {
            var objPubPoint = objServer.PublishingPoints.Add( szArgPubPoint, nType, szArgPubPointPath );
            szTemp = "Added " + szArgPubPoint;
        }
        catch(e)
        {
            var errorcode = e.number >>> 0;
            szTemp = "Error Code 0x" + errorcode.toString(16) + ": " + e.description;
        }
    }

    Trace( szTemp );
}

function DisplayUsage()
{
    Trace( "Usage: addpub [-s <Server1>] -n <pub1> -t <od|bc> -p <path>" );
    WScript.Quit(1);
}

function Trace(Msg)
{
    WScript.Echo(Msg);
}

