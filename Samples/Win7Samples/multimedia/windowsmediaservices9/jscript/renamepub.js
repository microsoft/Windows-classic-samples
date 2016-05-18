/*---------------------------------------------------------------------
 Copyright (C) Microsoft Corporation. All rights reserved.
 Script name    : Rename Publishing Point (renamepub)  
 Script version : 1.0 
 Description    : This script renames the publishing point on the 
                  specified server to a new name.  
 Command line parameters : 
           [-s <Server1>] -p <pub1> -n <newpub1>  
 where  -s represents target server, -p represents publishing point name,
 -n represents new name for that publishing point. 
 
 Example : renamepub -s s4 -p pub1 -n newpub1 
 Returns  : 
 1. Usage: renamepub   [-s <Server1>] -p <pub1> -n <newpub1>  
 2. Server %server% is not a valid WMS Server
 3. Src Publishing Point %pubname% etc is not a valid publishing point
 4. Destination Publishing Point %newpubname% already exists. Please use a new name
 OS Requirements       :  Windows Server 2003 (all versions)
 Software requirements :  WMS Server 
 Scripting Engine      : Jscript
 ---------------------------------------------------------------------*/
WMS_PUBLISHING_POINT_TYPE_ON_DEMAND      = 1;
WMS_PUBLISHING_POINT_TYPE_BROADCAST      = 2;

var objServer = null;
var objPubPoint = null;
var dwWhichArg = 0;
var dwNumPubPoints = 0;
var szArgServer = "";
var szArgNewPubPoint = "";
var szArgOldPubPoint = "";
var szEachArgument = "";
var szEachPubPointName = "";
var szTemp = "";
var flag = 0;
var bCheckedArgP = false;
var bCheckedArgN = false;
var bNameAlreadyInUse = false;

var objArgs = WScript.Arguments;
var dwNumArgs = WScript.Arguments.length;

if(WScript.Arguments.length == 0) 
{
    DisplayUsage();
}

// Parse the command to seperate out the server name and publishing points. 

while( dwWhichArg < WScript.Arguments.length )
{ 
    szEachArgument = objArgs( dwWhichArg );
    if( szEachArgument.toLowerCase()== "-s" )
    {   
        dwWhichArg = dwWhichArg + 1; 
        if(dwWhichArg >=WScript.Arguments.length)    //ex: renamepub -p p1 -s
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg);  
        if((szEachArgument.toLowerCase()== "-p")||(szEachArgument.toLowerCase()== "-n"))  //if next szEachArgument is -p, display usage : since syntax is wrong
        { 
            // dwWhichArg = dwWhichArg - 1;  //come back to previous szEachArgument. 
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
        bCheckedArgP = true; 
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)    //ex: renamepub -s s1 -p
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg); 
        if((szEachArgument.toLowerCase()== "-s")||(szEachArgument.toLowerCase()== "-n"))  //if next szEachArgument is -s, display usage : since syntax is wrong
        { 
            // dwWhichArg = dwWhichArg - 1;  //come back to previous szEachArgument. 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one server name 
            if(szEachArgument.lastIndexOf(",")== -1)
            { 
                szArgOldPubPoint = szEachArgument;  
            } 
            else 
            {   
                DisplayUsage(); 
            } 
        }
    }
    else if(szEachArgument.toLowerCase()== "-n")   
    {            
        bCheckedArgN = true; 
        dwWhichArg = dwWhichArg + 1;

        if(dwWhichArg >=WScript.Arguments.length)    //ex: renamepub -p p1 -n
        {
            DisplayUsage();
        }
        szEachArgument = objArgs(dwWhichArg); 
        if((szEachArgument.toLowerCase()== "-p")||(szEachArgument.toLowerCase()== "-s"))  //if next szEachArgument is -s, display usage : since syntax is wrong
        { 
            // dwWhichArg = dwWhichArg - 1;  //come back to previous szEachArgument. 
            DisplayUsage(); 
        }
        else
        { 
            //accept only one server name 
            if(szEachArgument.lastIndexOf(",")== -1)
            { 
                szArgNewPubPoint = szEachArgument;  
            } 
            else 
            {
                DisplayUsage();
            }
        }
    }  
    else  // if argument is not -p,-s  i.e. if it is an invalid argument    
    {
        DisplayUsage();
    }
    dwWhichArg = dwWhichArg + 1; 
}

if((!bCheckedArgP)||(!bCheckedArgN))       //Ex:  renamepub -s "s1"   and -p or -n part is missing. 
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
Trace("Renaming Publishing Points at "+szArgServer);     

//Search old Publishing Point
flag = 0;
bNameAlreadyInUse = false;
dwNumPubPoints = objServer.PublishingPoints.Count;
var i = 0; 
while( i < dwNumPubPoints )
{
    szEachPubPointName = objServer.PublishingPoints.item( i ).name;
    if( szArgOldPubPoint.toLowerCase() == szEachPubPointName.toLowerCase() ) 
    {   
        flag = 1;  
    } 
    if( szArgNewPubPoint.toLowerCase() == szEachPubPointName.toLowerCase() ) 
    {     
        bNameAlreadyInUse = true;   
    } 
    i = i + 1;   
}
 
if( flag == 0 )
{ 
    szTemp = "Publishing Point "+szArgOldPubPoint+" does not exist ";
    Trace(szTemp); 
    WScript.Quit(1);
}
else    //flag = 1
{ 
    if( bNameAlreadyInUse )
    {
        szTemp = "A publishing point by the name of "+ szArgNewPubPoint +" already exists";
        Trace(szTemp); 
        WScript.Quit(1);
    }   
    else    // flag==1 and flagexist == 0 
    {
        RenamePubPoint();                
    }
}
    
    
function RenamePubPoint()
{ 
    var i =0;
    while( i < dwNumPubPoints )
    {
        objPubPoint = objServer.PublishingPoints.item( i ).name;      
        if(szArgOldPubPoint.toLowerCase() == objPubPoint.toLowerCase())
        {       
            //rename the publishing point
            objServer.PublishingPoints.item( i ).name = szArgNewPubPoint ; 
            szTemp = szArgOldPubPoint +" is renamed as " + szArgNewPubPoint;
            Trace(szTemp);       
            break; 
        }
        i = i + 1;
    }
}

function DisplayUsage()
{ 
    Trace("Usage: renamepub [-s <Server1>] -p <pub1> -n <newpub1>  ");  
    Trace("Only one server name is accepted. ");
    Trace("If server name is not mentioned, local host is the default server ");
    WScript.Quit(1);
}

function Trace(Msg)
{
    WScript.Echo(Msg);
}
