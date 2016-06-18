/*************************************************************************
 *
 * File: Docked.js
 *
 * Description: 
 * Script for the "Resize" Desktop Gadget sample.
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
 
// Gadget width and height.
var gadgetSmallWidth = 130;
var gadgetSmallHeight = 108;
var gadgetLargeWidth = 260;
var gadgetLargeHeight = 216;

// Amount of time desired to perform transition (in seconds).
var timeTransition = 2;

// Declare the dock and undock event handlers.
System.Gadget.onDock = CheckSize;
System.Gadget.onUndock = CheckSize;

// Check the current size of the gadget; set the gadget style.
function CheckSize()
{
    var oBackground = document.getElementById("imgBackground");
    var oBody = document.body.style;
    if (System.Gadget.docked)
    {
        // "Docked" translates to "Small View"
        oBody.width = gadgetSmallWidth;
        oBody.height = gadgetSmallHeight;
	
        oBackground.src = "../images/bg_docked.png";
        
        txtSize.innerText = 'Small View';
        txtSize.className = 'gadgetSmall';
    }
    else
    {
        // "Undocked" translated to "Large View"
        oBody.width = gadgetLargeWidth;
        oBody.height = gadgetLargeHeight;
          
        oBackground.src = "../images/bg_undocked.png";

        txtSize.innerText = 'Large View';
        txtSize.className = 'gadgetLarge';
    }
}