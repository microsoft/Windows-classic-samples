// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

namespace Microsoft.Samples.HyperV.FibreChannel
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

                if (string.Equals(sample, "CreateSan", StringComparison.OrdinalIgnoreCase))
                {
                    CreateSanSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "DeleteSan", StringComparison.OrdinalIgnoreCase))
                {
                    DeleteSanSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "CreateVirtualFcPort", StringComparison.OrdinalIgnoreCase))
                {
                    CreateVirtualFcPortSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "DeleteVirtualFcPort", StringComparison.OrdinalIgnoreCase))
                {
                    DeleteVirtualFcPortSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "ModifySanName", StringComparison.OrdinalIgnoreCase))
                {
                    ModifySanNameSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "ModifySanPorts", StringComparison.OrdinalIgnoreCase))
                {
                    ModifySanPortsSample.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "ConfigureWwnGenerator", StringComparison.OrdinalIgnoreCase))
                {
                    ConfigureWwnGenerator.ExecuteSample(StripFirstArgument(args));
                }
                else if (string.Equals(sample, "EnumerateFcPorts", StringComparison.OrdinalIgnoreCase))
                {
                    EnumerateFcPorts.ExecuteSample(StripFirstArgument(args));
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
            Console.WriteLine("   CreateSan <SanName> <[WWPN WWNN]+> [SAN Notes]");
            Console.WriteLine("   DeleteSan <SanName>");
            Console.WriteLine("   ModifySanName <SanName> <NewSanName");
            Console.WriteLine("   ModifySanPorts <SanName> [WWPN WWNN]*");
            Console.WriteLine("   CreateVirtualFcPort <VmName> <SanName> [WWPN-A WWNN-A WWPN-B WWNN-B]");
            Console.WriteLine("   DeleteVirtualFcPort <VmName> <WWPN-A WWNN-A WWPN-B WWNN-B>");
            Console.WriteLine("   ConfigureWwnGenerator <Min WWPN> <Max WWPN> [New WWNN]");
            Console.WriteLine("   EnumerateFcPorts");

            Console.WriteLine("\nExamples:");
            Console.WriteLine("\n   CreateSan TestSAN 1122334455667788 0011223344556677 \"Notes for TestSAN\"");
            Console.WriteLine("      This creates a virtual SAN named TestSAN with notes and one host resource assigned.");
            Console.WriteLine("\n   CreateSan TestSAN 1122334455667788 0011223344556677 2233445566778899 0011223344556677");
            Console.WriteLine("      This creates a virtual SAN named TestSAN with two host resources assigned.");
            Console.WriteLine("\n   DeleteSan TestSAN");
            Console.WriteLine("      This deletes a virtual SAN named TestSAN.");
            Console.WriteLine("\n   ModifySanName TestSAN ProductionSAN");
            Console.WriteLine("      This renames a Virtual SAN from TestSAN to ProductionSAN.");
            Console.WriteLine("\n   ModifySanPorts TestSAN 2233445566778899 0011223344556677");
            Console.WriteLine("      This modifies the previously created TestSAN with 2 host resources to have only one host resource specified.");
            Console.WriteLine("\n   ModifySanPorts TestSAN");
            Console.WriteLine("      This modifies the previously created TestSAN to remove all previously assigned resources from the SAN.");
            Console.WriteLine("\n   CreateVirtualFcPort TestVM TestSAN");
            Console.WriteLine("      This creates a virtual FC Port on TestVM and connects it to TestSAN.");
            Console.WriteLine("      It uses the WWPN Generator to assign the World Wide Names.");
            Console.WriteLine("\n   CreateVirtualFcPort TestVM TestSAN 33445566778899aa 0033445566778899 33445566778899bb 0033445566778899");
            Console.WriteLine("      This creates a virtual FC Port on TestVM and connects it to TestSAN and assigns the given WWNs.");
            Console.WriteLine("\n   DeleteVirtualFcPort TestVM 33445566778899aa 0033445566778899 33445566778899bb 0033445566778899");
            Console.WriteLine("      This deletes a virtual FC Port on TestVM identified by the given WWNs.");
            Console.WriteLine("\n   ConfigureWwnGenerator 1122334400000000 11223344FFFFFFFF");
            Console.WriteLine("      This will make the WWN Generator generate WWPNs from within the specified range and use the CurrentWWNNAddress for the WWNN.");
            Console.WriteLine("\n   ConfigureWwnGenerator 1122334400000000 11223344FFFFFFFF 0011223344556677");
            Console.WriteLine("      This will make the WWN Generator generate WWPNs from within the specified range and use the new WWNN specified.");
            Console.WriteLine("\n   EnumerateFcPorts");
            Console.WriteLine("      This will enumerate all the external FC ports on the host and display the WWPN, WWNN and Status for each of them.");
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
