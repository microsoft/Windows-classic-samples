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

    static class Generation2VMGetSample
    {
        /// <summary>
        /// Query the generation of a VM,
        /// </summary>
        /// <param name="serverName">The name of the server on which to perform the action.</param>
        /// <param name="vmName">The name of the VM to query.</param>
        internal static void
        GetVMGeneration(
            string serverName,
            string vmName)
        {
            ManagementScope scope = new ManagementScope(@"\\" + serverName + @"\root\virtualization\v2", null);

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                string virtualSystemSubType = (string)vmSettings["VirtualSystemSubType"];

                if (string.Equals(virtualSystemSubType, "Microsoft:Hyper-V:SubType:1", StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine("VM {0} on server {1} is generation 1", vmName, serverName);
                }
                else if (string.Equals(virtualSystemSubType, "Microsoft:Hyper-V:SubType:2", StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine("VM {0} on server {1} is generation 2", vmName, serverName);
                }
                else
                {
                    Console.WriteLine("VM {0} on server {1} is an unknown generation", vmName, serverName);
                }
            }
        }
    }
}
