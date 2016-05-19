//-------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: PacketFilterPlugin.cs
//  Real time stylus plugin that demonstrates packet modification.
//
//  This class implements the real time stylus plugin
//  interface to receive pen input notifications.  It then
//  handles the StylusDown and Packets notifications and 
//  constrains all packet data within a user-specified rectangle.
//
//  The features used are: IRealTimeStylusPlugin
//
//--------------------------------------------------------------------------

using System;
using System.Diagnostics;
using System.Drawing;

// The Microsoft Tablet PC namespaces
using Microsoft.Ink;
using Microsoft.StylusInput;
using Microsoft.StylusInput.PluginData;

namespace RealTimeStylusPlugins
{
    /// <summary>
    /// A real time stylus plugin that demonstrates
    /// packet modification.  
    /// </summary>
    public class PacketFilterPlugin:IStylusSyncPlugin
    {

        // Declare the rectangular area where packets will be restricted
        // (in ink space coordinates)
        private Rectangle rectangle = Rectangle.Empty;

        /// <summary>
        /// Constructor for this plugin
        /// </summary>
        /// <param name="g">
        /// The rectangle where packets will be restricted (in ink space coordinates).
        /// </param>
        public PacketFilterPlugin(Rectangle r)
        {
            rectangle = r;
        }

        /// <summary>
        /// Helper method that constrains packet data within
        /// a specified input rectangle.
        /// </summary>
        /// <param name="data">The data to modify.</param>
        private void ModifyPacketData(StylusDataBase data)
        {
            // For each packet in the packet data, check whether
            // its x,y values fall outside of the specified rectangle.  
            // If so, replace them with the nearest point that still
            // falls within the rectangle.
            for (int i = 0; i < data.Count ; i += data.PacketPropertyCount)
            {
                // packet data always has x followed by y followed by the rest
                int x = data[i];
                int y = data[i+1];

                // Constrain points to the input rectangle
                x = Math.Max(x, rectangle.Left);
                x = Math.Min(x, rectangle.Right);
                y = Math.Max(y, rectangle.Top);
                y = Math.Min(y, rectangle.Bottom);

                // If necessary, modify the x,y packet data
                if (x != data[i])
                {
                    data[i] = x;
                }
                if (y != data[i+1])
                {
                    data[i+1] = y;
                } 
            }
        }

        /// <summary>
        /// Occurs when the stylus touches the digitizer surface.
        /// Use this notificaton to constrain the packet data within 
        /// a specified rectangle.  
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
        public void StylusDown(RealTimeStylus sender,  StylusDownData data)
        {
            ModifyPacketData(data);
        }

        /// <summary>
        /// Occurs when the stylus moves on the digitizer surface. 
        /// Use this notification to constrain the packet data within a 
        /// specified rectangle.
        /// </summary>
        /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
        public void Packets(RealTimeStylus sender,  PacketsData data)
        {
            ModifyPacketData(data);
        }

        /// <summary>
        /// Occurs when the stylus leaves the digitizer surface.
        /// Use this notification to constrain the packet data within
        /// a specified rectangle.
        /// </summary>
         /// <param name="sender">The real time stylus associated with the notification</param>
        /// <param name="data">The notification data</param>
        public void StylusUp(RealTimeStylus sender,  StylusUpData data)
        {
            ModifyPacketData(data);
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
                return DataInterestMask.StylusDown |
                       DataInterestMask.Packets |
                       DataInterestMask.StylusUp |
                       DataInterestMask.Error;
            }
        }

        // The remaining interface methods are not used in this sample application.
        public void RealTimeStylusDisabled(RealTimeStylus sender, RealTimeStylusDisabledData data) {}
        public void RealTimeStylusEnabled(RealTimeStylus sender, RealTimeStylusEnabledData data){}
        public void StylusOutOfRange(RealTimeStylus sender, StylusOutOfRangeData data) {}
        public void StylusInRange(RealTimeStylus sender, StylusInRangeData data) {}
        public void StylusButtonDown(RealTimeStylus sender, StylusButtonDownData data) {}
        public void StylusButtonUp(RealTimeStylus sender, StylusButtonUpData data) {}
        public void CustomStylusDataAdded(RealTimeStylus sender, CustomStylusData data){}
        public void SystemGesture(RealTimeStylus sender, SystemGestureData data){}
        public void InAirPackets(RealTimeStylus sender, InAirPacketsData data){}
        public void TabletAdded(RealTimeStylus sender, TabletAddedData data){}
        public void TabletRemoved(RealTimeStylus sender, TabletRemovedData data) {}
    }
}
