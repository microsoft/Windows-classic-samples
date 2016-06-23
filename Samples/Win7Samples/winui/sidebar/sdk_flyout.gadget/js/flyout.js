/*************************************************************************
 *
 * File: flyout.js
 *
 * Description: 
 * Script that controls the flyout functionality for the 
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
 
var oGadgetDocument = System.Gadget.document;
System.Gadget.Flyout.onShow = showFlyout;
System.Gadget.Flyout.onHide = hideFlyout;

// Display the Flyout state in the gadget.
function showFlyout()
{
    oGadgetDocument.getElementById("strFlyoutFeedback").innerText = "Flyout visible.";
}

// Hide the flyout and display the Flyout state in the gadget.
function hideFlyout()
{
    oGadgetDocument.getElementById("strFlyoutFeedback").innerText = "Flyout hidden.";
    System.Gadget.Flyout.show = false;
}

// Display user input in the gadget.
function showInGadget(userInput)
{
    oGadgetDocument.getElementById("strFlyoutFeedback").innerText = userInput.value;
}
