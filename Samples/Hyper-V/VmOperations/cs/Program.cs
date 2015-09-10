// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.VmOperations
{
    using System;
    using System.Globalization;
    using System.Management;
    using System.Reflection;
    using System.Collections.Generic;

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
            if (args.Length < 1)
            {
                ShowUsage();
                return;
            }

            if (string.Equals(args[0], "InjectNmi", StringComparison.OrdinalIgnoreCase))
            {
                if (args.Length != 2)
                {
                    ShowUsage();
                    return;
                }

                string vmName = args[1];
                LinuxAffinityUtilities.InjectNmi(vmName);
            }

            else if (string.Equals(args[0], "ConfigureMmioGap", StringComparison.OrdinalIgnoreCase))
            {
                if (args.Length != 3)
                {
                    ShowUsage();
                    return;
                }

                string vmName = args[1];
                uint gapSize = 0;

                if (!UInt32.TryParse(
                        args[2], NumberStyles.Integer, CultureInfo.InvariantCulture, out gapSize))
                {
                    Console.WriteLine("Invalid format for parameter GapSize!\n");
                    return;
                }

                LinuxAffinityUtilities.ConfigureMmioGap(vmName, gapSize);
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

            Console.WriteLine("Usage:\t{0} <SampleName> <Arguments>\n", moduleName);

            Console.WriteLine("Supported SampleNames and Arguments:\n");
            Console.WriteLine("   InjectNmi <VmName>\n");
            Console.WriteLine("     - VM must be running\n");
            Console.WriteLine("   ConfigureMmioGap <VmName> <GapSize>\n");
            Console.WriteLine("     - VM must be powered off\n");
        }
    }
}
