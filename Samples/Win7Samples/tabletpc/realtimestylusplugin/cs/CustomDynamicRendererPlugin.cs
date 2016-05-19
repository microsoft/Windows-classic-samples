//-------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: CustomDynamicRendererPlugin.cs
//  Real time stylus plugin that demonstrates custom
//  dynamic rendering.
//
//  This class implements the real time stylus plugin
//  interface to receive pen input notifications.  It then
//  handles the Packets notification to draw a small circle
//  around each new packet point. 
//
//  The features used are: IRealTimeStylusPlugin
//
//--------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Diagnostics;

// The Microsoft Tablet PC namespaces
using Microsoft.Ink;
using Microsoft.StylusInput;
using Microsoft.StylusInput.PluginData;

namespace RealTimeStylusPlugins
{
    /// <summary>
    /// A real time stylus plugin that demonstrates
    /// custom dynamic rendering.  
    /// </summary>
    public class CustomDynamicRendererPlugin:IStylusSyncPlugin
    {
        // Declare the graphics object used for dynamic rendering
        private Graphics myGraphics;

        /// <summary>
        /// Constructor for this plugin
        /// </summary>
        /// <param name="g">The graphics object used for dynamic rendering.</param>
        public CustomDynamicRendererPlugin(Graphics g)
        {
            myGraphics = g;
        }


        /// <summary>
        /// Occurs when the stylus moves on the digitizer surface. 
        /// Use this notification to draw a small circle around each
        /// new packet received.
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
        public void Packets(RealTimeStylus sender,  PacketsData data)
        {           
            // For each new packet received, extract the x,y data
            // and draw a small circle around the result.
            for (int i = 0; i < data.Count; i += data.PacketPropertyCount)
            {
                // Packet data always has x followed by y followed by the rest
                Point point = new Point(data[i], data[i+1]);

                // Since the packet data is in Ink Space coordinates, we need to convert to Pixels...
                point.X = (int)Math.Round((float)point.X * (float)myGraphics.DpiX/2540.0F);
                point.Y = (int)Math.Round((float)point.Y * (float)myGraphics.DpiY/2540.0F);

                // Draw a circle corresponding to the packet
                myGraphics.DrawEllipse(Pens.Green, point.X - 2, point.Y - 2, 4, 4);
            }
        }

        /// <summary>
        /// Called when the current plugin or the ones previous in the list
        /// threw an exception.
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
	public void Error(RealTimeStylus sender, ErrorData data)
        {
             Debug.Assert(false, null, "An error occurred.  DataId=" + data.DataId + ", " + "Exception=" + data.InnerException);
        }

        /// <summary>
        /// Defines the types of notifications the plugin is interested in.
        /// </summary>
        public DataInterestMask DataInterest
        {
            get
            {
                return DataInterestMask.Packets | DataInterestMask.Error;
            }
        }

        // The remaining interface methods are not used in this sample application.
        public void RealTimeStylusDisabled(RealTimeStylus sender, RealTimeStylusDisabledData data) {}
        public void RealTimeStylusEnabled(RealTimeStylus sender, RealTimeStylusEnabledData data){}
        public void StylusOutOfRange(RealTimeStylus sender, StylusOutOfRangeData data) {}
        public void StylusInRange(RealTimeStylus sender, StylusInRangeData data) {}
        public void StylusDown(RealTimeStylus sender, StylusDownData data) {}
        public void StylusUp(RealTimeStylus sender, StylusUpData data) {}
        public void StylusButtonDown(RealTimeStylus sender, StylusButtonDownData data) {}
        public void StylusButtonUp(RealTimeStylus sender, StylusButtonUpData data) {}
        public void CustomStylusDataAdded(RealTimeStylus sender, CustomStylusData data){}
        public void SystemGesture(RealTimeStylus sender, SystemGestureData data){}
        public void InAirPackets(RealTimeStylus sender, InAirPacketsData data){}
        public void TabletAdded(RealTimeStylus sender, TabletAddedData data){}
        public void TabletRemoved(RealTimeStylus sender, TabletRemovedData data) {}
    }
}
