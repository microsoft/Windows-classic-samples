// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.AppHealth
{
    using System;

    class AppHealth
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
            const string usageAppHealthStatus = "Usage: AppHealth status <hostMachine> <vmName>";

            try
            {
                AppHealthStatus appHealth = new AppHealthStatus();

                if (args == null || args.Length == 0)
                {
                    Console.WriteLine(usageAppHealthStatus);
                }
                else if (args[0].Equals("status", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length != 3)
                    {
                        Console.WriteLine(usageAppHealthStatus);
                    }
                    else
                    {
                        appHealth.GetAppHealthStatus(args[1], args[2]);
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