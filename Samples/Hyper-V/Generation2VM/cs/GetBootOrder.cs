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
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    static class Generation2VMGetBootOrderSample
    {
        /// <summary>
        /// Query the boot order for a VM.
        /// </summary>
        /// <param name="serverName">The name of the server on which to perform the action.</param>
        /// <param name="vmName">The name of the VM to query.</param>
        internal static void
        GetGeneration2BootOrder(
            string serverName,
            string vmName)
        {
            ManagementScope scope = new ManagementScope(@"\\" + serverName + @"\root\virtualization\v2", null);

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                // Determine the generation of this VM.
                string virtualSystemSubType = (string)vmSettings["VirtualSystemSubType"];
                UInt16 networkBootPreferredProtocol = (UInt16)vmSettings["NetworkBootPreferredProtocol"];

                if (!string.Equals(virtualSystemSubType, "Microsoft:Hyper-V:SubType:2", StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine("VM {0} on server {1} is not generation 2", vmName, serverName);
                    return;
                }

                string[] bootOrder = (string[])vmSettings["BootSourceOrder"];
                int index = 0;

                Console.WriteLine("NetworkBootPreferredProtocol = {0}", networkBootPreferredProtocol);
                Console.WriteLine("BootSourceOrder:");

                foreach (string entry in bootOrder)
                {
                    Console.WriteLine("Entry {0}:", index++);

                    ManagementPath entryPath = new ManagementPath(entry);

                    using (ManagementObject entryObject = new ManagementObject(entryPath))
                    {
                        Console.WriteLine("    BootSourceDescription    = {0}", entryObject["BootSourceDescription"]);
                        Console.WriteLine("    BootSourceType           = {0}", entryObject["BootSourceType"]);
                        Console.WriteLine("    OtherLocation            = {0}", entryObject["OtherLocation"]);
                        Console.WriteLine("    FirmwareDevicePath");
                        Console.WriteLine("        {0}", entryObject["FirmwareDevicePath"]);
                        Console.WriteLine();
                    }
                }
            }
        }
    }
}
