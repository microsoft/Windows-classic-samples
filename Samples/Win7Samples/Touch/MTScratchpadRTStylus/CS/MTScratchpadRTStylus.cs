// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// MTScratchpadRTStylus application.
// Description:
//  Inside the application window, user can draw using multiple fingers
//  at the same time. The trace of each finger is drawn using different
//  color. The primary finger trace is always drawn in black, and the
//  remaining traces are drawn by rotating through the following colors:
//  red, blue, green, magenta, cyan and yellow.
//
// Purpose:
//  This sample demonstrates handling of the multi-touch input inside
//  a C# application using Real Time Stylus inking API:
//  - Setting up minimal RTS object configuration that allows the window
//    to be inking-enabled (this is not multi-touch specific);
//    Construction and initialization of RealTimeStylus object and
//    DynamicRenderer object.
//  - Registering RTS object for multi-touch;
//  - Deriving and initializing synchronous RTS plugin that receives
//    inking notifications that are multi-touch aware.
//  - Handling multi-touch aware pen-down/up/move notifications: changing
//    stroke color on pen-down.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Runtime.InteropServices;

// Microsoft Tablet PC namespaces
using Microsoft.StylusInput;
using Microsoft.StylusInput.PluginData;

namespace Microsoft.Samples.Touch.MTScratchpadRTStylus
{
    public partial class MTScratchpadRTStylusForm : Form
    {
        // MTScratchpadRTStylusForm constructor
        public MTScratchpadRTStylusForm()
        {
            InitializeComponent();

            // Setup event handlers
            Load += new System.EventHandler(this.OnLoadHandler);
            Paint += new PaintEventHandler(this.OnPaintHandler);
        }

        // OnLoad window event handler
        // in:
        //      sender      object that has sent the event
        //      e           event arguments
        private void OnLoadHandler(Object sender, EventArgs e)
        {
            // Create RealTimeStylus object and enable it for multi-touch
            realTimeStylus = new RealTimeStylus(this);
            realTimeStylus.MultiTouchEnabled = true;

            // Create DynamicRenderer and event handler, and add them to the RTS object as synchronous plugins
            dynamicRenderer = new DynamicRenderer(this);
            eventHandler = new EventHandlerPlugIn(this.CreateGraphics(), dynamicRenderer);
            realTimeStylus.SyncPluginCollection.Add(eventHandler);
            realTimeStylus.SyncPluginCollection.Add(dynamicRenderer);

            // Enable RTS and DynamicRenderer object, and enable auto-redraw of the DynamicRenderer
            realTimeStylus.Enabled = true;
            dynamicRenderer.Enabled = true;
            dynamicRenderer.EnableDataCache = true;
        }

        // OnPaint window event handler
        // in:
        //      sender      object that has sent the event
        //      e           event arguments
        private void OnPaintHandler(object sender, PaintEventArgs e)
        {
            // Erase the background
            Brush brush = new SolidBrush(SystemColors.Window);
            e.Graphics.FillRectangle(brush, ClientRectangle);

            // Ask DynamicRenderer to redraw itself
            dynamicRenderer.Refresh();
        }

        // Attributes
        private RealTimeStylus realTimeStylus;      // RealTimeStylus object
        private DynamicRenderer dynamicRenderer;    // DynamicRenderer object
        private EventHandlerPlugIn eventHandler;    // Event handler object
    }

    // Notification receiver that changes pen color.
    public class EventHandlerPlugIn : IStylusSyncPlugin
    {
        // EventHandlerPlugIn constructor
        public EventHandlerPlugIn(Graphics graph, DynamicRenderer render)
        {
            graphics = graph;
            cntContacts = 0;
            touchColor = new TouchColor();
            dynamicRenderer = render;
        }

        // Pen-down notification handler.
        // Sets the color for the newly started stroke and increments finger-down counter.
        //      sender      RTS event sender object
        //      data        event arguments
        public void StylusDown(RealTimeStylus sender, StylusDownData data)
        {
            // Set new stroke color to the DrawingAttributes of the DynamicRenderer
            // If there are no fingers down, this is a primary contact
            dynamicRenderer.DrawingAttributes.Color = touchColor.GetColor(cntContacts == 0);

            ++cntContacts;  // Increment finger-down counter
        }

        // Pen-move notification handler
        // In this case, does nothing, but likely to be used in a more complex application.
        // RTS framework does stroke collection and rendering for us.
        //      sender      RTS event sender object
        //      data        event arguments
        public void Packets(RealTimeStylus sender, PacketsData data)
        {
        }

        // Pen-up notification handler.
        // Decrements finger-down counter.
        //      sender      RTS event sender object
        //      data        event arguments
        public void StylusUp(RealTimeStylus sender, StylusUpData data)
        {
            --cntContacts;  // Decrement finger-down counter
        }

        // Defines which handlers are called by the framework. We set the flags for pen-down, pen-up and pen-move.
        public DataInterestMask DataInterest
        {
            get
            {
                return DataInterestMask.StylusDown |
                       DataInterestMask.Packets |
                       DataInterestMask.StylusUp;
            }
        }

        // The remaining interface methods are not used in this sample application.
        public void RealTimeStylusDisabled(RealTimeStylus sender, RealTimeStylusDisabledData data) { }
        public void RealTimeStylusEnabled(RealTimeStylus sender, RealTimeStylusEnabledData data) { }
        public void StylusOutOfRange(RealTimeStylus sender, StylusOutOfRangeData data) { }
        public void StylusInRange(RealTimeStylus sender, StylusInRangeData data) { }
        public void StylusButtonDown(RealTimeStylus sender, StylusButtonDownData data) { }
        public void StylusButtonUp(RealTimeStylus sender, StylusButtonUpData data) { }
        public void CustomStylusDataAdded(RealTimeStylus sender, CustomStylusData data) { }
        public void SystemGesture(RealTimeStylus sender, SystemGestureData data) { }
        public void InAirPackets(RealTimeStylus sender, InAirPacketsData data) { }
        public void TabletAdded(RealTimeStylus sender, TabletAddedData data) { }
        public void TabletRemoved(RealTimeStylus sender, TabletRemovedData data) { }
        public void Error(RealTimeStylus sender, ErrorData data) { }

        // Attributes
        private Graphics graphics;          // graphics object of the window this plugin is attached to
        private int cntContacts;            // number of fingers currently in the contact with the touch digitizer
        private TouchColor touchColor;      // color generator
        private DynamicRenderer dynamicRenderer; // dynamic renderer
    }

    // Color generator: assigns a color to the new stroke.
    public class TouchColor
    {
        public TouchColor()
        {
        }

        // Returns color for the newly started stroke.
        // in:
        //      primary         flag, whether the contact is the primary contact
        // returns:
        //      color of the stroke
        public Color GetColor(bool primary)
        {
            if (primary)
            {
                // The primary contact is drawn in black.
                return Color.Black;
            }
            else
            {
                // Take current secondary color.
                Color color = secondaryColors[idx];

                // Move to the next color in the array.
                idx = (idx + 1) % secondaryColors.Length;

                return color;
            }
        }

        // Attributes
        static private Color[] secondaryColors =    // secondary colors
        {
            Color.Red,
            Color.LawnGreen,
            Color.Blue,
            Color.Cyan,
            Color.Magenta,
            Color.Yellow
        };
        private int idx = 0;                // rotating secondary color index
    }
}
