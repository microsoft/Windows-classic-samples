// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// MTGestures application
// Description:
//  This application demonstrate how to handle multi-touch gesture commands. This
//  application will initially draw rectangle in the middle of client area. By 
//  using gestures a user can manipulate the rectangle. The available 
//  commands are:
//      - rectangle stretch 
//          By putting two fingers on the screen and modifying distance between 
//          them by moving fingers in the opposite directions or towards each
//          other the user can zoom in/out this rectangle.
//      - panning
//          By touching the screen with two fingers and moving them in the same 
//          direction the user can move the rectangle. 
//      - rotate
//          By putting one finger in the center of the rotation and then rotating 
//          the other finger around it the user can rotate the rectangle
//      - two finger tap
//          By tapping the screen with two fingers the user can toggle drawing of 
//          the diagonals
//      - finger press and tap
//          This gesture involves movements of two fingers. It consists first of
//          putting one finger down. Then putting the second finger down and then
//          lifting it up. Finally the first finger is lifted up. This gesture 
//          will change the color of the rectangle.
//
// Purpose:
//  This sample demonstrates handling of the multi-touch gestures. 
//
// Program.cs : Defines the entry point for the application.

using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace Microsoft.Samples.Touch.MTGestures
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MTGesturesForm()); // renamed default Form1 to TouchableForm
        }
    }
}