// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

using System;
using System.Windows;

namespace Microsoft.Samples.Speech.VoiceAccess
{
    /// <summary>
    /// Voice Access application entry point
    /// </summary>
    public partial class VoiceAccessApp : Application
    {
        /// <summary>
        /// Application entry point
        /// </summary>
        [STAThread]
        public static void Main()
        {
            VoiceAccessApp app = new VoiceAccessApp();
            app.Run(new VoiceAccessWindow());
        }
    }
}