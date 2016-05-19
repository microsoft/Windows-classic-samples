/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : Display connected clients (getclientlist)  
 Script version : 1.0 
 Description    : This script displays a list of clients connected 
                  to a publishing point.
 Command line parameters :  [-s <server1>]-p <pub1>
 where  -s represents target server
 -p represents publishing point 
 Example: getclientlist -s server1, -p pub1
 Returns  : 
 1. Usage: getclientlist [-s <server1>]-p <pub1>
 2. Server %server% is not a valid WMS server.
  
 OS Requirements       :  Windows Server 2003 (all versions)
 Software requirements :  WMS Server 
 Scripting Engine      : Jscript
 ---------------------------------------------------------------------*/
var objServer = null;
var objPubPoint = null;
var dwWhichArg = 0;
var dwNumPlayers = 0;
var szArgServer = "";
var szArgPubPoint = "";
var szEachArgument = "";
var szTemp = "";
var dwPubPointIndex = 0;
var bCheckedVerboseOption = false;
var bCheckedPubPointOption = false;

var PubPointArgumentList = new Array();
var objArgs = WScript.Arguments;

// Parse the command to seperate out the server names  
while(dwWhichArg<WScript.Arguments.length)
{ 
    szEachArgument =  objArgs(dwWhichArg);  
    if(szEachArgument.toLowerCase()== "-s")
    {
        dwWhichArg = dwWhichArg + 1; 
        if(dwWhichArg >=WScript.Arguments.length)    //ex: getclientlist -s
        {  
            DisplayUsage(); 
        }
        szEachArgument = objArgs(dwWhichArg );
        
         //if next szEachArgument is -p, display usage : since sytax is wrong
        if(szEachArgument.toLowerCase()== "-p") 
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one server name 
            if(szEachArgument.lastIndexOf(",")== -1)
            { 
                szArgServer = szEachArgument;  
            } 
            else 
            {   
                DisplayUsage(); 
            } 
        }
    }       
    else if(szEachArgument.toLowerCase()== "-p")   
    {            
        bCheckedPubPointOption = true;
        dwWhichArg = dwWhichArg + 1;

        if( dwWhichArg >= WScript.Arguments.length )    //ex: getclientlist -s server1 -p 
        {
            DisplayUsage();
        }
        szEachArgument = objArgs( dwWhichArg );
        
        //if next szEachArgument is -s, display usage : since syntax is wrong
        if(szEachArgument.toLowerCase()== "-s")
        { 
            DisplayUsage(); 
        }
        else
        {
            //accept only one publishing point name 
            if( szEachArgument.lastIndexOf( "," )== -1 )
            {
                szArgPubPoint=szEachArgument;
            }
            else 
            {
                DisplayUsage();
            } 
        }
    }               
    else  // if argument is not -s  i.e. if it is an invalid argument    
    {
        DisplayUsage();
    }
    dwWhichArg = dwWhichArg + 1; 
}

if( ! bCheckedPubPointOption )
{   
    DisplayUsage();   
} 

// Connect to Server  
//i.e. if server name is not mentioned, then start Publishing points on localhost  
if( szArgServer == "" )
{  
    szArgServer="LocalHost" 
}

objServer = new ActiveXObject( "WMSServer.server", szArgServer );
Trace("Fetching client list for "+szArgServer);   

dwPubPointIndex =  FindPubPointIndex(); 
dwNumPlayers = objServer.PublishingPoints.Item(dwPubPointIndex).Players.Count;
objPubPoint = objServer.PublishingPoints.Item(dwPubPointIndex);
//get client list 
GetClientList(); 


// This function gets pubpt position 
function FindPubPointIndex()        
{  
    var i =0 ; 
    var dwNumPubPoints = 0;
    var bFound = false;
    dwNumPubPoints = objServer.PublishingPoints.Count; 
    while(i<dwNumPubPoints)
    {
        szTemp = objServer.PublishingPoints.item( i ).name;
        if(szTemp.toLowerCase() == szArgPubPoint.toLowerCase())
        {       
            bFound = true; 
            break; 
        }  
        i = i + 1;
    }

    if( false == bFound )
    { 
        szTemp = "Publishing Point "+szArgPubPoint+" does not exist ";
        Trace(szTemp); 
        WScript.Quit(1);
    }
    //else 
    return( i );
}
 

function GetClientList()        
{ 
    var i = 0; 
    if( 0 == dwNumPlayers )
    {
        Trace("No clients connected");
    }
    else 
    {
        for( i = 0; i < dwNumPlayers; i++ )
        {  
            Trace("Authenticated Client Name = " + objPubPoint.Players.Item( i ).UserName);
            Trace("Authenticated Client Name = " + objPubPoint.Players.Item( i ).NetworkAddress); 
        }    
    } 
} 
 

 
function DisplayUsage()
{ 
    Trace("Usage: GetClientList [-s <server1>]-p <pub1>  ");  
    Trace("Only one server name is accepted. ");
    Trace("If server name is not mentioned, local host is the default server ");
    WScript.Quit(1);
}

function Trace(Msg)
{
    WScript.Echo(Msg);
}
