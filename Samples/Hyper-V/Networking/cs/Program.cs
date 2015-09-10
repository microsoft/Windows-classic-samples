// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Networking
{
    using System;
    using System.Globalization;
    using System.Reflection;

    static class Program
    {
        /// <summary>
        /// Entry point of the sample.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        static void 
        Main(
            string[] args)
        {
            if (args.Length > 0)
            {
                string sample = args[0];

                if (string.Equals(sample, "CreateSwitch", StringComparison.OrdinalIgnoreCase))
                {
                    CreateSwitchSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "DeleteSwitch", StringComparison.OrdinalIgnoreCase))
                {
                    DeleteSwitchSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "ModifySwitch", StringComparison.OrdinalIgnoreCase))
                {
                    ModifySwitchSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "AddAndRemovePorts", StringComparison.OrdinalIgnoreCase))
                {
                    AddAndRemovePortsSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "ModifyPorts", StringComparison.OrdinalIgnoreCase))
                {
                    ModifyPortsSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "EnumerateSwitch", StringComparison.OrdinalIgnoreCase))
                {
                    EnumerateSwitchSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "SupportsTrunkMode", StringComparison.OrdinalIgnoreCase))
                {
                    SupportsTrunkModeSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "ConnectVmToSwitch", StringComparison.OrdinalIgnoreCase))
                {
                    ConnectVmToSwitchSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "AdvancedConnectionProperties", StringComparison.OrdinalIgnoreCase))
                {
                    AdvancedConnectionPropertiesSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "ConnectVmUsingResourcePool", StringComparison.OrdinalIgnoreCase))
                {
                    ConnectVmUsingResourcePoolSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "ManageExtension", StringComparison.OrdinalIgnoreCase))
                {
                    ManageExtensionSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "SetRequiredFeature", StringComparison.OrdinalIgnoreCase))
                {
                    SetRequiredFeatureSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "AdvancedSwitchProperties", StringComparison.OrdinalIgnoreCase))
                {
                    AdvancedSwitchPropertiesSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "SyntheticEthernetPortProperties", StringComparison.OrdinalIgnoreCase))
                {
                    SyntheticEthernetPortPropertiesSample.ExecuteSample(StripFirstArgument(args));
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
        /// Displays the command line usage for the sample.
        /// </summary>
        static void
        ShowUsage()
        {
            string moduleName = Assembly.GetExecutingAssembly().GetModules()[0].Name;

            Console.WriteLine("Usage:\t{0} <SampleName> <Arguments>\n", moduleName);

            Console.WriteLine("Supported SampleNames (Type <SampleName> /? for information):\n");
            Console.WriteLine("   CreateSwitch");
            Console.WriteLine("   DeleteSwitch");
            Console.WriteLine("   ModifySwitch");
            Console.WriteLine("   AddAndRemovePorts");
            Console.WriteLine("   ModifyPorts");
            Console.WriteLine("   EnumerateSwitch");
            Console.WriteLine("   SupportsTrunkMode");
            Console.WriteLine("   ConnectVmToSwitch");
            Console.WriteLine("   AdvancedConnectionProperties");
            Console.WriteLine("   ConnectVmUsingResourcePool");
            Console.WriteLine("   ManageExtension");
            Console.WriteLine("   SetRequiredFeature");
            Console.WriteLine("   AdvancedSwitchProperties");
            Console.WriteLine("   SyntheticEthernetPortProperties");
        }

        /// <summary>
        /// Helper method to take the command line arguments and extract the first item from the 
        /// array in order to more easily pass the remaining arguments to the specific sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        /// <returns>The modified command line arguments.</returns>
        static string[]
        StripFirstArgument(
            string[] args)
        {
            string[] newArgs = new string[args.Length - 1];

            Array.Copy(args, 1, newArgs, 0, args.Length - 1);

            return newArgs;
        }
    }
}
