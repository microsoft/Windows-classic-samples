/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : Set Publishing Point Limits (setlimits)  
 Script version : 1.0 
 Description    : Set the resource limits  
                1. Max unicast clients  
                2. Max. aggregate bandwidth 
for a publishing point on a server. 
                
 Command line parameters : 
           [-s <Server1,server2,Server N>] 
           -p [Pubname]
           -muc <max unicast clients allowed> 
           -mbw <max bandwidth usage allowed>  
 -s represents target server, -p represents publishingpoint name
 -muc represent max unicast clients allowed 
  Example : setlimits -s Server1,server2 -muc 100 -mbw 230 
 Returns  : 
 1. Usage: setlimits [-s <Server1,server2,Server N>] -p [Pubname] -muc <max unicast clients allowed> 
     -mbw <max bandwidth usage allowed>   
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
var dwNumPubPoints = 0; 
var dwPubPointIndex = 0;
var ServerArgsList = "";
var szEachArgument = "";
var szTemp = "";
var szArgMUC = "";
var szArgMBW = "";
var bCheckedArgMUC = false;
var bCheckedArgMBW = false;

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
        if( dwWhichArg >= dwNumArgs )    //ex: setlimits -muc -1 -s 
        {
            DisplayUsage();
        }
        szEachArgument = objArgs( dwWhichArg );
        ServerArgsList = szEachArgument.split(",");        
        //if next szEachArgument is -muc,-mmc,-mbw display usage : since syntax is wrong
        if((szEachArgument.toLowerCase()== "-p")||(szEachArgument.toLowerCase()== "-muc") || (szEachArgument.toLowerCase()== "-mmc") || (szEachArgument.toLowerCase()== "-mbw"))   
        { 
            DisplayUsage(); 
        }
    }
    else if(szEachArgument.toLowerCase()== "-p")   
    {            
        CheckP = 1; 
        dwWhichArg = dwWhichArg + 1;

        if( dwWhichArg >=dwNumArgs )    //ex: srvsetlimits -s server1 -p 
        {
            DisplayUsage(); 
        }  
        szEachArgument = objArgs( dwWhichArg ); 
        if((szEachArgument.toLowerCase()== "-s") || (szEachArgument.toLowerCase()== "-mmc") ||(szEachArgument.toLowerCase()== "-muc")||(szEachArgument.toLowerCase()== "-mbw"))   
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one muc value  
            if(szEachArgument.lastIndexOf(",")== -1)
            { 
                objPubPoint = szEachArgument;
            }
            else 
            {
                DisplayUsage(); 
            } 
        }
    }  
                        
    else if(szEachArgument.toLowerCase()== "-muc")   
    {
        bCheckedArgMUC = true; 
        dwWhichArg = dwWhichArg + 1;

        if( dwWhichArg >= dwNumArgs )    //ex: srvsetlimits -s server1 -muc 
        {
            DisplayUsage(); 
        }  
        szEachArgument = objArgs( dwWhichArg );
        if((szEachArgument.toLowerCase()== "-s") || (szEachArgument.toLowerCase()== "-p") ||(szEachArgument.toLowerCase()== "-mmc") || (szEachArgument.toLowerCase()== "-mbw"))   
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one muc value  
            if(szEachArgument.lastIndexOf(",")== -1)
            {
                szArgMUC = szEachArgument;
            } 
            else 
            {
                DisplayUsage(); 
            } 
        }
    }  

    else if(szEachArgument.toLowerCase()== "-mbw")   
    {            
        bCheckedArgMBW = true;
        dwWhichArg = dwWhichArg + 1;

        if( dwWhichArg >= dwNumArgs )
        {
            DisplayUsage();
        }
        szEachArgument = objArgs( dwWhichArg ); 
        if((szEachArgument.toLowerCase()== "-s") || (szEachArgument.toLowerCase()== "-p") || (szEachArgument.toLowerCase()== "-mmc")|| (szEachArgument.toLowerCase()== "-mbw"))   
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one type 
            if(szEachArgument.lastIndexOf(",")== -1)
            {
                szArgMBW = szEachArgument;
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

if((!CheckP)||(!bCheckedArgMUC)||(!bCheckedArgMBW))       //Ex:  compulsory part -muc,-mmc or -mbw is missing. 
{
    DisplayUsage();
}

//i.e. if server name is not mentioned, then start setlimits of localhost server 
if( ServerArgsList.length == 0 )
{
    //i.e. if server name is not mentioned, then start Publishing points on localhost  
    objServer = new ActiveXObject( "WMSServer.server" );
    Trace("\nLocal host started successfully");

   //Set server limits
   SetPubPointLimits(); 
}
else 
{
    srvnum = 0;
    while( srvnum < ServerArgsList.length )
    {
        objServerName = ServerArgsList[ srvnum ];
        objServer = new ActiveXObject( "WMSServer.server", objServerName );
        Trace(objServerName+" started successfully");     

        //Set server limits
        SetPubPointLimits(); 
        srvnum = srvnum + 1 ; 
    }
}   


// This function sets pubpt limits on multiple servers 
function SetPubPointLimits()      
{  
    dwNumPubPoints = objServer.PublishingPoints.Count; 
    while( dwPubPointIndex < dwNumPubPoints )
    {
        szTemp = objServer.PublishingPoints.item( dwPubPointIndex ).name;      
        if( szTemp.toLowerCase() == objPubPoint.toLowerCase() )
        {       
            flag = 1; 
            break; 
        }  
        dwPubPointIndex = dwPubPointIndex + 1;
    }
}   

if( flag == 0 )
{ 
    szTemp = "Publishing Point "+objPubPoint+" does not exist ";
    Trace(szTemp); 
    WScript.Quit(1);
}
else 
{
    objServer.PublishingPoints.Item( dwPubPointIndex ).Limits.ConnectedPlayers = szArgMUC
    objServer.PublishingPoints.Item( dwPubPointIndex ).Limits.PlayerBandwidth = szArgMBW 
    Trace("Server limits are set");        
} 

function DisplayUsage()
{ 
    Trace("Usage: SetLimits [-s <Server1,server2,Server N>] -p [Pubname] -muc <max unicast clients allowed> -mbw <max bandwidth usage allowed>   ");
    WScript.Quit(1);
}

function Trace(Msg)
{
    WScript.Echo(Msg);
}
