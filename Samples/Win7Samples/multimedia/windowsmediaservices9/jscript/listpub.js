/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : List On Demand, Broadcast, Cache, Proxy or All Publishing Points (listpub)  
 Script version : 1.0 
 Description    :This script lists publishing points on the target machine.
 The following  information should be presented for on demand and 
 broadcst publishing points. 
 Command line parameters : 
           [-s <Server1>] -t <od|bc|cache|proxy|all>
 where  -s represents target server, -t represents type of publishing point. 
 Example :  listpub -t all    
 Returns  : 
 1. Usage: listpub [-s <Server1>] -t <od|bc|cache|proxy|all>
 2. Server %server% is not a valid WMS Server
 3. Publishing Point %pub% etc is not a valid publishing point on server %server%
 OS Requirements       : Windows Server 2003 (all versions)
 Software requirements :  WMS Server 
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
var szEachArgument = "";
var szTemp = "";
var bCheckedArgT = false;

var objArgs = WScript.Arguments;
var dwNumArgs = WScript.Arguments.length;

if( 0 == dwNumArgs ) 
{
    DisplayUsage();
}

// Parse the command to seperate out the server name and publishing points. 
while( dwWhichArg < dwNumArgs )
{ 
    szEachArgument = objArgs( dwWhichArg );
    if(szEachArgument.toLowerCase()== "-s")
    {   
        dwWhichArg = dwWhichArg + 1; 
        if( dwWhichArg >= dwNumArgs )    //ex: listpub -t p1 -s
        {
            DisplayUsage();
        } 
        szEachArgument = objArgs( dwWhichArg );
        if( szEachArgument.toLowerCase()== "-t" )  //if next szEachArgument is -t , display usage : since syntax is wrong
        {
            DisplayUsage();
        }
        else
        {
            //accept only one server name 
            if( szEachArgument.lastIndexOf(",") == -1 )
            {
                szArgServer = szEachArgument;
            } 
            else 
            {
                DisplayUsage();
            } 
        }
    }                       
    else if( szEachArgument.toLowerCase()== "-t" )
    {            
        bCheckedArgT = true; 
        dwWhichArg = dwWhichArg + 1;

        if( dwWhichArg >= dwNumArgs )    //ex: listpub -s s1 -t
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
            //accept only one option
            if(szEachArgument.lastIndexOf(",")== -1)
            {
                szArgOpt = szEachArgument;
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

if(!bCheckedArgT)       //Ex:  listpub -s "s1"   and -t  part is missing. 
{
    DisplayUsage();
}   
 

// Connect to Server  
//if server name is not mentioned, then start Publishing points on localhost  
if( szArgServer == "")
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
    Trace("Listing Publishing Points at "+szArgServer);
    GetPublishingPoints( szArgOpt.toLowerCase() ); 
}

function GetPublishingPoints( Option )
{  
    var szPublishingPointType = "";
    var szPubPointPath = "";
    var dwPubPointType = 0;
    var szPubName = "";
    var szPublishingPointStatus = "";
    var szPubClients = "";
    var szPubBWdth = "";
    
    dwNumPubPoints = objServer.PublishingPoints.Count;
    switch(Option.toLowerCase())
    {
        case 'od':
            dwOption = WMS_PUBLISHING_POINT_TYPE_ON_DEMAND;
            break;
        case 'bc': 
            dwOption = WMS_PUBLISHING_POINT_TYPE_BROADCAST;
            break;
        case 'cache' :
            dwOption = WMS_PUBLISHING_POINT_TYPE_CACHE;
            break;
        case 'proxy':
            dwOption = WMS_PUBLISHING_POINT_TYPE_PROXY;
            break;
        case 'all': 
            dwOption = 0;
            break;
        default:
            dwOption = 99;
            Trace("Invalid Type"); 
            WScript.Quit(1); 
            break;
    } 
    //Search Publishing Points of specified type/option
    var i = 0;
    for(i = 0; i < dwNumPubPoints; i++ )
    {
        szPubName = "";
        szPublishingPointStatus = "";
        szPubClients = "";
        szPubBWdth = "";
        szPubPointPath = "";
        
        objPubPoint = objServer.PublishingPoints.Item( i );
        dwPubPointType = objPubPoint.Type;
        if( ( 0 == dwOption ) || ( dwOption == dwPubPointType ) )
        {
            switch( dwPubPointType )
            {
                case WMS_PUBLISHING_POINT_TYPE_ON_DEMAND:
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
                    
                case WMS_PUBLISHING_POINT_TYPE_BROADCAST:
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
                    
                case WMS_PUBLISHING_POINT_TYPE_CACHE: 
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
                    
                case WMS_PUBLISHING_POINT_TYPE_PROXY:
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
            
            szPubName = objPubPoint.Name;
            szPubClients = objPubPoint.CurrentCounters.ConnectedPlayers;
            szPubBWdth = objPubPoint.CurrentCounters.PlayerAllocatedBandwidth + objPubPoint.CurrentCounters.OutgoingDistributionAllocatedBandwidth;
            
            szTemp = "\nName : " + szPubName;
            if( 0 < szPubPointPath.length )
            {
                szTemp = szTemp + "\nPath : " + szPubPointPath;
            }
            if( 0 == dwOption )
            {
                szTemp = szTemp + "\nType : " + szPublishingPointType;
            }
            szTemp = szTemp + "\nStatus : " + szPublishingPointStatus;
            szTemp = szTemp + "\nCurrent # of clients : " + szPubClients;
            szTemp = szTemp + "\nCurrent Bandwidth usage : " + szPubBWdth;
            Trace(szTemp);
        }
    }
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
    szTemp = "Usage: listpub [-s <Server1>] -t <od|bc|cache|proxy|all>";
    szTemp = szTemp + "\nOnly one server name is accepted. ";
    szTemp = szTemp + "\nIf server name is not mentioned, local host is the default server ";
    Trace(szTemp);
    WScript.Quit(1);
}

function Trace( Msg )
{
    WScript.Echo( Msg );
}
