// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// -------------------------------------------------------------------- 
namespace TestLargeMessageQueue
{
    using System;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Media;

    class Utilities
    {
        private Utilities()
        {
        }

        // Start Indicator for operation success/failure
        internal static void StartProgressBar(ProgressBar progressBar)
        {
            progressBar.Visibility = Visibility.Visible;
            progressBar.Foreground = Brushes.Green;
        }

        // Stop Indicator on operation completion
        internal static void StopProgressBar(ProgressBar progressBar)
        {
            progressBar.Value = progressBar.Maximum;
        }

        // Change Indicator on operation failure
        internal static void ErrorProgressBar(ProgressBar progressBar)
        {
            progressBar.Foreground = Brushes.Red;
            progressBar.Value = progressBar.Maximum;
        }
    }
}
