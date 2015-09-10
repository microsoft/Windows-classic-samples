// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.IntegrationServices
{
    using System;
    using System.Reflection;
    using System.Globalization;

    class CopyFileSample
    {
        /// <summary>
        /// The entry point.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        static void
        Main(
            string[] args)
        {
            try
            {
                if ((args == null) || (args.Length < 5) || (args.Length > 7))
                {
                    ShowUsage();
                }

                if (string.Equals(args[0], "CopyFileToGuest", StringComparison.OrdinalIgnoreCase))
                {
                    bool overwriteExisting = false;
                    bool createFullPath = false;

                    //
                    // If OverwriteExisting argument is specified, get its value.
                    //

                    if (args.Length > 5)
                    {
                        overwriteExisting = Convert.ToBoolean(
                            args[5],
                            CultureInfo.InvariantCulture);

                        //
                        // If CreateFullPath argument is specified, get its value.
                        //

                        if (args.Length == 7)
                        {
                            createFullPath = Convert.ToBoolean(
                                args[6],
                                CultureInfo.InvariantCulture);
                        }
                    }

                    //
                    // Copy file to the guest.
                    //

                    GuestServiceInterface guestServiceInterface = new GuestServiceInterface();

                    guestServiceInterface.CopyFileToGuest(
                        args[1],
                        args[2],
                        args[3],
                        args[4],
                        overwriteExisting,
                        createFullPath);
                }
                else
                {
                    ShowUsage();
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
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
            Console.WriteLine("   CopyFileToGuest <HostMachine> <VMName> <SourceFile> <DestinationFile> [<OverwriteExisting> [<CreateFullPath>]] \n");

            Console.WriteLine("Examples:\n");
            Console.WriteLine("   {0} CopyFileToGuest HyperVHost WS12VM c:\\temp\\testfile c:\\temp\\testfile1", moduleName);
            Console.WriteLine("   {0} CopyFileToGuest HyperVHost WS12VM c:\\temp\\testfile c:\\temp\\testfile1 true", moduleName);
            Console.WriteLine("   {0} CopyFileToGuest HyperVHost WS12VM c:\\temp\\testfile c:\\temp\\testfile1 true true", moduleName);
        }
    }
}
