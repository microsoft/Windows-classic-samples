// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.StorageQoS
{
    using System;

    class Program
    {
        /// <summary>
        /// The entry point.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        static
        void
        Main(
            string[] args)
        {
            const string usageGetSQoSAttributes = "Usage: StorageQoSSamples get <hostMachine> <vmName> <vhdFileName>";
            const string usageSetSQoSAttributes = "Usage: StorageQoSSamples set <hostMachine> <vmName> <vhdFileName>  <-max|-min> <IOPS>";
            const string usageMonitorSQoSEvents = "Usage: StorageQoSSamples monitor <hostMachine>";

            try
            {
                SQoS sqos = new SQoS();

                if (args == null || args.Length == 0)
                {
                    Console.WriteLine(usageGetSQoSAttributes);
                    Console.WriteLine(usageSetSQoSAttributes);
                    Console.WriteLine(usageMonitorSQoSEvents);
                }
                else if (args[0].Equals("get", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length == 3)
                    {
                        sqos.GetSQoSAttributes(args[1], args[2], null);
                    }
                    else if (args.Length == 4)
                    {
                        sqos.GetSQoSAttributes(args[1], args[2], args[3]);
                    }
                    else
                    {
                        Console.WriteLine(usageGetSQoSAttributes);
                    }
                }
                else if (args[0].Equals("set", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length == 6 && (args[4].Equals("-Max", StringComparison.OrdinalIgnoreCase) 
                                          || args[4].Equals("-Min", StringComparison.OrdinalIgnoreCase)))
                    {
                        UInt64 iops;
                        if (UInt64.TryParse(args[5], out iops))
                        {
                            sqos.SetSQoSAttributes(args[1], args[2], args[3], args[4], iops);
                        }
                        else
                        {
                            Console.WriteLine("Please specify the desired IOPS value as an integer number.");
                        }                        
                    }
                    else
                    {
                         Console.WriteLine(usageSetSQoSAttributes);
                    }
                }
                else if (args[0].Equals("monitor", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length == 2)
                    {
                        sqos.MonitorSQoSEvents(args[1]);
                    }
                    else
                    {
                        Console.WriteLine(usageMonitorSQoSEvents);
                    }
                }
                else
                {
                    Console.WriteLine(usageGetSQoSAttributes);
                    Console.WriteLine(usageSetSQoSAttributes);
                    Console.WriteLine(usageMonitorSQoSEvents);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
            }
        }
    }
}