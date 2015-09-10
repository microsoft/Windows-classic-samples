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

    static class Generation2VMGetSecureBootSample
    {
        /// <summary>
        /// Query the secure boot property for a Generation 2 VM.
        /// </summary>
        /// <param name="serverName">The name of the server on which to perform the action.</param>
        /// <param name="vmName">The name of the VM to query.</param>
        internal static void
        GetGeneration2SecureBoot(
            string serverName,
            string vmName)
        {
            ManagementScope scope = new ManagementScope(@"\\" + serverName + @"\root\virtualization\v2", null);

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            {
                using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
                {
                    // Get incremental backup status.
                    bool secureBootEnabled = (bool)vmSettings["SecureBootEnabled"];

                    if (secureBootEnabled)
                    {
                        Console.WriteLine("VM {0} on server {1} has SecureBoot enabled.", vmName, serverName);
                    }
                    else
                    {
                        Console.WriteLine("VM {0} on server {1} has SecureBoot disabled", vmName, serverName);
                    }
                }
            }
        }
    }
}
