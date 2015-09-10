// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Metrics
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
            if (args.Length == 2)
            {
                if (string.Equals(args[0], "EnableMetricsForVm", StringComparison.OrdinalIgnoreCase))
                {
                    ControlMetricsSample.EnableMetricsForVirtualMachine(args[1]);
                }
                else if (string.Equals(args[0], "ConfigureMetricsFlushInterval", StringComparison.OrdinalIgnoreCase))
                {
                    TimeSpan interval = new TimeSpan(
                        Int32.Parse(args[1], CultureInfo.CurrentCulture), 0, 0);

                    ModifyServiceSettingsSample.ConfigureMetricsFlushInterval(interval);
                }
                else if (string.Equals(args[0], "QueryMetricCollectionEnabledForVm", StringComparison.OrdinalIgnoreCase))
                {
                    EnumerateMetricsSample.QueryMetricCollectionEnabledForVirtualMachine(args[1]);
                }
                else if (string.Equals(args[0], "EnumerateDiscreteMetricsForVm", StringComparison.OrdinalIgnoreCase))
                {
                    EnumerateMetricsSample.EnumerateDiscreteMetricsForVm(args[1]);
                }
                else
                {
                    ShowUsage();
                }
            }
            else if (args.Length == 3 || args.Length == 4)
            {
                if (string.Equals(args[0], "DisableMetricsForNetworkAdapter", StringComparison.OrdinalIgnoreCase))
                {
                    ControlMetricsSample.DisableMetricsForNetworkAdapter(args[1], args[2]);
                }
                else if (string.Equals(args[0], "EnumerateMetricsForResourcePool", StringComparison.OrdinalIgnoreCase))
                {
                    string poolId = (args.Length == 3) ? string.Empty : args[3];

                    EnumerateMetricsSample.EnumerateMetricsForResourcePool(args[1], args[2], poolId);
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

            Console.WriteLine("Usage:\t{0} <SampleName> <Arguments>\n", moduleName);

            Console.WriteLine("Supported SampleNames and Arguments:\n");
            Console.WriteLine("   EnableMetricsForVm <VmName>");
            Console.WriteLine("   DisableMetricsForNetworkAdapter <MacAddress> <IpAddress>");
            Console.WriteLine("   ConfigureMetricsFlushInterval <IntervalHours>");
            Console.WriteLine("   QueryMetricCollectionEnabledForVm <VmName>");
            Console.WriteLine("   EnumerateDiscreteMetricsForVm <VmName>");
            Console.WriteLine("   EnumerateMetricsForResourcePool <ResourceType> <ResourceSubType> [PoolId]\n");

            Console.WriteLine("Examples:\n");
            Console.WriteLine("   {0} EnableMetricsForVm WS08R2VM", moduleName);
            Console.WriteLine("   {0} DisableMetricsForNetworkAdapter 00-15-5D-CB-23-02 10.0.0.0/8", moduleName);
            Console.WriteLine("   {0} ConfigureMetricsFlushInterval 10", moduleName);
            Console.WriteLine("   {0} EnumerateMetricsForResourcePool 4 \"Microsoft:Hyper-V:Memory\"\n", moduleName);
        }
    }
}
