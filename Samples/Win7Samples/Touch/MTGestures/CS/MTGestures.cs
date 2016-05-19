// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// The main form (touchable). All gestures are handled through this form.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Security.Permissions;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Microsoft.Samples.Touch.MTGestures
{
    public partial class MTGesturesForm : Form
    {
        // Private variables used to maintain the state of gestures
        private DrawingObject _dwo = new DrawingObject();
        private Point _ptFirst = new Point();
        private Point _ptSecond = new Point();
        private int _iArguments = 0;

        // One of the fields in GESTUREINFO structure is type of Int64 (8 bytes).
        // The relevant gesture information is stored in lower 4 bytes. This
        // bit mask is used to get 4 lower bytes from this argument.
        private const Int64 ULL_ARGUMENTS_BIT_MASK = 0x00000000FFFFFFFF;

        //-----------------------------------------------------------------------
        // Multitouch/Touch glue (from winuser.h file)
        // Since the managed layer between C# and WinAPI functions does not 
        // exist at the moment for multi-touch related functions this part of 
        // code is required to replicate definitions from winuser.h file.
        //-----------------------------------------------------------------------
        // Touch event window message constants [winuser.h]
        private const int WM_GESTURENOTIFY = 0x011A;
        private const int WM_GESTURE = 0x0119;

        private const int GC_ALLGESTURES = 0x00000001;

        // Gesture IDs 
        private const int GID_BEGIN = 1;
        private const int GID_END = 2;
        private const int GID_ZOOM = 3;
        private const int GID_PAN = 4;
        private const int GID_ROTATE = 5;
        private const int GID_TWOFINGERTAP = 6;
        private const int GID_PRESSANDTAP = 7;

        // Gesture flags - GESTUREINFO.dwFlags
        private const int GF_BEGIN   = 0x00000001;
        private const int GF_INERTIA = 0x00000002;
        private const int GF_END     = 0x00000004;
        
        //
        // Gesture configuration structure
        //   - Used in SetGestureConfig and GetGestureConfig
        //   - Note that any setting not included in either GESTURECONFIG.dwWant
        //     or GESTURECONFIG.dwBlock will use the parent window's preferences
        //     or system defaults.
        //
        // Touch API defined structures [winuser.h]
        [StructLayout(LayoutKind.Sequential)]
        private struct GESTURECONFIG
        {
            public int dwID;    // gesture ID
            public int dwWant;  // settings related to gesture ID that are to be
                                // turned on
            public int dwBlock; // settings related to gesture ID that are to be
                                // turned off
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct POINTS
        {
            public short x;
            public short y;
        }

        //
        // Gesture information structure
        //   - Pass the HGESTUREINFO received in the WM_GESTURE message lParam 
        //     into the GetGestureInfo function to retrieve this information.
        //   - If cbExtraArgs is non-zero, pass the HGESTUREINFO received in 
        //     the WM_GESTURE message lParam into the GetGestureExtraArgs 
        //     function to retrieve extended argument information.
        //
        [StructLayout(LayoutKind.Sequential)]
        private struct GESTUREINFO 
        {
            public int cbSize;           // size, in bytes, of this structure
                                         // (including variable length Args 
                                         // field)
            public int dwFlags;          // see GF_* flags
            public int dwID;             // gesture ID, see GID_* defines
            public IntPtr hwndTarget;    // handle to window targeted by this 
                                         // gesture
            [MarshalAs(UnmanagedType.Struct)]
            internal POINTS ptsLocation; // current location of this gesture
            public int dwInstanceID;     // internally used
            public int dwSequenceID;     // internally used
            public Int64 ullArguments;   // arguments for gestures whose 
                                         // arguments fit in 8 BYTES
            public int cbExtraArgs;      // size, in bytes, of extra arguments, 
                                         // if any, that accompany this gesture
        } 

        // Currently touch/multitouch access is done through unmanaged code
        // We must p/invoke into user32 [winuser.h]
        [DllImport("user32")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetGestureConfig(IntPtr hWnd,int dwReserved,int cIDs,ref GESTURECONFIG pGestureConfig,int cbSize);

        [DllImport("user32")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool GetGestureInfo(IntPtr hGestureInfo, ref GESTUREINFO pGestureInfo);

        // size of GESTURECONFIG structure
        private int _gestureConfigSize;
        // size of GESTUREINFO structure
        private int _gestureInfoSize;

        [SecurityPermission(SecurityAction.Demand)]
        private void SetupStructSizes()
        {
            // Both GetGestureCommandInfo and GetTouchInputInfo need to be
            // passed the size of the structure they will be filling
            // we get the sizes upfront so they can be used later.
            _gestureConfigSize = Marshal.SizeOf(new GESTURECONFIG());
            _gestureInfoSize = Marshal.SizeOf(new GESTUREINFO());
        }


        //-------------------------------------------------------------
        // Since there is no managed layer at the moment that supports
        // event handlers for WM_GESTURENOTIFY and WM_GESTURE
        // messages we have to override WndProc function
        // 
        // in 
        //   m - Message object
        //-------------------------------------------------------------
        [PermissionSet(SecurityAction.Demand, Name = "FullTrust")]
        protected override void WndProc(ref Message m)
        {
            bool handled;
            handled = false;

            switch (m.Msg)
            {
                case WM_GESTURENOTIFY:
                    {
                        // This is the right place to define the list of gestures
                        // that this application will support. By populating 
                        // GESTURECONFIG structure and calling SetGestureConfig 
                        // function. We can choose gestures that we want to 
                        // handle in our application. In this app we decide to 
                        // handle all gestures.
                        GESTURECONFIG gc = new GESTURECONFIG();
                        gc.dwID = 0;                // gesture ID
                        gc.dwWant = GC_ALLGESTURES; // settings related to gesture
                                                    // ID that are to be turned on
                        gc.dwBlock = 0; // settings related to gesture ID that are
                                        // to be     

                        // We must p/invoke into user32 [winuser.h]
                        bool bResult = SetGestureConfig(
                            Handle, // window for which configuration is specified
                            0,      // reserved, must be 0
                            1,      // count of GESTURECONFIG structures
                            ref gc, // array of GESTURECONFIG structures, dwIDs 
                                    // will be processed in the order specified 
                                    // and repeated occurances will overwrite 
                                    // previous ones
                            _gestureConfigSize // sizeof(GESTURECONFIG)
                        );

                        if (!bResult)
                        {
                            throw new Exception("Error in execution of SetGestureConfig");
                        }
                    }
                    handled = true;
                    break;

                case WM_GESTURE:
                    // The gesture processing code is implemented in 
                    // the DecodeGesture method
                    handled = DecodeGesture(ref m);
                    break;

                default:
                    handled = false;
                    break;
            }

            // Filter message back up to parents.
            base.WndProc(ref m);

            if (handled)
            {
                // Acknowledge event if handled.
                try
                {
                    m.Result = new System.IntPtr(1);
                }
                catch (Exception excep)
                {
                    Debug.Print("Could not allocate result ptr");
                    Debug.Print(excep.ToString()); 
                }
            }
        }

        // Taken from GCI_ROTATE_ANGLE_FROM_ARGUMENT.
        // Converts from "binary radians" to traditional radians.
        static protected double ArgToRadians(Int64 arg)
        {
            return ((((double)(arg) / 65535.0) * 4.0 * 3.14159265) - 2.0 * 3.14159265);
        }


        // Handler of gestures
        //in:
        //  m - Message object
        private bool DecodeGesture(ref Message m)
        {
            GESTUREINFO gi;

            try
            {
                gi = new GESTUREINFO();
            }
            catch (Exception excep)
            {
                Debug.Print("Could not allocate resources to decode gesture");
                Debug.Print(excep.ToString());

                return false;
            }

            gi.cbSize = _gestureInfoSize;

            // Load the gesture information.
            // We must p/invoke into user32 [winuser.h]
            if(!GetGestureInfo(m.LParam,ref gi))
            {
                return false;
            }

            switch (gi.dwID)
            {
                case GID_BEGIN:
                case GID_END:
                    break;

                case GID_ZOOM:
                    switch (gi.dwFlags)
                    {
                        case GF_BEGIN:
                            _iArguments = (int)(gi.ullArguments & ULL_ARGUMENTS_BIT_MASK);
                            _ptFirst.X = gi.ptsLocation.x;
                            _ptFirst.Y = gi.ptsLocation.y;
                            _ptFirst = PointToClient(_ptFirst);
                            break;

                        default:
                            // We read here the second point of the gesture. This
                            // is middle point between fingers in this new 
                            // position.
                            _ptSecond.X = gi.ptsLocation.x;
                            _ptSecond.Y = gi.ptsLocation.y;
                            _ptSecond = PointToClient(_ptSecond);
                            {
                                // We have to calculate zoom center point 
                                Point ptZoomCenter = new Point((_ptFirst.X + _ptSecond.X) / 2,
                                                            (_ptFirst.Y + _ptSecond.Y) / 2);

                                // The zoom factor is the ratio of the new
                                // and the old distance. The new distance 
                                // between two fingers is stored in 
                                // gi.ullArguments (lower 4 bytes) and the old 
                                // distance is stored in _iArguments.
                                double k =  (double)(gi.ullArguments & ULL_ARGUMENTS_BIT_MASK) / 
                                            (double)(_iArguments);

                                // Now we process zooming in/out of the object
                                _dwo.Zoom(k, ptZoomCenter.X, ptZoomCenter.Y);

                                Invalidate();
                            }

                            // Now we have to store new information as a starting
                            // information for the next step in this gesture.
                            _ptFirst = _ptSecond;
                            _iArguments = (int)(gi.ullArguments & ULL_ARGUMENTS_BIT_MASK);
                            break;
                    }
                    break;

                case GID_PAN:
                    switch (gi.dwFlags)
                    {
                        case GF_BEGIN:
                            _ptFirst.X = gi.ptsLocation.x;
                            _ptFirst.Y = gi.ptsLocation.y;
                            _ptFirst = PointToClient(_ptFirst);
                            break;

                        default:
                            // We read the second point of this gesture. It is a
                            // middle point between fingers in this new position
                            _ptSecond.X = gi.ptsLocation.x;
                            _ptSecond.Y = gi.ptsLocation.y;
                            _ptSecond = PointToClient(_ptSecond);

                            // We apply move operation of the object
                            _dwo.Move(_ptSecond.X - _ptFirst.X, _ptSecond.Y - _ptFirst.Y);

                            Invalidate();

                            // We have to copy second point into first one to
                            // prepare for the next step of this gesture.
                            _ptFirst = _ptSecond;
                            break;
                    }
                    break;

                case GID_ROTATE:
                    switch (gi.dwFlags)
                    {
                        case GF_BEGIN:
                            _iArguments = 0;
                            break;

                        default:
                            _ptFirst.X = gi.ptsLocation.x;
                            _ptFirst.Y = gi.ptsLocation.y;
                            _ptFirst = PointToClient(_ptFirst);

                            // Gesture handler returns cumulative rotation angle. However we
                            // have to pass the delta angle to our function responsible 
                            // to process the rotation gesture.
                            _dwo.Rotate(
                                ArgToRadians(gi.ullArguments & ULL_ARGUMENTS_BIT_MASK) 
                                - ArgToRadians(_iArguments),
                                _ptFirst.X, _ptFirst.Y
                            );

                            Invalidate();

                            _iArguments = (int)(gi.ullArguments & ULL_ARGUMENTS_BIT_MASK);
                            break;
                    }
                    break;

                case GID_TWOFINGERTAP:
                    // Toggle drawing of diagonals
                    _dwo.ToggleDrawDiagonals();
                    Invalidate();
                    break;

                case GID_PRESSANDTAP:
                    if (gi.dwFlags == GF_BEGIN)
                    {
                        // Shift drawing color
                        _dwo.ShiftColor();
                        Invalidate();
                    }
                    break;
            }

            return true;
        }

        // Class constructor
        public MTGesturesForm()
        {
            InitializeComponent();
            SetupStructSizes();
        }

        // This is event handler for WM_SIZE message
        // in:
        //      sender  - sender object
        //      e       - event arguments
        private void OnSizeChanged(object sender, EventArgs e)
        {
            // resize rectangle and place it in the middle of the new client area
            _dwo.ResetObject(ClientRectangle);
            Invalidate(); // triggers OnPaint event handler
        }

        // This is event handler for WM_PAINT message
        // in:
        //      sender  - sender object
        //      e       - event arguments
        private void OnPaint(object sender, PaintEventArgs e)
        {
            // Full redraw of the rectangle
            _dwo.Paint(e.Graphics);
        }

        // This is event handler for loading of this form
        // in:
        //      sender  - sender object
        //      e       - event arguments
        private void OnLoad(object sender, EventArgs e)
        {
            // resize rectangle and place it in the middle of the new client area
            _dwo.ResetObject(ClientRectangle);
        }

    }

 }