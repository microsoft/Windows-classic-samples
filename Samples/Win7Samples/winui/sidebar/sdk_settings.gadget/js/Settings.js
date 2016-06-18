/*************************************************************************
 *
 * File: Settings.js
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
 
// Delegate for the settings closing event. 
System.Gadget.onSettingsClosing = SettingsClosing;
        
// --------------------------------------------------------------------
// Initialize the settings.
// --------------------------------------------------------------------
function LoadSettings()
{
    var currentSetting = 
        System.Gadget.Settings.readString("settingsUserEntry");
    
    if (String(currentSetting) != "")
    {
        txtUserEntry.innerText = currentSetting;
    }
    txtUserEntry.select();
}

// --------------------------------------------------------------------
// Handle the Settings dialog closing event.
// Parameters:
// event - System.Gadget.Settings.ClosingEvent argument.
// --------------------------------------------------------------------
function SettingsClosing(event)
{
    // User hit OK on the settings page.
    if (event.closeAction == event.Action.commit)
    {
        if (txtUserEntry.value != "")
        {        
            System.Gadget.Settings.writeString(
                "settingsUserEntry", txtUserEntry.value);
            // Allow the Settings dialog to close.
            event.cancel = false;
        }
        // No user entry and 'Ok' invoked, cancel the Settings closing event.
        else
        {
            if (event.cancellable == false)
            {
                event.cancel = true;  
            }
        }
    }
}
