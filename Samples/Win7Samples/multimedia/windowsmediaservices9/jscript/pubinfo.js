/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script pubinfo.js : Display information for specific publishing point (pubinfo)  
 Script version : 1.0 
 Description    :The script lists information for a specific publshing point. 
 The following information should be presented. 
 Name, Path, Status, Current # of dwNumConnectedClients, current bandwidth usage. 
 Command line parameters : 
           [-s <Server1>] -p <Pub1>   
 where  -s represents target server, -p represents publishing point(s).
 Example : pubinfo -p pub1 
 Returns  : 
 1. Usage: pubinfo [-s <server1>] -p <pub1>  
 2. Server %server% is not a valid WMS Server
 3. Publishing Point %pub% etc is not a valid publishing point on server %server%
 OS Requirements       : Windows Server 2003 (all versions)
 Software requirements : WMS Server 
 Scripting Engine      : Jscript
 ---------------------------------------------------------------------*/
WMS_PUBLISHING_POINT_TYPE_ON_DEMAND      = 1;
WMS_PUBLISHING_POINT_TYPE_BROADCAST      = 2;
WMS_PUBLISHING_POINT_TYPE_CACHE          = 3;
WMS_PUBLISHING_POINT_TYPE_PROXY          = 4;

var objServer = null;
var objPubPoint = null;
var dwWhichArg = 0;
var dwNumPubPoints = 0;
var dwOption = 0;
var szArgOpt = null;
var szArgServer = "";
var szArgPubPoint = "";
var szEachArgument = "";
var szTemp = "";
var bCheckedArgP = false;

var objArgs = WScript.Arguments;
var dwNumArgs = WScript.Arguments.length;

if(dwNumArgs == 0) 
{
    DisplayUsage();
}

// Parse the command to seperate out the server and publishing points. 

while( dwWhichArg < dwNumArgs )
{ 
    szEachArgument = objArgs(dwWhichArg);  
    if( szEachArgument.toLowerCase()== "-s" )
    {   
        dwWhichArg = dwWhichArg + 1; 
        if( dwWhichArg >=dwNumArgs )    //ex: pubinfo -p p1 -s
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg);  
        if( szEachArgument.toLowerCase()== "-p" )  //if next szEachArgument is -p , display usage : since syntax is wrong
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one server argument 
            if( szEachArgument.lastIndexOf(",")== -1 )
            {
                szArgServer = szEachArgument;
            } 
            else 
            {
                DisplayUsage();
            } 
        }
    }                       
    else if( szEachArgument.toLowerCase()== "-p" )
    {            
        bCheckedArgP = true;
        dwWhichArg = dwWhichArg + 1;

        if( dwWhichArg >= dwNumArgs )    //ex: pubinfo -s s1 -p
        {
            DisplayUsage();
        }

        szEachArgument = objArgs( dwWhichArg );
        if(szEachArgument.toLowerCase()== "-s")  //if next szEachArgument is -s, display usage : since syntax is wrong
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one publishing point argument 
            if(szEachArgument.lastIndexOf(",")== -1)
            {
                szArgPubPoint=szEachArgument;
            } 
            else 
            {
                DisplayUsage();
            } 
        }
    }  
    else  // if argument is not -p,-t  i.e. if it is an invalid argument    
    {
        DisplayUsage();
    }
    dwWhichArg = dwWhichArg + 1; 
}

if( ! bCheckedArgP )       //Ex:  pubinfo -s "s1"   and -p  part is missing. 
{
    DisplayUsage();
}   
 
// Connect to Server  
//if server argument is not mentioned, then consider localhost as the server.
if(szArgServer == "")
{
    szArgServer="LocalHost"
}

var bFailed = false;

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
    try
    {
        objPubPoint = objServer.PublishingPoints.item(szArgPubPoint);
    }
    catch(e)
    {
        // This function is called if the Publishing Point is not found
        szTemp = "Publishing Point "+szArgPubPoint+" does not exist ";
        Trace(szTemp); 
        WScript.Quit(1);
    }

    GetPubPointInfo( objPubPoint )
}


//This function gets information about the publishing point.  
function GetPubPointInfo(objPubPoint)
{ 
    var szPublishingPointStatus = "";
    var dwPubPointStatus = 0;
    var dwNumConnectedClients = 0;
    var lPubPointBandwidth = -1;
    
    var szPubPointName = objPubPoint.Name;      
    var szPubPointPath = ""
    var enumPubPointType =  objPubPoint.Type;
    
    switch( enumPubPointType )
    {
    case WMS_PUBLISHING_POINT_TYPE_ON_DEMAND :
        szPubPointPath = objPubPoint.Path;
        szPublishingPointType = "On-Demand";
        if( true == objPubPoint.AllowClientsToConnect )
        {
            szPublishingPointStatus = "Allowing client connections";
        }
        else
        {
            szPublishingPointStatus = "Denying client connections";
        }
        szPubPointPath = objPubPoint.Path;
        break;
                    
    case WMS_PUBLISHING_POINT_TYPE_BROADCAST :
        szPubPointPath = objPubPoint.Path;
        szPublishingPointType = "Broadcast";
        dwStateFlags = objPubPoint.BroadcastStatus;
        szPubPointPath = objPubPoint.Path;
        szPublishingPointStatus = PPBcastStatusToString( dwStateFlags );
        if( true == objPubPoint.AllowClientsToConnect )
        {
            szPublishingPointStatus = szPublishingPointStatus + ", Allowing client connections";
        }
        else
        {
            szPublishingPointStatus = szPublishingPointStatus + ", Denying client connections";
        }
        break;
        
    case WMS_PUBLISHING_POINT_TYPE_CACHE :
        szPubPointPath = "N/A";
        szPublishingPointType = "Cache";
        if( true == objPubPoint.AllowClientsToConnect )
        {
            szPublishingPointStatus = "Allowing connections";
        }
        else
        {
            szPublishingPointStatus = "Denying connections";
        }
        break;
        
    case WMS_PUBLISHING_POINT_TYPE_PROXY : 
        szPubPointPath = "N/A";
        szPublishingPointType = "Proxy";
        dwStateFlags = objPubPoint.Status;
        szPublishingPointStatus = PPBcastStatusToString( dwStateFlags );
        if( true == objPubPoint.AllowClientsToConnect )
        {
            szPublishingPointStatus = szPublishingPointStatus + ", Allowing connections";
        }
        else
        {
            szPublishingPointStatus = szPublishingPointStatus + ", Denying connections";
        }
        break;
        
    default:
        szPublishingPointType = "Unknown";
        break;  
    }             

    dwNumConnectedClients = objPubPoint.CurrentCounters.ConnectedPlayers;
    lPubPointBandwidth = objPubPoint.CurrentCounters.PlayerAllocatedBandwidth;
    lPubPointBandwidth = lPubPointBandwidth + objPubPoint.CurrentCounters.OutgoingDistributionAllocatedBandwidth;
    
    szTemp = "Name : " + szPubPointName + " \n";
    if( 0 < szPubPointPath.length )
    {
        szTemp = szTemp + "Path : " + szPubPointPath + " \n";
    }
    szTemp = szTemp + "Type : " + szPublishingPointType + " \n";
    szTemp = szTemp + "Status : " + szPublishingPointStatus + " \n";
    szTemp = szTemp + "Current # of clients : " + dwNumConnectedClients + " \n";
    szTemp = szTemp + "Current Bandwidth usage : " + lPubPointBandwidth + " ";
    Trace(szTemp);       
 }

 
function PPBcastStatusToString( dwStateFlags )
{
    if( 8 & dwStateFlags )
    {
        return( "Change in Progress" );
    }
    
    switch( dwStateFlags )
    {
        case 0 : return( "Stopped" ); break;
        case 1 : return( "Started without data" ); break;
        case 2 : return( "Started" ); break;
        case 3 : return( "Started without data and archiving" ); break;
        case 4 : return( "Archiving" ); break;
        case 6 : return( "Started and Archiving" ); break;
        case 8 : return( "Change in Progress" ); break;
        default : return ( "Unknown State" ); break;
    }
}         

function DisplayUsage()
{ 
    Trace("Usage: pubinfo [-s <server1>] -p <pub1> ");  
    Trace("Only one server name is accepted. ");
    Trace("If server name is not mentioned, local host is the default server ");
    WScript.Quit(1);
}

function Trace(Msg)
{
    WScript.Echo(Msg);
}
