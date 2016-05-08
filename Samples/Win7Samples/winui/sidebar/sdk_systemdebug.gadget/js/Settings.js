/*************************************************************************
 *
 * File: Settings.js
 *
 * Description: 
 * Script for the settings functionality of the 
 * "Debug" Desktop Gadget sample. 
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

// Initialize the settings.
function LoadSettings()
{
    var sFilename = 
        System.Gadget.Settings.readString("settingsFilename");
    var sFileContent = 
        System.Gadget.Settings.readString("settingsContent");
    
    if (String(sFilename) != "")
    {
	    txtFilename.value = sFilename;
	}
    txtFilename.select();
    
    if (String(sFileContent) != "")
    {
	    taContent.value = sFileContent;
	}
}

// Handle the Settings dialog closing event.
// Parameters:
// event - event arguments.
function SettingsClosing(event)
{
    // Save the settings if the user clicked OK.
    if (event.closeAction == event.Action.commit)
    {
        if (IsValid(txtFilename.value) == true)
        {
            txtFilename.className = "inputValid";
	        System.Gadget.Settings.writeString(
	            "settingsFilename", txtFilename.value);
	        System.Gadget.Settings.writeString(
	            "settingsContent", taContent.value);
            event.cancel = false;
	    }
        else
        {
            event.cancel = true;
            txtFilename.className = "inputInvalid";
        }
    }
}

// Validate user input. 
// Returns False on failure.
// Note: You should build a function or functions that include regular 
// expressions to verify that the input is correctly formed, and 
// if it is not, you should reject the data. The following is a loose   
// example that only allows alphanumerics, periods, dashes, and spaces. 
// Parameters:
// sUserInput - user input.
function IsValid(sUserInput)
{
  var regexp = /[\.\w\-\\s]$/gi;
  return regexp.test(sUserInput);
}
