// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.PVM
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
            if (args.Length > 1 && args.Length < 5)
            {            
                if (string.Equals(args[0], "ImportVm", StringComparison.OrdinalIgnoreCase))
                {
                    string vmDefinitionPath = string.Empty;
                    string snapshotFolderPath = string.Empty;
                    bool newId = true;

                    if (args.Length < 2 || args.Length > 4)
                    {
                        ShowUsage();
                        return;
                    }
                    
                    if (args.Length >= 2)
                    {
                        vmDefinitionPath = args[1];
                    }
                    
                    if (args.Length >= 3)
                    {
                        snapshotFolderPath = args[2];
                    }
                    
                    if (args.Length == 4)
                    {
                        if (string.Equals(args[3], "True", StringComparison.OrdinalIgnoreCase))
                        {
                            newId = true;
                        }
                        else if (string.Equals(args[3], "False", StringComparison.OrdinalIgnoreCase))
                        {
                            newId = false;
                        }
                        else
                        {
                            ShowUsage();
                        }
                    }

                    using (ManagementObject pvm = ImportUtilities.ImportVm(vmDefinitionPath, snapshotFolderPath, newId))
                    {
                        Console.WriteLine("Imported Planned VM: {0} ({1}).\n", pvm["ElementName"], pvm["Name"]);
                    }
                }
                else if (string.Equals(args[0], "ImportSnapshots", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length == 3)
                    {
                        string pvmName = args[1];
                        string snapshotFolderPath = args[2];

                        IList<ManagementObject> snapshots =
                            ImportUtilities.ImportSnasphotDefinitions(pvmName, snapshotFolderPath);
                        
                        Console.WriteLine("Imported snapshots: \n");
                        foreach (ManagementObject snapshot in snapshots)
                        using (snapshot)
                        {
                            Console.WriteLine("                    {0} ({1})\n",
                                snapshot["ElementName"],
                                snapshot["InstanceID"]);
                        }
                        
                    }
                    else
                    {
                        ShowUsage();
                    }
                }
                else if (string.Equals(args[0], "ValidatePvm", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length == 2)
                    {
                        string pvmName = args[1];

                        ImportUtilities.ValidatePvm(pvmName);
                    }
                    else
                    {
                        ShowUsage();
                    }
                }
                else if (string.Equals(args[0], "RealizePvm", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length == 2)
                    {
                        string pvmName = args[1];

                        using (ManagementObject vm = ImportUtilities.RealizePvm(pvmName))
                        {
                            Console.WriteLine("Realized Virtual Machine: {0} ({1})", vm["ElementName"], vm["Name"]);
                        }
                    }
                    else
                    {
                        ShowUsage();
                    }
                }
                else if (string.Equals(args[0], "RemovePvm", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length == 2)
                    {
                        string pvmName = args[1];

                        ImportUtilities.RemovePvm(pvmName);
                    }
                }
                else if (string.Equals(args[0], "FixVHDPaths", StringComparison.OrdinalIgnoreCase))
                {
                    if (args.Length == 3)
                    {
                        string pvmName = args[1];
                        string vhdFolderPath = args[2];

                        FixUpUtilities.FixVhdPaths(pvmName, vhdFolderPath);
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
            Console.WriteLine("   ImportVm <VmDefinitionPath> [SnapshotFolderPath] [NewId<True/False>]\n");
            Console.WriteLine("   ImportSnapshots <PvmName> <SnapshotFolderPath>\n");
            Console.WriteLine("   ValidatePvm <PvmName>\n");
            Console.WriteLine("   RealizePvm <PvmName>\n");
            Console.WriteLine("   RemovePvm <PvmName>\n");
            Console.WriteLine("   FixVHDPaths <PvmName> <VHDFolderPath>\n");
        }
    }
}
