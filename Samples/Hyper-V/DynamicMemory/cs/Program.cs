// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Slp
{
    using System;

    class DynMem
    {
        /// <summary>
        /// The entry point.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        static
        void
        Main(
            string[] args
            )
        {
            const string usageMemStatus = "Usage: DynMem mem-status <hostMachine> <vmName>";
            const string usageGetSlpRoot = "Usage: DynMem get-swap-root <hostMachine> <vmName>";
            const string usageModifySlpRoot = "Usage: DynMem modify-swap-root <hostMachine> <vmName> <new-location>";

            try
            {
                Slp slp = new Slp();

                if (args == null || args.Length == 0)
                {
                    Console.WriteLine(usageMemStatus);
                    Console.WriteLine(usageGetSlpRoot);
                    Console.WriteLine(usageModifySlpRoot);
                }
                else if (args[0].Equals("mem-status", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length != 3)
                    {
                        Console.WriteLine(usageMemStatus);
                    }
                    else
                    {
                        slp.GetVmMemoryStatus(args[1], args[2]);
                    }
                }
                else if (args[0].Equals("get-swap-root", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length != 3)
                    {
                        Console.WriteLine(usageGetSlpRoot);
                    }
                    else
                    {
                        slp.GetSlpDataRoot(args[1], args[2]);
                    }
                }
                else if (args[0].Equals("modify-swap-root", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length != 4)
                    {
                        Console.WriteLine(usageModifySlpRoot);
                    }
                    else
                    {
                        slp.ModifySlpDataRoot(args[1], args[2], args[3]);
                    }
                }
                else
                {
                    Console.WriteLine("Incorrect option");
                }

                Console.WriteLine("Please press ENTER to exit...");
                Console.ReadLine();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
            }
        }
    }
}