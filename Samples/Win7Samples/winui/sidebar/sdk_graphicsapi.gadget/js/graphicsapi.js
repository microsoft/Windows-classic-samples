/*************************************************************************
*
* File: graphicsapi.js
*
* Description: 
* Script that controls the gadget functionality for the 
* "Graphics API" Desktop Gadget sample. 
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

// Global Variables and Constants
var BKRND_ID = "background";                    // ID of the g:background element
var BKRND_PATH = "images/background.png";       // Path of the background image
var BKRND_WIDTH = 130;                          // Width of the background image
var BKRND_HEIGHT = 220;                         // Height of the background image

var oImage;                                     // Demo image g:image object
var IMG_PATH96 = "images/image96.png";          // Demo image path for 96 DPI
var IMG_PATH120 = "images/image120.png";        // Demo image path for 120 DPI
var IMG_PATH144 = "images/image144.png";        // Demo image path for 144 DPI
var IMG_WIDTH = 64;                             // Demo image width
var IMG_HEIGHT = 64;                            // Demo image height
var IMG_LEFT = 32;                              // Demo image left value
var IMG_TOP = 95;                               // Demo image top value

var TXT_FONT = "Calibri";                       // Default font for text
var TXT_COLOR = "Color(255, 255, 255, 255)";    // Default color for text

var oAbstractText;                              // Abstract text g:text object
var TXT_ABSTRACT_LEFT = 7;                      // Abstract text left value
var TXT_ABSTRACT_TOP = 10;                      // Abstract text top value
var TXT_ABSTRACT_SIZE = 13;                     // Abstract text font size

var oText;                                      // Demo text g:text object
var TXT_ANIMATION_LEFT = 16;                    // Demo text left value
var TXT_ANIMATION_TOP = 100;                    // Demo text top value
var TXT_ANIMATION_VALUE = "I <3 Gadgets";       // Demo text string value
var TXT_ANIMATION_SIZE = 18;                    // Demo text font size

var fPlaying = true;                            // Flag that indicates if Animations are currently running
var NAVBAR_ID = "navbar";                       // ID of the Nav Bar element
var NAVBAR_PATH = "images/navbar.png";          // Nav Bar image path
var NAVBAR_WIDTH = 75;                          // Nav Bar image width
var NAVBAR_HEIGHT = 23;                         // Nav Bar image height
var NAVBAR_LEFT = 28;                           // Nav Bar image left value
var NAVBAR_TOP = 184;                           // Nav Bar image top value
var NAVBAR_BUTTONS = "prev play pause next";    // Button string to be parsed by initNavButtons()

var animationFuncs = new Array();               // List of animations to perform
var cFunction = 0;                              // Current function
var cAnimation = 1;                             // Current frame
var gadgetTimeout;                              // Frame timer object
var animationTemp1;                             // Temp storage slot 1 that an animation can use between frames
var animationTemp2;                             // Temp storage slot 2
var animationTemp3;                             // Temp storage slot 3
var animationTemp4;                             // Temp storage slot 4
var animationTemp5;                             // Temp storage slot 5
var animationTemp6;                             // Temp storage slot 6

var TICK_LEN = 75;                              // Length of a frame in milliseconds
var ANIMATION_LEN = 50;                         // Length in frames of an Animation

var dpiScaleFactor = 1.0;                       // Current system DPI scaling factor

// Called when the body is loaded
function loadMain()
{
    initGadget();
    animateGadget();
}

// Initialize necessary data/state
function initGadget()
{
    // Determine the system DPI scaling factor
    determineDPIScaleFactor();
    
    // Init body
    document.body.style.width = BKRND_WIDTH;
    document.body.style.height = BKRND_HEIGHT; 
    
    // Init background image
    var bkrnd = document.getElementById(BKRND_ID);
    bkrnd.src = BKRND_PATH;
    bkrnd.style.width = BKRND_WIDTH;
    bkrnd.style.height = BKRND_HEIGHT;

    // Init abstract text. Take into account the current system DPI when choosing the font size.
    oAbstractText = document.getElementById(BKRND_ID).addTextObject("", TXT_FONT, TXT_ABSTRACT_SIZE * dpiScaleFactor, TXT_COLOR, 0, 0);

    // Init demo image
    initImage();
    hideImage();
    
    // Init demo text. Take into account the current system DPI when choosing the font size.
    oText = document.getElementById(BKRND_ID).addTextObject(TXT_ANIMATION_VALUE, TXT_FONT, TXT_ANIMATION_SIZE * dpiScaleFactor, TXT_COLOR, 0, 0);
    hideText();
        
    // Init the navigation bar
    initNavBar();
    
    // Init the list of animation functions
    initAnimationFunctions();
}

// Determine the current system DPI
function determineDPIScaleFactor()
{
    // Calculate the horizontal scale factor that Internet Explorer is applying to determine the DPI
    dpiScaleFactor = screen.deviceXDPI / screen.logicalXDPI;
}

// Initialize the demo image.  Choose the appropriate image based on the current system DPI.
function initImage()
{
    var imagePath = "";
    if (dpiScaleFactor >= 1.5)
    {
        imagePath = IMG_PATH144;
    }
    else if (dpiScaleFactor >= 1.25)
    {
        imagePath = IMG_PATH120;
    }
    else
    {
        imagePath = IMG_PATH96;
    }
    
    oImage = document.getElementById(BKRND_ID).addImageObject(imagePath, 0, 0);
}

function updateAbstractText(newValue)
{
    oAbstractText.value = newValue;
    oAbstractText.width = 0;
    oAbstractText.height = 0;
    oAbstractText.left = TXT_ABSTRACT_LEFT;
    oAbstractText.top = TXT_ABSTRACT_TOP;
}

// Reset the demo image to its default settings
function resetImage()
{
    oImage.width = IMG_WIDTH * dpiScaleFactor;
    oImage.height = IMG_HEIGHT * dpiScaleFactor;
    oImage.left = IMG_LEFT * dpiScaleFactor;
    oImage.top = IMG_TOP * dpiScaleFactor;
    oImage.blur = 0;
    oImage.brightness = 0;
    oImage.opacity = 100;
    oImage.rotation = 0;
    oImage.softEdge = 0;
    oImage.addGlow('white', 0, 0);
    oImage.addShadow('white', 0, 0, 0, 0);
}

// Hide the demo image
function hideImage()
{
    oImage.opacity = 0;
}

// Reset the demo text to its default settings
function resetText()
{
    oText.left = TXT_ANIMATION_LEFT * dpiScaleFactor;
    oText.top = TXT_ANIMATION_TOP * dpiScaleFactor;
    oText.align = 0;
    oText.blur = 0;
    oText.brightness = 0;
    oText.color = TXT_COLOR;
    oText.font = TXT_FONT;
    oText.fontSize = TXT_ANIMATION_SIZE * dpiScaleFactor;
    oText.opacity = 100;
    oText.rotation = 0;
    oText.addGlow('white', 0, 0);
    oText.addShadow('white', 0, 0, 0, 0);
}

// Hide the demo text
function hideText()
{
    oText.opacity = 0;
}

// Initialize the Navigation Bar
function initNavBar()
{
    var bar = document.getElementById(NAVBAR_ID);
    var bkrnd = document.getElementById(BKRND_ID);
    bar.style.backgroundImage = "URL(" + NAVBAR_PATH + ")";
    hideNavBar();
    initNavButtons();
    bar_play.style.display="none";
    bar_pause.style.display="";
}

// Creates and injects the html for the buttons into the Navigation Bar
function initNavButtons()
{
    var bar = document.getElementById(NAVBAR_ID);
    var barHTML = "";
    var buttonHTML;
    var buttonList = NAVBAR_BUTTONS.split(" ");
    for (var i = 0; i < buttonList.length; i++)
    {
        var buttonName = buttonList[i];
        if (buttonName == "-")
        {
            buttonHTML = "";
        }
        else
        {
            buttonHTML = "<a id='link_" + buttonName + "' href='javascript:void(0);' onClick='this.blur();'>"
                + "<img border=0 id='bar_" + buttonName + "' vspace='3' hspace='3' src='images/"+buttonName+"_rest.png' "
                + "onMouseOver='src=\"images/" + buttonName + "_hov.png\"' onMouseOut='src=\"images/" + buttonName + "_rest.png\"' "
                + "onMouseDown='src=\"images/" + buttonName + "_down.png\"' onMouseUp='src=\"images/" + buttonName + "_hov.png\";"
                + "onAction(\"" + buttonName + "\");' \>" + "</a>";
        }
        barHTML += buttonHTML;
    }
    bar.innerHTML = barHTML;
}

// Show the Navigation Bar
function showNavBar()
{
    if(event.fromElement)
    {
        return;
    }
    var bar = document.getElementById(NAVBAR_ID);
    bar.style.width = NAVBAR_WIDTH;
    bar.style.height = NAVBAR_HEIGHT;
    bar.style.left = NAVBAR_LEFT;
    bar.style.top = NAVBAR_TOP;
}

// Hide the Navigation Bar
function hideNavBar()
{
    if(event.toElement)
    {
        return;
    }
    var bar = document.getElementById(NAVBAR_ID);
    bar.style.top = 0;
    bar.style.left = parseInt(document.body.style.width) + 1;
}

// Called when a Navigation Bar action occurs, like when the user clicks on a button
function onAction(action)
{
    if (event.button == 2 || event.button == 3)
    {
        return false
    }
    else
    {
        switch(action)
        {
            case "prev":
                clearTimeout(gadgetTimeout);
                if (bar_play.style.display == "")
                {
                    bar_play.style.display="none";
                    bar_pause.style.display="";
                }
                cAnimation = 1;
                cFunction--;
                fPlaying = true;
                gadgetTimeout = setTimeout("animateGadget()", TICK_LEN);
                break;
            case "play":
                clearTimeout(gadgetTimeout);
                bar_play.style.display="none";
                bar_pause.style.display="";
                fPlaying = true;
                gadgetTimeout = setTimeout("animateGadget()", TICK_LEN);
                break;
            case "pause":
                clearTimeout(gadgetTimeout);
                bar_play.style.display="";
                bar_pause.style.display="none";
                fPlaying = false ;
                break; 
            case "next":
                clearTimeout(gadgetTimeout);
                if (bar_play.style.display == "")
                {
                    bar_play.style.display="none";
                    bar_pause.style.display="";
                }
                cAnimation = 1;
                cFunction++;
                fPlaying = true;
                gadgetTimeout = setTimeout("animateGadget()", TICK_LEN);
                break;
       }
    }
}

// Handler for a key press
function keyNavigate()
{
    switch (event.keyCode)
    {
        // Right Arrow 
        case 37:    
            onAction("prev");
            break;
        // Left Arrow 
        case 39:
            onAction("next");
            break;
        // Space
        case 32:
        // Enter
        case 13:
            if(event.srcElement.id == "link_restart")
            {
                onAction("restart");
            }
            else if(event.srcElement.id == "link_next")
            {
                onAction("next");
            }
            else if(event.srcElement.id == "link_prev")
            {
                onAction("prev");
            }
            else
            {
                if(fPlaying)
                {
                    onAction("pause");
                    fPlaying = false;
                }
                else
                {
                    onAction("play");
                    fPlaying = true;
                }       
            }
            break;
        // Tab 
        case 9:
            showNavBar();
            break;
    }
}

// Animation Class and Functions
function Animation(f, t)
{
    this.func = f;
    this.text = t;
}

// Create the list of animations to perform
function initAnimationFunctions()
{
    //gImage Properties
    animationFuncs.push(new Animation(animate_gImage_p_blur, "g:image\nblur\n0 > 20 > 0"));
    animationFuncs.push(new Animation(animate_gImage_p_brightness, "g:image\nbrightness\n0% > 100% > 0%"));
    animationFuncs.push(new Animation(animate_gImage_p_heightwidth, "g:image\nheight, width\n100% > 50% > 150%"));
    animationFuncs.push(new Animation(animate_gImage_p_lefttop, "g:image\nleft, top\n(10,85) > (50,125)"));
    animationFuncs.push(new Animation(animate_gImage_p_opacity, "g:image\nopacity\n100% > 0% > 100%"));
    animationFuncs.push(new Animation(animate_gImage_p_rotation, "g:image\nrotation\n0 > 45 > -45 > 0"));
    animationFuncs.push(new Animation(animate_gImage_p_softEdge, "g:image\nsoftEdge\n0 > 13 > 0"));

    //gImage Methods
    animationFuncs.push(new Animation(animate_gImage_m_addGlow, "g:image\naddGlow()\n0 > 10 > 0 (x3)"));
    animationFuncs.push(new Animation(animate_gImage_m_addShadow, "g:image\naddShadow()\n0 > 30 > 0 (x2)"));

    //gText Properties
    animationFuncs.push(new Animation(animate_gText_p_blur, "g:text\nblur\n0 > 10 > 0"));
    animationFuncs.push(new Animation(animate_gText_p_brightness, "g:text\nbrightness\n0% > 100% > 0%"));
    animationFuncs.push(new Animation(animate_gText_p_color, "g:text\ncolor\n#000000 > #ffffff"));
    animationFuncs.push(new Animation(animate_gText_p_lefttop, "g:text\nleft, top\n(5,85) > (25,105)"));
    animationFuncs.push(new Animation(animate_gText_p_opacity, "g:text\nopacity\n100% > 0% > 100%"));

    //gText Methods
    animationFuncs.push(new Animation(animate_gText_m_addGlow, "g:text\naddGlow()\n0 > 10 > 0 (x3)"));
    animationFuncs.push(new Animation(animate_gText_m_addShadow, "g:text\naddShadow()\n0 > 30 > 0 (x2)"));    
}

// Animate the gadget
function animateGadget()
{
    clearTimeout(gadgetTimeout);
    
    if (cFunction >= animationFuncs.length)
    {
        cFunction = 0;
    }
    else if (cFunction < 0)
    {
        cFunction = animationFuncs.length - 1;
    }
    
    if (cAnimation == 1)
    {
        hideImage();
        hideText();
        updateAbstractText(animationFuncs[cFunction].text, TXT_ABSTRACT_LEFT, TXT_ABSTRACT_TOP);
    }
    if (cAnimation <= ANIMATION_LEN)
    {
        animationFuncs[cFunction].func();   
        gadgetTimeout = setTimeout("animateGadget()", TICK_LEN);
        cAnimation++;
    }
    else
    {
        cFunction++;
        cAnimation = 1;
                
        gadgetTimeout = setTimeout("animateGadget()", TICK_LEN);
    }
}

function animate_gImage_p_blur()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        animationTemp1 = 0.0;   // gImage.blur;
        oImage.blur = animationTemp1;
        animationTemp2 = 40.0 / (ANIMATION_LEN - 6);  // Calculate blur per tick to change. No blur at the end for a few ticks.
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 >= 20.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        if (animationTemp1 < 0.0)
        {
            animationTemp1 = 0.0;
        }
        oImage.blur = animationTemp1;
    }
}

function animate_gImage_p_brightness()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        animationTemp1 = 0.0;   // gImage.brightness;
        oImage.brightness = animationTemp1;
        animationTemp2 = 2.0 / (ANIMATION_LEN - 6);  // Calculate brightness per tick to change. Normal brightness at the end for a few ticks.
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 >= 1.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        if (animationTemp1 < 0.0)
        {
            animationTemp1 = 0.0;
        }
        oImage.brightness = animationTemp1;
    }
}

function animate_gImage_p_heightwidth()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        animationTemp1 = oImage.width;  // gImage.width
        animationTemp2 = oImage.height; // gImage.height
        animationTemp3 = (oImage.width * 1.5) / ANIMATION_LEN;  // Calculate pixels per tick to change width by
        animationTemp4 = (oImage.height * 1.5) / ANIMATION_LEN; // Calculate pixels per tick to change height by
        animationTemp5 = false; // Count up?
        animationTemp6 = oImage.width;  // Original image width
    }
    else
    {
        if (animationTemp1 <= (animationTemp6 / 2.0))
        {
            animationTemp5 = true;
        }
        if (animationTemp5)
        {
            animationTemp1 += animationTemp3;
            animationTemp2 += animationTemp4;
        }
        else
        {
            animationTemp1 -= animationTemp3;
            animationTemp2 -= animationTemp4;
        }
        oImage.width = animationTemp1;
        oImage.height = animationTemp2;
    }
}

function animate_gImage_p_lefttop()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        oImage.left = animationTemp1 = 15.0 * dpiScaleFactor;   // gImage.left
        oImage.top = animationTemp2 = 85.0 * dpiScaleFactor;    // gImage.top
        animationTemp3 = (40.0 * dpiScaleFactor) / ANIMATION_LEN;   // Calculate pixels per tick to shift right
        animationTemp4 = (40.0 * dpiScaleFactor) / ANIMATION_LEN;   // Calculate pixels per tick to shift down
    }
    else
    {
        animationTemp1 += animationTemp3;
        oImage.left = animationTemp1;
        animationTemp2 += animationTemp4;
        oImage.top = animationTemp2;
    }
}

function animate_gImage_p_opacity()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        animationTemp1 = 100.0;   // gImage.opacity;
        oImage.opacity = animationTemp1;
        animationTemp2 = 200.0 / (ANIMATION_LEN - 6);  // Calculate brightness per tick to change. Completely opaque at the end for a few ticks.
        animationTemp3 = false; // Count up?
    }
    else
    {
        if (animationTemp1 <= 0.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 += animationTemp2 : animationTemp1 -= animationTemp2;
        if (animationTemp1 > 100.0)
        {
            animationTemp1 = 100.0;
        }
        oImage.opacity = animationTemp1;
    }
}

function animate_gImage_p_rotation()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        animationTemp1 = 0.0;   // gImage.rotation
        oImage.rotation = animationTemp1;
        animationTemp2 = 180.0 / ANIMATION_LEN;    // Calculate degrees per tick to rotate
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 >= 45.0)
        {
            animationTemp3 = true;
        }
        else if (animationTemp1 <= -45.0)
        {
            animationTemp3 = false;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        oImage.rotation = animationTemp1;
    }
}

function animate_gImage_p_softEdge()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        animationTemp1 = 0.0;   // gImage.softEdge;
        oImage.softEdge = animationTemp1;
        animationTemp2 = (26.0 * dpiScaleFactor) / (ANIMATION_LEN - 6);  // Calculate soft edge per tick to change. No soft edge at the end for a few ticks.
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 >= (13.0 * dpiScaleFactor))
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        if (animationTemp1 < 0.0)
        {
            animationTemp1 = 0.0;
        }
        oImage.softEdge = animationTemp1;
    }
}

function animate_gImage_m_addGlow()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        animationTemp1 = 0.0;  // Glow radius
        animationTemp2 = 60.0 / ANIMATION_LEN;  // Calculate soft edge per tick to change
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 <= 0.0)
        {
            animationTemp3 = false;
            animationTemp4 = "Color(255, " + Math.floor(Math.random() * 255) + ", " + Math.floor(Math.random() * 255) + ", " + Math.floor(Math.random() * 255) + ")";
        }
        if (animationTemp1 > 10.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        oImage.addGlow(animationTemp4, animationTemp1, 100);
    }
}

function animate_gImage_m_addShadow()
{
    if (cAnimation == 1)
    {
        hideText();
        resetImage();
        animationTemp1 = 0.0;  // Shadow radius
        animationTemp2 = 120.0 / ANIMATION_LEN;  // Calculate shadow per tick to change
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 <= 0.0)
        {
            animationTemp3 = false;
            animationTemp4 = "Color(255, " + Math.floor(Math.random() * 255) + ", " + Math.floor(Math.random() * 255) + ", " + Math.floor(Math.random() * 255) + ")";
        }
        if (animationTemp1 >= 30.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        oImage.addShadow(animationTemp4, animationTemp1, 100, 0, 0);
    }
}

function animate_gText_p_blur()
{
    if (cAnimation == 1)
    {
        hideImage();
        resetText();
        animationTemp1 = 0.0;   // gText.blur;
        oText.blur = animationTemp1;
        animationTemp2 = 20.0 / (ANIMATION_LEN - 6);  // Calculate blur per tick to change. No blur at the end for a few ticks.
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 >= 10.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        if (animationTemp1 < 0.0)
        {
            animationTemp1 = 0.0;
        }
        oText.blur = animationTemp1;
    }
}

function animate_gText_p_brightness()
{
    if (cAnimation == 1)
    {
        hideImage();
        resetText();
        oText.color = "red";
        animationTemp1 = 0.0;   // oImage.brightness;
        oText.brightness = animationTemp1;
        animationTemp2 = 2.0 / (ANIMATION_LEN - 6);  // Calculate brightness per tick to change. Normal brightness at the end for a few ticks.
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 >= 1.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        if (animationTemp1 < 0.0)
        {
            animationTemp1 = 0.0;
        }
        oText.brightness = animationTemp1;
    }
}

function animate_gText_p_color()
{
    if (cAnimation == 1)
    {
        hideImage();
        resetText();
        animationTemp1 = 0.0;   // R
        animationTemp2 = 0.0;   // G
        animationTemp3 = 0.0;   // B
        animationTemp4 = 765.0 / ANIMATION_LEN;  // Calculate gradient per tick to change cur color by
        animationTemp5 = 1; // Current channel we're animating on (R, G or B)
        oText.color = "Color(255, " + Math.floor(animationTemp1) + ", " + Math.floor(animationTemp2) + ", " + Math.floor(animationTemp3) + ")";
    }
    else
    {
        switch (animationTemp5)
        {
            case 1:
                if (animationTemp1 < 255.0)
                {
                    animationTemp1 += animationTemp4;
                    if (animationTemp1 <= 255.0)
                    {
                        break;
                    }
                    else
                    {
                        animationTemp1 = 255.0;
                    }
                }
                else
                {
                    animationTemp5 = 2;
                }
            case 2:
                if (animationTemp2 < 255.0)
                {
                    animationTemp2 += animationTemp4;
                    if (animationTemp2 <= 255.0)
                    {
                        break;
                    }
                    else
                    {
                        animationTemp2 = 255.0;
                    }
                }
                else
                {
                    animationTemp5 = 3;
                }
            case 3:
                if (animationTemp3 < 255.0)
                {
                    animationTemp3 += animationTemp4;
                }
                if (animationTemp3 > 255.0)
                {
                    animationTemp3 = 255.0;
                }
                break;
        }
        oText.color = "Color(255, " + Math.floor(animationTemp1) + ", " + Math.floor(animationTemp2) + ", " + Math.floor(animationTemp3) + ")";
    }
}

function animate_gText_p_lefttop()
{
    if (cAnimation == 1)
    {
        hideImage();
        resetText();
        animationTemp1 = 5.0 * dpiScaleFactor;  // gText.left
        animationTemp2 = 85.0 * dpiScaleFactor; // gText.top
        oText.left = animationTemp1;
        oText.top = animationTemp2;
        animationTemp3 = (20.0 * dpiScaleFactor) / ANIMATION_LEN;   // Calculate pixels per tick to shift right
        animationTemp4 = (20.0 * dpiScaleFactor) / ANIMATION_LEN;   // Calculate pixels per tick to shift down
    }
    else
    {
        animationTemp1 += animationTemp3;
        animationTemp2 += animationTemp4;
        oText.left = animationTemp1;
        oText.top = animationTemp2;
    }
}

function animate_gText_p_opacity()
{
    if (cAnimation == 1)
    {
        hideImage();
        resetText();
        animationTemp1 = 100.0;   // gText.opacity;
        oText.opacity = animationTemp1;
        animationTemp2 = 200.0 / (ANIMATION_LEN - 6);  // Calculate brightness per tick to change. Completely opaque at the end for a few ticks.
        animationTemp3 = false; // Count up?
    }
    else
    {
        if (animationTemp1 <= 0.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 += animationTemp2 : animationTemp1 -= animationTemp2;
        if (animationTemp1 > 100.0)
        {
            animationTemp1 = 100.0;
        }
        oText.opacity = animationTemp1;
    }
}

function animate_gText_m_addGlow()
{
    if (cAnimation == 1)
    {
        hideImage();
        resetText();
        animationTemp1 = 0.0;  // Glow radius
        animationTemp2 = 60.0 / ANIMATION_LEN;  // Calculate soft edge per tick to change
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 <= 0.0)
        {
            animationTemp3 = false;
            animationTemp4 = "Color(255, " + Math.floor(Math.random() * 255) + ", " + Math.floor(Math.random() * 255) + ", " + Math.floor(Math.random() * 255) + ")";
        }
        if (animationTemp1 > 10.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        oText.addGlow(animationTemp4, animationTemp1, 100);
    }
}

function animate_gText_m_addShadow()
{
    if (cAnimation == 1)
    {
        hideImage();
        resetText();
        animationTemp1 = 0.0;  // Shadow radius
        animationTemp2 = 120.0 / ANIMATION_LEN;  // Calculate shadow per tick to change
        animationTemp3 = false; // Count down?
    }
    else
    {
        if (animationTemp1 <= 0.0)
        {
            animationTemp3 = false;
            animationTemp4 = "Color(255, " + Math.floor(Math.random() * 255) + ", " + Math.floor(Math.random() * 255) + ", " + Math.floor(Math.random() * 255) + ")";
        }
        if (animationTemp1 >= 30.0)
        {
            animationTemp3 = true;
        }
        animationTemp3 ? animationTemp1 -= animationTemp2 : animationTemp1 += animationTemp2;
        oText.addShadow(animationTemp4, animationTemp1, 100, 0, 0);
    }
}
