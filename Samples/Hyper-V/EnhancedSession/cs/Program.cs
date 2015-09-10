// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.EnhancedSession
{
    using System;
    using System.Globalization;
    using System.Reflection;
 
    class Program
    {
        /// <summary>
        /// Entry point of the program.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        static void 
        Main(
            string[] args)
        {
            if (args.Length == 3)
            {
                if (string.Equals(args[0], "IsAvailable", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];

                    EnhancedSessionIsAvailableSample.IsAvailable(
                        hostName,
                        vmName);
                }
                else
                {
                    ShowUsage();
                }
            }
            else if (args.Length == 2)
            {
                if (string.Equals(args[0], "Enable", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];

                    EnhancedSessionPolicySample.Enable(
                        hostName,
                        true,
                        true);
                }
                else if (string.Equals(args[0], "Disable", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];

                    EnhancedSessionPolicySample.Enable(
                        hostName,
                        true,
                        false);
                }
                else if (string.Equals(args[0], "IsEnabled", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];

                    EnhancedSessionPolicySample.Enable(
                        hostName,
                        false,
                        false);
                }
                else
                {
                    ShowUsage();
                }
            }
            else
            {
                ShowUsage();
            }
        }

        /// <summary>
        /// Displays the command line usage for the program.
        /// </summary>
        static void
        ShowUsage()
        {
            string moduleName = Assembly.GetExecutingAssembly().GetModules()[0].Name;

            Console.WriteLine("\nUsage:\t{0} <SampleName> <Arguments>\n", moduleName);

            Console.WriteLine("Supported SampleNames and Arguments:\n");
            Console.WriteLine("   Enable <server>");
            Console.WriteLine("   Disable <server>");
            Console.WriteLine("   IsEnabled <server>");
            Console.WriteLine("   IsAvailable <server> vmname");
            Console.WriteLine("\n");
 
            Console.WriteLine("Examples:\n");
            Console.WriteLine("   {0} Enable .", moduleName);
            Console.WriteLine("   {0} Disable .", moduleName);
            Console.WriteLine("   {0} IsEnabled .", moduleName);
            Console.WriteLine("   {0} IsAvailable . vmname", moduleName);
            Console.WriteLine("\n");
        }
    }
}
