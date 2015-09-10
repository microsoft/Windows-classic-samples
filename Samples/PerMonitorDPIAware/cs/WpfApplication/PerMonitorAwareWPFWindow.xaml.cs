//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

using NativeHelpers;
using System;
using System.Windows;


namespace PerMonitorAwareWPFApplication
{
     // This sample showcases how to make a WPF application Per-Monitor Aware. WPF uses the system DPI to scale the size of the window and 
     // rendering. In order to make a WPF application Per_Monitor aware, we need to
     //    a. Mark the WPF application as DPI_Unaware: In the AssemblyInfo.cs file add the following line -> [assembly:  DisableDpiAwareness] 
     //    b. Inherit the main WPF window class from PerMonitorDPIWindow class. This will mark the application as Per_Monitor_DPI_Aware.  
     //
     // These steps will 
     //    a. Make the application as Per_Monitor_DPI_Aware to the OS. This will ensure that the OS 
     //        i. Does not  scale the application when the system DPI != monitor DPI
     //        ii. Send the WM_DPICHANGED message whenever the DPI of the window changes
     //
     //    b. When the application is marked as Per_Monitor_DPI_Aware, WPF still treats the application as System DPI aware and scales the 
     //       application window and rendering by the System DPI. In order to ensure that the scale of the Window and the rendering is based 
     //       on the current DPI of the monitor the window is on, we need to 
     //        i. Update the size of the window based on the DPI of the monitor rather than the WPF DPI
     //        ii. Update the scaling used by WPF to be based on the  DPI of the monitor rather than the its own DPI 
     //
     // The samples includes the following core set of files/classes
     //     a. PerMonitorDPIHelpers.h/PerMonitorDPIHelpers.cpp : Provides helper functions for DPI related operations
     //     b. PerMonitorDPIWindow.h/PerMonitorDPIWindow.cpp: Base class that implements the operations required to make this application 
     //        Per_Monitor_DPI_Aware. Implements the window message handle that is used to respond to DPI change 
     //        as well as apply the appropriate scale transform to the root node of the main WPF window
    //     c.  PerMonitorAwareWPFWindow.xaml/PerMonitorAwareWPFWindow.xaml.cs : Sample WPF application that uses the native helpers to showcase a per-monitor DPI aware WPF application. 
     //        The Window is designed for 96 DPI with the expectation that the sample will scale the window size and the rendering scale based 
     //        on the current DPI of the application.
    

    public partial class PerMonitorAwareWPFWindow : PerMonitorDPIWindow
    {

        public PerMonitorAwareWPFWindow()
        {
           InitializeComponent();          
        }



       private void PerMonitorDPIWindow_LayoutUpdated(object sender, EventArgs e)
       {
           UpdateDPIText();

       }
       
       private void PerMonitorDPIWindow_DPIChanged(object sender, EventArgs e)
        {

            UpdateDPIText();
        }

       private void PerMonitorDPIWindow_SizeChanged(object sender, SizeChangedEventArgs e)
       {
           UpdateDPIText();
         
       }

        private void UpdateDPIText()
       {
           DPI.Text = "Window Information" +
                   "\n\tSystem DPI = " + PerMonitorDPIHelper.GetSystemDPI() +
                   "\n\tWPF Rendering DPI = " + this.WpfDPI +
                   "\n\tMonitor DPI = " + this.CurrentDPI +
                   "\n\tWindow Size in Physical Pixels  = " + (this.ActualWidth * this.WpfDPI / 96.0) + " x " + (this.ActualHeight * this.WpfDPI / 96.0);
       }

   


    }       
}
