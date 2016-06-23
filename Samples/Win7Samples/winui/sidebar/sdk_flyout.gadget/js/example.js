/*************************************************************************
 *
 * File: example.js
 *
 * Description: 
 * Script that controls the gadget functionality for the 
 * "Flyout" Desktop Gadget sample. 
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
var oFlyoutDocument;

// Initialize the gadget.
function Init()
{
    // Specify the flyout root.
    System.Gadget.Flyout.file = "flyout.html";
    
    // Initialize the Flyout state display.
    if (!System.Gadget.Flyout.show)
    {
        strFlyoutFeedback.innerText = "Flyout hidden.";
    }
}

// Display the flyout associated with the "Flyout" gadget sample.
function showFlyout()
{
    System.Gadget.Flyout.show = true;
    oFlyoutDocument = System.Gadget.Flyout.document;
}
