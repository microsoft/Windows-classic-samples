/*************************************************************************
 *
 * File: Example.js
 *
 * Description: 
 * Script that controls the settings functionality for the 
 * "Settings" Desktop Gadget sample. 
 * 
 * This file is part of the Microsoft Windows SDK Code Samples.
 * 
 * Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/
 
// Member variables.
var defaultText = "Enter text using the gadget settings.\n\n" + 
    "Click the wrench icon or right click the gadget and select 'Options' " +
    "from the context menu.";
var userEntry = "";

// Enable the gadget settings functionality. 
System.Gadget.settingsUI = "Settings.html";
// Delegate for when the Settings dialog is closed.
System.Gadget.onSettingsClosed = SettingsClosed;
// Delegate for when the Settings dialog is instantiated.
System.Gadget.onShowSettings = SettingsShow;

// --------------------------------------------------------------------
// Initialize the gadget.
// --------------------------------------------------------------------
function Init()
{    
    // Retrieve an existing user entry, if any.
    userEntry = 
        System.Gadget.Settings.readString("settingsUserEntry");
		    
    // Initialize the gadget content.
    SetContentText();
}

// --------------------------------------------------------------------
// Set the text of the gadget based on the user input; 
// execute this function at startup and after settings changes.
// txtGadget = control for displaying user input.
// --------------------------------------------------------------------
function SetContentText(strFeedback)
{
    if (strFeedback)
    {
	    txtGadget.innerText = strFeedback;
	}
	else
	{
	    txtGadget.innerText = defaultText;
	}
}

// --------------------------------------------------------------------
// Handle the Settings dialog closed event.
// event = System.Gadget.Settings.ClosingEvent argument.
// --------------------------------------------------------------------
function SettingsClosed(event)
{
    // User hit OK on the settings page.
    if (event.closeAction == event.Action.commit)
    {
        userEntry = 
            System.Gadget.Settings.readString("settingsUserEntry");
        SetContentText(userEntry);
    }
    // User hit Cancel on the settings page.
    else if (event.closeAction == event.Action.cancel)
    {
        SetContentText("Cancelled");
    }
}

// --------------------------------------------------------------------
// Handle the Settings dialog show event.
// --------------------------------------------------------------------
function SettingsShow()
{
    SetContentText("Settings opening.");
}
