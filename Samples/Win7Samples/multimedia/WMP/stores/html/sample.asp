<HTML>

    <!-- Copyright (C) Microsoft Corporation. All rights reserved. -->
    <!-- sample.asp Sample service task Web page for the Windows Media Player SDK -->
    
    <BODY onLoad="Init();" onUnload="Shutdown();">
        <h5>Windows Media Player Premium Service Sample</h5>
        Pending: 
        <!-- Download collections list -->
        <SELECT id="selDLC" name="selDLC" onChange="OnSelectDLC();" STYLE="WIDTH:200px">
        </SELECT>&nbsp;&nbsp;
  
        <!-- Start download button -->
        <INPUT type="button" name="btnDownload" value="Download" onClick="OnDownload();">
        <BR>
        <!-- Pause download button -->
        <INPUT type="button" name="btnPause" value="Pause" onClick="OnPause();">
        <!-- Resume download button -->
        <INPUT type="button" name="btnResume" value="Resume" onClick="OnResume();">
        <!-- Cancel download button -->
        <INPUT type="button" name="btnCancel" value="Cancel" onClick="OnCancel();">
        <!-- Remove download button -->
        <INPUT type="button" name="btnRemove" value="Remove" onClick="OnRemove();">
        <BR>

        Download items:<BR>        
	<!-- Download items list -->
        <SELECT id="selDLItem" name="selDLItem" onChange="OnSelectDLItem();" size="5" STYLE="WIDTH:75%">
        </SELECT>

        <!-- Show download status DIV -->
        <DIV name="dlstate" id="dlstate">
            Status info shows here when you select an item.
        </DIV>

        <SCRIPT Language="JScript">
// Global variables
var g_oManager; // Global download manager object.
var g_oCurrentDLC; // Global current DLC user selection.
var g_oCurrentDLItem; // Global current DL item user selection.
var g_DLI = new Array(5); // Array of new download items.
var g_sDLType = "background"; // "real time" or "background" 
                             // Note that background downloading requires Microsoft Windows XP.
var g_Pending = 0; // Count of pending downloads
var g_sCountCookie = "WMPDLCCOUNT";  // Name of pending count cookie.

// Form for cookie names is: g_sPreCookie + iIndex, 
// where iIndex is the index of the pending download in the selection 
// dropdown list.
// Ex.:  WMPDLC0, WMPDLC1, ...
// Use unique names for your cookies.
var g_sPreCookie = "WMPDLC"; // Prepend other cookies with this string.

var g_sDLStates = new Array( "Downloading", "Paused", "Processing", "Completed", "Canceled" );

// Replace these values with URLs for your digital media files.
var g_sFiles = new Array( "http://www.proseware.com/song1.wma", "http://www.proseware.com/song2.wma", "http://www.proseware.com/song3.wma", "http://www.proseware.com/song4.wma", "http://www.proseware.com/song1.wma" );

// Initialize the service.
function Init()
{
    btnDownload.disabled = true;
    var DLCount = 0;
 
    DLCount = GetPendingDlCount();
    g_Pending = DLCount;
    PopulateDlList(DLCount);
 
    // Initialize the download manager.
    try
    {
        g_oManager = external.DownloadManager;
        btnDownload.disabled = false;
        
        // Set up the color change event
        // This is only available for Windows XP.
        external.OnColorChange = OnAppColor;
        // Sync to the current color
        OnAppColor();
    }
    catch(err)
    {
        dlstate.innerHTML = err.Description;
    }
    
    OnSelectDLC(); // Init the dropdown list.     
    window.setInterval("OnTimer()", 1000); // Timer for status display.
 
    // Initialize the items list.   
    selDLItem.selectedIndex = 0;
    OnSelectDLItem();
}

// Persist session data using cookies.
function Shutdown()
{
    SetPendingCookies();
    SetCookie(g_sCountCookie, g_Pending.toString(10)); // Write the count cookie.    
}

// Handle Player color changes.
function OnAppColor()
{
    window.document.bgColor = external.appColorLight;
}

/**********************************************************
* 
*  Button handlers
*
**********************************************************/
// User clicked the Download button. Start the downloads.
function OnDownload()
{
    var oDLC = null;

    try
    {
        oDLC = g_oManager.createDownloadCollection();
        g_DLI[0] = oDLC.startDownload(g_sFiles[0], g_sDLType);
        g_DLI[1] = oDLC.startDownload(g_sFiles[1], g_sDLType);
        g_DLI[2] = oDLC.startDownload(g_sFiles[2], g_sDLType);
        g_DLI[3] = oDLC.startDownload(g_sFiles[3], g_sDLType);
        g_DLI[4] = oDLC.startDownload(g_sFiles[4], g_sDLType);

        // Update the selection list.        
        var oOption = document.createElement("OPTION");
        oOption.text = oDLC.id.toString(10);
        oOption.value = selDLC.length;
        selDLC.add(oOption);
        selDLC.selectedIndex = selDLC.length - 1;
        OnSelectDLC();
    }
    catch(err)
    {
        dlstate.innerHTML = err.Description;
    }
}

// User clicked Pause
function OnPause()
{
    if(g_oCurrentDLItem != null)
    {
        g_oCurrentDLItem.pause();
    }
}

// User clicked Resume
function OnResume()
{
    if(g_oCurrentDLItem != null)
    {
        g_oCurrentDLItem.resume();
    }
}

// User clicked Cancel
function OnCancel()
{
    if(g_oCurrentDLItem != null)
    {
        g_oCurrentDLItem.cancel();
    }
}

// User clicked Remove
function OnRemove()
{
    if(g_oCurrentDLItem != null &&
       g_oCurrentDLC != null)
    {
        g_oCurrentDLC.removeItem(selDLItem.selectedIndex);
        selDLItem.options[selDLItem.selectedIndex].text = "Item removed";
    }
}

/***************************************************************
*
*   Status timer
*
****************************************************************/        
// Update the status display.
function OnTimer()
{
    dlstate.innerHTML = "";
    
    if(g_oCurrentDLItem != null)
    {   
        var Size = g_oCurrentDLItem.size <=0 ? "Waiting..." : g_oCurrentDLItem.size + " bytes";
        var Progress = g_oCurrentDLItem.progress <=0 ? "Waiting..." : g_oCurrentDLItem.progress + " bytes";
        
        switch(g_oCurrentDLItem.downloadState)
        {
            case 3:            
                Size = "Completed";
                Progress = "Completed";
                break;
                
            case 4:
                Size = "Canceled";
                Progress = "Canceled";
                break;
                
            default:
                break;                
        }
        
        dlstate.innerHTML += "State: " + g_sDLStates[g_oCurrentDLItem.downloadState] + "<BR>";
        dlstate.innerHTML += "Type: " + g_oCurrentDLItem.type + "<BR>"
        dlstate.innerHTML += "Size: " + Size + "<BR>";
        dlstate.innerHTML += "Progress: " + Progress + "<BR>";
    }
    else
    {
        dlstate.innerHTML = "Status info shows here when you select an item."
    }
}

/***************************************************************
*
*   Cookie functions
*
***************************************************************/
// Test whether the DLCount cookie exists
function GetPendingDlCount()
{
    var DLCount;
    var Cookieval;
       
    Cookieval = GetCookie(g_sCountCookie);

    if(Cookieval == null)
    {
        DLCount = 0;
    }
    else
    {
        DLCount = Cookieval.valueOf();
    }      

    return DLCount;
}

// Write a cookie with the provided name and value.
function SetCookie(sName, sValue)
{
    date = new Date();
    document.cookie = sName + "=" + escape(sValue) + "; expires= Mon, 31 Dec 2010 23:59:59 UTC";
}

// Retrieve the value of the cookie with the specified name.
function GetCookie(sName)
{
    // cookies are separated by semicolons
    var aCookie = document.cookie.split("; ");
    for (var i=0; i < aCookie.length; i++)
    {
        // a name/value pair (a crumb) is separated by an equal sign
        var aCrumb = aCookie[i].split("=");
        if (sName == aCrumb[0]) 
        return unescape(aCrumb[1]);
    }

    // a cookie with the requested name does not exist
    return null;
}

// Delete the cookie with the specified name.
function DelCookie(sName)
{
    document.cookie = sName + "=" + escape(" ") + "; expires=Fri, 31 Dec 1999 23:59:59 GMT;";
}

// Write cookies for pending downloads.
function SetPendingCookies()
{
    g_Pending = 0; // Init the pending count.
    
    for(var i = 0; i < selDLC.length; i++)
    {
        var sCookieName = g_sPreCookie + i.toString(10);
        var sCookieVal = selDLC.options[i].text;
    
        // Write a cookie for each pending download.    
        if(IsDLCComplete(sCookieVal.valueOf()) == false)
        {      
            SetCookie(sCookieName, sCookieVal);
            g_Pending++;
        }        
    }
}

// Returns a boolean specifying whether every item
// in a download collection is completed or cancelled.
function IsDLCComplete(ID)
{  
    var oDLC = g_oManager.getDownloadCollection(ID);
    var iCount = oDLC.count;
    var bRet = true;
  
    for (var i = 0; i < iCount; i++)
    {
        if(oDLC.item(i).downloadState != 3 &&
           oDLC.item(i).downloadstate != 4)
        {
            return false; // Some pending item in the collection.
        }     
    }

    return bRet; // No pending items.    
}

/**********************************************************
*
*   List control functions
*
***********************************************************/
// Fill the dropdown list with pending download collection IDs.
function PopulateDlList(iCount)
{
    ClearList(selDLC);
     
    // For each cookie, add the value to the SELECT element.
    // The value represents a download collection ID.  
    for (var j = 0; j < iCount; j++)
    {
        var sCookieName = g_sPreCookie + j.toString(10);        
        var sCookieVal = GetCookie(sCookieName);
        DelCookie(sCookieName); // Don't leave the cookies lying around. 
  
        if(sCookieVal != null)
        {      
            var oOption = document.createElement("OPTION");
            oOption.text = sCookieVal;
            oOption.value = j;
            selDLC.add(oOption); 
        }          
    }
}

// Add the download collection items to the items list box.
function PopulateItemList(ID)
{
    ClearList(selDLItem);
    
    var oDLC = g_oManager.getDownloadCollection(ID);
    var iCount = oDLC.count;
  
    // If all the items were "real time", they don't persist between sessions.
    // Tell the user.
    if(iCount == 0)
    {
        var oOption = document.createElement("OPTION");
        oOption.value = 0;
        oOption.text = "No items left in the collection";
        selDLItem.add(oOption);
    }
          
    for (var i = 0; i < iCount; i++)
    {
        var oOption = document.createElement("OPTION");
        var oDLItem = oDLC.item(i);
        oOption.value = i;
        oOption.text = oDLItem.sourceURL;
        selDLItem.add(oOption);
    }
}

// Empty a select element list box.
function ClearList(list)
{
   // Remove existing options from the list
    for (var i=list.length; i>0; i--)
    {
        list.remove(i-1);
    }
}

// Runs when the user changes the selection.
// Call this whenever you change selection in code.
function OnSelectDLC()
{
    if(selDLC.length <= 0)
    {
        return;
    }
     
    var CurrentDLCId = selDLC.options[selDLC.selectedIndex].text.valueOf();

    if(CurrentDLCId != null)
    {
        try
        {  
            // Try to get the download collection.
            g_oCurrentDLC = g_oManager.getDownloadCollection(CurrentDLCId);
            PopulateItemList(CurrentDLCId);
        }
        catch(err)
        {
            dlstate.innerHTML = err.Description;
        }
    }
  
    OnTimer();  // Update the status display immediately.
}

// The user selected an item.
// Call this whenever you change the selection in code.
function OnSelectDLItem()
{
    if(g_oCurrentDLC != null &&
       selDLItem.selectedIndex != null)
    {
        try
        {
            g_oCurrentDLItem = g_oCurrentDLC.item(selDLItem.selectedIndex);
        }
        catch(err)
        {
            dlstate.innerHTML = err.Description;
        }
    }
    
    OnTimer(); // Update the status display immediately.
}

        </SCRIPT>
    </BODY>
</HTML>
