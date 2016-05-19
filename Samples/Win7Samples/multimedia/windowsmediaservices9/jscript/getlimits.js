/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : Get Publishing Point Limits (getlimits)  
 Script version : 1.0 
 Description    : Displays the resource limits  
                1. Max connected players
                2. Max bandwidth per player
                3. Max aggregate bandwidth 
for a publishing point on a server. 
                
 Command line parameters : 
           [-s <Server1>] 
           -p [Pubname]
 -s represents target server, -p represents publishingpoint name
 Example : getlimits -s Server1 -p pub1  
 Returns  : 
 1. Usage: getlimits [-s <Server1>] -p [Pubname] 
 2. Server %server% is not a valid WMS Server
 3.  invalid value specified for %parameter% 
 OS Requirements       :  Windows Server 2003 (all versions)
 Software requirements :  WMS Server 
 Scripting Engine      : Jscript
 ---------------------------------------------------------------------*/
var objServer = null;
var objPubPoint = null;
var dwWhichArg = 0;
var flag = 0;
var ServerArgsList = "";
var szEachArgument = "";
var szTemp = "";
var bCheckedPubPointArg = false; 

var objArgs = WScript.Arguments;

if(WScript.Arguments.length == 0) 
{
    DisplayUsage();
}

// Parse the command to seperate out the server name and publishing points. 

while(dwWhichArg<WScript.Arguments.length)
{ 
    szEachArgument = objArgs( dwWhichArg );
    if(szEachArgument.toLowerCase()== "-s")
    {   
        dwWhichArg = dwWhichArg + 1; 
        if(dwWhichArg >=WScript.Arguments.length)    //ex: getlimits -muc -1 -s 
        {
            DisplayUsage();
        }
        szEachArgument = objArgs( dwWhichArg );
        ServerArgsList = szEachArgument.split( "," );
        //if next szEachArgument is -muc,-mmc,-mbw display usage : since syntax is wrong
        if((szEachArgument.toLowerCase()== "-p")||(szEachArgument.toLowerCase()== "-muc") || (szEachArgument.toLowerCase()== "-mmc") || (szEachArgument.toLowerCase()== "-mbw"))   
        { 
            DisplayUsage(); 
        }
    }
    else if(szEachArgument.toLowerCase()== "-p")   
    {            
        bCheckedPubPointArg = true; 
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)    //ex: objServergetlimits -s server1 -p 
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg); 
        if((szEachArgument.toLowerCase()== "-s") || (szEachArgument.toLowerCase()== "-mmc") ||(szEachArgument.toLowerCase()== "-muc")||(szEachArgument.toLowerCase()== "-mbw"))   
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one muc value  
            if(szEachArgument.lastIndexOf(",")== -1)
            {
                objPubPoint=szEachArgument;
            } 
            else 
            {
                DisplayUsage();
            }
        }
    }  
    else  // if argument is not -s,-muc,-mmc,-mbw  i.e. if it is an invalid argument    
    {
        DisplayUsage();
    }

    dwWhichArg = dwWhichArg + 1; 
}

if( ! bCheckedPubPointArg )       //Ex:  compulsory part -p is missing. 
{
    DisplayUsage();
}

//i.e. if server name is not mentioned, then get pubpt limits from localhost server 
if( ServerArgsList.length == 0 ) 
{
    //i.e. if server name is not mentioned, then start Publishing points on localhost  
    objServer = new ActiveXObject( "WMSServer.server" );
    Trace("\nLocal host started successfully");

    //get server limits
    GetPubPointLimits(); 
}
else 
{
    var dwServerIndex = 0; 
    while( dwServerIndex < ServerArgsList.length )  
    {
        objServerName = ServerArgsList[ dwServerIndex ];
        objServer = new ActiveXObject( "WMSServer.server", objServerName );
        Trace(objServerName+" started successfully");

        //get server limits
        GetPubPointLimits(); 
        dwServerIndex = dwServerIndex + 1 ; 
    }
}   

// This function gets pubpt limits on multiple servers 
function GetPubPointLimits()      
{  
    i = 0;
    var dwNumPubPoints = 0;
    dwNumPubPoints = objServer.PublishingPoints.Count; 
    while( i < dwNumPubPoints )
    {
        szTemp = objServer.PublishingPoints.item( i ).name;
        if(szTemp.toLowerCase() == objPubPoint.toLowerCase())
        {
            flag = 1;
            break;
        } 
        i = i + 1;
    }
}   

if( 0 == flag )
{ 
    szTemp = "Publishing Point "+objPubPoint+" does not exist ";
    Trace(szTemp); 
    WScript.Quit(1);
}
else    //flag = 1
{       
    Trace("Max connected players allowed = " + objServer.PublishingPoints.Item( i ).Limits.ConnectedPlayers); 
    Trace("Max bandwidth allowed per player = " + objServer.PublishingPoints.Item( i ).Limits.PerPlayerConnectionBandwidth); 
    Trace("Max aggregate bandwidth allowed = " + objServer.PublishingPoints.Item( i ).Limits.PlayerBandwidth);  
}    

function DisplayUsage()
{ 
    Trace("Usage: GetLimits [-s <Server1>] -p [Pubname]");
    WScript.Quit(1);
}

function Trace(Msg)
{
    WScript.Echo(Msg);
}
