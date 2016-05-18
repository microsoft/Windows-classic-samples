/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : Disconnect client (killclient)  
 Script version : 1.0 
 Description    : This script disconnects one or more clients from  
                  Windows Media Server.
 Command line parameters :  [-s <server1>][-p <pub1>][-all|-l <ipaddress>|-n <clientname>]  
 where  -s represents target server, -all represents all clients
 -p represents publishing point 
 -l represents ipaddress, -n represents client name 
 when -all is used, -l and -n are ignored.
 -l and -n cannot be used together. 
 Example: killclient -s server1 -all 
 Returns  : 
 1. Usage: killclient [-s <server1>][-p <pub1>][-all|-l <ipaddress>|-n <clientname>] 
 2. A client with the specified ip address does not exist 
 3. A client with the specified name does not exist 
 4. Invalid servername has been specified. 
 5. Invalid Publishing point name has been specified.    
 OS Requirements       :  Windows Server 2003 (all versions)
 Software requirements :  WMS Server 
 Scripting Engine      : Jscript
 ---------------------------------------------------------------------*/
var objServer = null;
var objPubPoint = null;
var dwWhichArg = 0;
var dwNumPlayers = 0;
var dwNumPubPoints = 0;
var szArgServer = "";
var szArgPubPoint = "";
var szEachArgument = "";
var szArgIPAddress = "";
var szArgName = "";
var szTemp = "";
var dwPubPointIndex = 0;
var bCheckedIPAddress = false;
var bCheckedClientName = false;
var bCheckedPubPointOption = false;
var bCheckAll = false;

var PubPointArgumentList = new Array();
var objArgs = WScript.Arguments;

// Parse the command to seperate out the server names  
while( dwWhichArg < WScript.Arguments.length )
{ 
    szEachArgument = objArgs(dwWhichArg);  
    if( "-s" == szEachArgument.toLowerCase() )
    {   
        dwWhichArg = dwWhichArg + 1; 
        if(dwWhichArg >= WScript.Arguments.length )    //ex: killclient -s
        {  
            DisplayUsage(); 
        } 
        szEachArgument = objArgs(dwWhichArg);  
        //if next szEachArgument is any valid argument, display usage : since sytax is wrong
        if(( "-p" == szEachArgument.toLowerCase() ) || ( "-all" == szEachArgument.toLowerCase() ) || ( "-l" == szEachArgument.toLowerCase() ) || ( "-n" == szEachArgument.toLowerCase() ))   
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one server name 
            if( -1 == szEachArgument.lastIndexOf(",") )
            { 
                szArgServer = szEachArgument;  
            } 
            else
            {   
                DisplayUsage(); 
            } 
        }
    }                       
    else if( "-p" == szEachArgument.toLowerCase() )
    {            
        bCheckedPubPointOption = true;
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >= WScript.Arguments.length )    //ex: killclient -s server1 -p 
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg); 
        if(( "-s" == szEachArgument.toLowerCase() ) || ( "-all" == szEachArgument.toLowerCase() ) ||( "-l" == szEachArgument.toLowerCase() ) || ( "-n" == szEachArgument.toLowerCase() ))    //if next szEachArgument is -s, display usage : since syntax is wrong
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one publishing point name 
            if( -1 == szEachArgument.lastIndexOf( "," ) )
            { 
                szArgPubPoint = szEachArgument;
            } 
            else 
            {
                DisplayUsage();
            } 
        }
    }  

    else if( "-all" == szEachArgument.toLowerCase() )
    {            
        bCheckAll = true; 
    }  
 
    else if( "-1" == szEachArgument.toLowerCase() )
    {            
        bCheckedIPAddress = true; 
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >= WScript.Arguments.length )    //ex: killclient -s server1 -l 
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg); 
        if(( "-s" == szEachArgument.toLowerCase() )||( "-p" == szEachArgument.toLowerCase() ) || ( "-all" == szEachArgument.toLowerCase() ) || ( "-n" == szEachArgument.toLowerCase() ))   
        {
            DisplayUsage(); 
        }
        else
        { 
            //accept only one ipaddress value  
            if( -1 == szEachArgument.lastIndexOf( "," ) )
            { 
                szArgIPAddress = szEachArgument;
            } 
            else 
            {
                DisplayUsage();
            }
        }
    }  
    
    else if( "-n" == szEachArgument.toLowerCase() )   
    {            
        bCheckedClientName = true; 
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)    //ex: killclient -s server1 -n
        {
            DisplayUsage();
        }  
        szEachArgument = objArgs(dwWhichArg); 
        if(( "-s" == szEachArgument.toLowerCase() ) || ( "-p" == szEachArgument.toLowerCase() ) || ( "-all" == szEachArgument.toLowerCase() ) ||( "-l" == szEachArgument.toLowerCase() ))   
        { 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one ipaddress value  
            if(szEachArgument.lastIndexOf(",")== -1)
            {
                szArgName = szEachArgument;
            } 
            else 
            {
                DisplayUsage();
            } 
        }
    }  
    else  // if it is an invalid argument    
    {
        DisplayUsage();
    }
    dwWhichArg = dwWhichArg + 1; 
}

//if both -l and -n arguments are used, it's wrong 
if(( bCheckedClientName )&&( bCheckedIPAddress )) 
{
    DisplaUsage();
}
// -p must exist and (-l or -n )one of them must exist, if this is not the case, display usage
if(!(( bCheckedPubPointOption ) && (( bCheckAll )||( bCheckedIPAddress )||( bCheckedClientName ))))
{
    DisplayUsage();
}
 
// Connect to Server  
//i.e. if server name is not mentioned, then start Publishing points on localhost  
if( "" == szArgServer )
{  
    szArgServer="LocalHost" 
}   
objServer = new ActiveXObject( "WMSServer.server", szArgServer );

dwPubPointIndex = GetRequestedPubPointIndex(); 
dwNumPlayers = objServer.PublishingPoints.Item( dwPubPointIndex ).Players.Count;
objPubPoint = objServer.PublishingPoints.Item( dwPubPointIndex );
Trace( dwNumPlayers );

if( bCheckAll )
{  
    DisconnectAllClients(); 
}
else if( bCheckedIPAddress )
{ 
    DisconnecsClientsByIPAddr();  
} 
else if( bCheckedClientName )
{ 
    DisconnecsClientsByName();  
}
else 
{
    DisplayUsage(); 
}

function DisconnectAllClients()        
{  
    if( 0 == dwNumPlayers )
    { 
        Trace("No clients connected");
    } 
    else 
    {
        var strUserName;
        var i = 0;
        for( i = ( dwNumPlayers - 1 ); i >= 0; i-- ) 
        {
            // objPubPoint.Players.RemoveAll();
            strUserName = objPubPoint.Players.Item( i ).UserName;
            objPubPoint.Players.Remove( i );
            Trace( "Disconnected " + strUserName );
        }    
    } 
} 

function DisconnecsClientsByIPAddr()      
{ 
    if( 0 == dwNumPlayers )
    { 
        Trace("No clients connected");
    } 
    else 
    {
        var i = 0;
        for( i = ( dwNumPlayers - 1 ); i >= 0; i-- ) 
        {
            if( szArgIPAddress == objPubPoint.Players.Item( i ).NetworkAddress )
            { 
                objPubPoint.Players.Remove( i );
            }
            Trace("Disconnected "+ objPubPoint.Players.Item( i ).NetworkAddress );
            break; 
        }    
    } 
} 
 
// This function sets pubpt limits on multiple servers 
function GetRequestedPubPointIndex()        
{  
    var flag = 0;
    var i = 0;
    dwNumPubPoints = objServer.PublishingPoints.Count; 
    while( i < dwNumPubPoints )
    {
        szTemp = objServer.PublishingPoints.item( i ).name;      
        if(szTemp.toLowerCase() == szArgPubPoint.toLowerCase())
        {
            flag = 1; 
            break; 
        }  
        i = i + 1;
    }

    if( 0 == flag )
    { 
        szTemp = "Publishing Point "+szArgPubPoint+" does not exist ";
        Trace(szTemp); 
        WScript.Quit(1);
    }
    //else 
    return( i );      
 
}
 
function DisconnecsClientsByName()        
{ 
    if( 0 == dwNumPlayers )
    { 
        Trace("No clients connected");
    } 
    else 
    {
        var i = 0;
        for( i = ( dwNumPlayers - 1 ); i >= 0; i-- ) 
        {
            if( szArgName == objPubPoint.Players.Item( i ).UserName )
            { 
                objPubPoint.Players.Remove( i );
            }
            Trace("Disconnected "+ objPubPoint.Players.Item( i ).UserName );
            break; 
        }    
    } 
} 

 
function DisplayUsage()
{ 
    Trace("Usage: KillClient -s <server1> -p <pub1> [-all|-l <ipaddress>|-n <clientname>] ");  
    Trace("Only one server name is accepted. ");
    Trace("If server name is not mentioned, local host is the default server ");
    WScript.Quit(1);
}

function Trace(Msg)
{   
    WScript.Echo(Msg);
}
