// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Generation2VM
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
                if (string.Equals(args[0], "GetVMGeneration", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];

                    Generation2VMGetSample.GetVMGeneration(
                        hostName,
                        vmName);
                }
                else if (string.Equals(args[0], "CreateGeneration2VM", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];

                    Generation2VMCreateSample.CreateGeneration2VM(
                        hostName,
                        vmName);
                }
                else if (string.Equals(args[0], "GetBootOrder", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];

                    Generation2VMGetBootOrderSample.GetGeneration2BootOrder(
                        hostName,
                        vmName);
                }
                else if (string.Equals(args[0], "GetSecureBoot", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];

                    Generation2VMGetSecureBootSample.GetGeneration2SecureBoot(
                        hostName,
                        vmName);
                }
                else if (string.Equals(args[0], "GetPauseAfterBootFailure", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];

                    Generation2VMGetPauseAfterBootFailureSample.GetPauseAfterBootFailure(
                        hostName,
                        vmName);
                }
                else
                {
                    ShowUsage();
                }
            }
            else if (args.Length == 4)
            {
                if (string.Equals(args[0], "SetBootOrder", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];
                    string first = args[3];

                    Generation2VMSetBootOrderSample.SetGeneration2BootOrder(
                        hostName,
                        vmName,
                        first);
                }
                else if (string.Equals(args[0], "SetSecureBoot", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];

                    if (string.Equals(args[3], "On", StringComparison.OrdinalIgnoreCase))
                    {
                        Generation2VMSetSecureBootSample.SetGeneration2SecureBoot(
                            hostName,
                            vmName,
                            true);
                    }
                    else if (string.Equals(args[3], "Off", StringComparison.OrdinalIgnoreCase))
                    {
                        Generation2VMSetSecureBootSample.SetGeneration2SecureBoot(
                            hostName,
                            vmName,
                            false);
                    }
                    else
                    {
                        ShowUsage();
                    }
                }
                else if (string.Equals(args[0], "SetPauseAfterBootFailure", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];
                    string pauseAfterBootFailure = args[3];

                    Generation2VMSetPauseAfterBootFailureSample.SetPauseAfterBootFailure(
                        hostName,
                        vmName,
                        pauseAfterBootFailure);
                }
                else if (string.Equals(args[0], "SetBootProtocol", StringComparison.OrdinalIgnoreCase))
                {
                    string hostName = args[1];
                    string vmName = args[2];

                    UInt16 protocol = UInt16.Parse(args[3], CultureInfo.CurrentCulture);
                    
                    Generation2VMSetBootProtocolSample.SetGeneration2BootProtocol(
                        hostName,
                        vmName,
                        protocol);
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
            Console.WriteLine("   GetVMGeneration <server> <vm>");
            Console.WriteLine("   CreateGeneration2VM <server> <vm>");
            Console.WriteLine("   GetBootOrder <server> <vm>");
            Console.WriteLine("   SetBootOrder <server> <vm> <device path>");
            Console.WriteLine("   SetBootProtocol <server> <vm> <protocol>");
            Console.WriteLine("   GetPauseAfterBootFailure <server> <vm>");
            Console.WriteLine("   SetPauseAfterBootFailure <server> <vm> <pause after boot failure>");
            Console.WriteLine("   GetSecureBoot <server> <vm>");
            Console.WriteLine("   SetSecureBoot <server> <vm> <Value>");
            Console.WriteLine("\n");
 
            Console.WriteLine("Examples:\n");
            Console.WriteLine("   {0} GetVMGeneration . vmname", moduleName);
            Console.WriteLine("   {0} CreateGeneration2VM . vmname", moduleName);
            Console.WriteLine("   {0} GetBootOrder . vmname", moduleName);
            Console.WriteLine("   {0} SetBootOrder . vmname \\AcpiEx(VMBus,0,0)\\VenHw(9B17E5A2-0891-42DD-B653-80B5C22809BA,635161F83EDFC546913FF2D2F965ED0EDEC0CDF01C45974892F6A09D14FF12DC)\\MAC(000000000000)", moduleName);
            Console.WriteLine("   {0} SetBootProtocol . vmname 4096", moduleName);
            Console.WriteLine("   {0} GetPauseAfterBootFailure . vmname", moduleName);
            Console.WriteLine("   {0} SetPauseAfterBootFailure . vmname true", moduleName);            
            Console.WriteLine("   {0} GetSecureBoot . vmname", moduleName);
            Console.WriteLine("   {0} SetSecureBoot . vmname On", moduleName);
            Console.WriteLine("   {0} SetSecureBoot . vmname Off", moduleName);
            Console.WriteLine("\n");
        }
    }
}
