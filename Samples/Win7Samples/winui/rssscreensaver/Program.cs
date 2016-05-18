// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.

using System;
using System.Windows.Forms;
using System.Globalization;

namespace Microsoft.Samples.RssPlatform.ScreenSaver
{
    /// <summary>
    /// Thrown in the event that the Interop.Feeds.dll cannot be located.
    /// </summary>
    [Serializable]
    public class NoInteropException : System.Exception
    {
        public NoInteropException() { }
        public NoInteropException(string message)
            : base(message) { }
        public NoInteropException(string message, Exception inner)
            : base(message, inner) { }
        protected NoInteropException(
                System.Runtime.Serialization.SerializationInfo info, 
                System.Runtime.Serialization.StreamingContext context)
            : base(info, context) { }
    }
    
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            try
            {
                if (args.Length > 0)
                {
                    // Get the 2 character command line argument
                    string arg = args[0].ToLowerInvariant().Trim().Substring(0, 2);
                    switch (arg)
                    {
                        case "/c":
                            // Show the options dialog
                            ShowOptions();
                            break;
                        case "/p":
                            // Don't do anything for preview
                            break;
                        case "/s":
                            // Show screensaver form
                            ShowScreenSaver();
                            break;
                        case "/d":
                            // Show screensaver in debug mode
                            ShowScreenSaver(true);
                            break;
                        default:
                            MessageBox.Show("Invalid command line argument: " + arg, "Invalid Command Line Argument", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            break;
                    }
                }
                else
                {
                    // If no arguments were passed in, show the screensaver
                    ShowScreenSaver();
                }
            }
            catch (NoInteropException ex)
            {
                string msg = ex.Message;

                if (msg.Contains("Interop.Feeds"))
                    msg = "Please copy Interop.Feeds.dll into the same directory as the screen saver binary.";

                MessageBox.Show(msg, "Missing Interop Assembly", MessageBoxButtons.OK, MessageBoxIcon.Stop);
            }
        }
        
        static void ShowOptions()
        {
            OptionsForm optionsForm = new OptionsForm();
            Application.Run(optionsForm);
        }

        static void ShowScreenSaver()
        {
            ShowScreenSaver(false);
        }
        
        static void ShowScreenSaver(bool debugmode)
        {
            ScreenSaverForm screenSaver = new ScreenSaverForm(debugmode);
            Application.Run(screenSaver);
        }
    }
}