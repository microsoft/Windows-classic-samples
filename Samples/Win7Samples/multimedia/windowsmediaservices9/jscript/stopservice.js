/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : Stop the Windows Media Server Service (stopservice)  
 Script version : 1.0 
 Description    : This script stops the Windows Media Service on the 
 specified Server. If the -s parameter is not specified, the service on the 
 local machine should be stoped.  
           [-s <Server1, [Server2, Server N]>] 
 where  -s represents target server.
 Example : stopservice -s server1,server2 
 Returns  : 
 1. Usage: stopservice [-s <Server1, [Server2, Server N]>] 
 2. Server %server% is not a valid WMS Server
 OS Requirements       :  Windows Server 2003 (all versions)
 Software requirements :  WMS Server 
 Scripting Engine      : Jscript
---------------------------------------------------------------------*/
var szEachArgument = "";
var objWMI = null;
var objWMIServices = null;
var dwEachArg = 0;
var szTemp = "";
var objArgs = WScript.Arguments;
var dwNumArgs = WScript.Arguments.length;

// Parse the command to separate out the server names  

while(dwEachArg< dwNumArgs )
{ 
    szEachArgument = objArgs( dwEachArg );
    if( szEachArgument.toLowerCase()== "-s" )
    {   
        dwEachArg = dwEachArg + 1; 
        if( dwEachArg >= dwNumArgs )    //ex: stopservice -s
        {
            DisplayUsage();
        } 
        szEachArgument = objArgs(dwEachArg);  
        objArgs=szEachArgument.split(",");        
    }           
    else
    {
        DisplayUsage();     //ex: stopservice -s s1,s2 junk
    }   
    dwEachArg = dwEachArg + 1; 
}   

if(objArgs.length == 0) 
{
    //i.e. if server name is not mentioned, then start Publishing points on localhost  
    try
    {
        objWMI = ActiveXObject("winmgmts:");
        objWMIServices = objWMI.Get("Win32_Service.Name='wmserver'")
        objWMIServices.stopService;
        szTemp = "Server 'LocalHost' stopped successfully \n";
    }
    catch(e)
    {
        var errorcode = e.number >>> 0;
        szTemp = "Error Code 0x" + errorcode.toString( 16 ) + ": 'LocalHost' could not be stopped \n";
    }
}
else 
{
    szTemp = "";
    var dwEachServer = 0; 
    var szEachServerName = "";
    while( dwEachServer < objArgs.length )  
    {
        szEachServerName = objArgs[ dwEachServer ];
        try
        {
            var strPath = "winmgmts://" + szEachServerName;
            objWMI = ActiveXObject(strPath);
            objWMIServices = objWMI.Get("Win32_Service.Name='wmserver'")
            objWMIServices.stopService;
            szTemp = szTemp + "Server '" + szEachServerName + "' stopped successfully \n";     
        }
        catch(e)
        {
            var errorcode = e.number >>> 0;
            szTemp = szTemp + "Error Code 0x" + errorcode.toString( 16 ) + ": '" + szEachServerName + "' could not be stopped \n";
        }
        dwEachServer = dwEachServer + 1 ; 
    }
}   

Trace( szTemp );

function DisplayUsage()
{ 
    szTemp = "Usage: StopService [-s <Server1, [Server2, ServerN]>] \n";  
    szTemp = szTemp + "If server name(s) is/are not mentioned, local host is the default server \n";
    Trace( szTemp );
    WScript.Quit(1);
}

function Trace(Msg)
{
    WScript.Echo(Msg);
}

