/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : Delete Publishing Point (deletepub)  
 Script version : 1.0 
 Description    : This script removes the publishing point(s) from the specified
 server. If the all option is specified, all the publishing points are deleted. 
 Command line parameters : 
           [-s <Server1>] -p <pub1[,pub2,pubN|all]> 
 where  -s represents target server, -p represents publishing point name(s).
 EX: deletepub -p pub1    
 Returns  : 
 1. Usage: deletepub [-s <Server1>] -p <pub1[,pub2,pubN|all]> 
 2. Server %server% is not a valid WMS Server
 3. Publishing Point %pub% etc is not a valid publishing point on server %server%
 OS Requirements       : Windows Server 2003 (all versions)
 Software requirements : WMS Server 
 Scripting Engine      : Jscript
 ---------------------------------------------------------------------*/
WMS_PUBPOINT_ONDEMAND = 1;
WMS_PUBPOINT_BROADCAST = 2; 

var objServer = null;
var objPubPoint = null;
var dwWhichArg = 0;
var dwNumPubPoints = 0; 
var szArgServer = "";
var szEachArgument = "";
var szTemp = "";
var dwEachPubIndex = 0;
var bCheckedPubPointName = false;
var bFailed = false;

var PubPointArgumentList = new Array();
var objArgs = WScript.Arguments;

if(WScript.Arguments.length == 0) 
{
    DisplayUsage();
}

// Parse the command to seperate out the server name and publishing points. 

while(dwWhichArg<WScript.Arguments.length)
{ 
    szEachArgument =  objArgs(dwWhichArg);  
    if(szEachArgument.toLowerCase()== "-s")
    {   
        dwWhichArg = dwWhichArg + 1; 
        if(dwWhichArg >=WScript.Arguments.length)    //ex: deletepub -p p1 -s
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
    else if( szEachArgument.toLowerCase()== "-p" )
    {            
        bCheckedPubPointName = true; 
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)    //ex: deletepub -p p1 -s
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
            PubPointArgumentList = szEachArgument.split(","); 
        }
    }  
    else  // if argument is not -p,-s  i.e. if it is an invalid argument    
    {
        DisplayUsage();
    }
    
    dwWhichArg = dwWhichArg + 1; 
}

if(!bCheckedPubPointName)       //Ex:  deletepub -s "s1"   and -p is part is missing. 
{
    DisplayUsage();
}   

// Connect to Server  
//if server name is not mentioned, then start Publishing points on localhost  
if(szArgServer == "")
{
    szArgServer="LocalHost"
}   

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
    Trace("\nDeleting Publishing Points at "+szArgServer);
     
    //Delete Publishing Points 

    dwNumPubPoints = objServer.PublishingPoints.Count;
    if(PubPointArgumentList[0].toLowerCase() == "all") 
    { 
        for(i = dwNumPubPoints;i > 0;i--) 
        { 
            DeletePubPoint(objServer,i - 1);    
        }
    }
    else 
    {
        cPubList = PubPointArgumentList.length
        
        for( dwEachPubIndex = 0; dwEachPubIndex < cPubList; dwEachPubIndex ++ )
        {
            objPubPoint = PubPointArgumentList[dwEachPubIndex];
            try
            {
                objServer.PublishingPoints.Remove( objPubPoint )
                szTemp = "Deleting " + objPubPoint;
            }
            catch(e)
            {
                // This function is called if the Publishing Point is not found
                szTemp = "Publishing Point "+objPubPoint+" does not exist ";
            }
            Trace(szTemp);
        }
    }   //end of else 
}

function DeletePubPoint( Server, i )
{ 
    objPubPoint = Server.PublishingPoints.item( i ).name;     //required for deletepub -p all
    try
    {
        Server.PublishingPoints.Remove( i );
        szTemp = "Deleting " + objPubPoint;
    }
    catch(e)
    {
        szTemp = "Publishing Point "+objPubPoint+" could not be deleted";
    }
    Trace(szTemp);
}   

function DisplayUsage()
{ 
    szTemp = "Usage: deletepub [-s <Server1>] -p <pub1[,pub2,pubN]|all> ";
    szTemp = szTemp + "\nOnly one server name is accepted. ";
    szTemp = szTemp + "\nIf server name is not mentioned, local host is the default server ";
    Trace( szTemp );
    WScript.Quit(1);
}

function Trace(Msg)
{
    WScript.Echo(Msg);
}
