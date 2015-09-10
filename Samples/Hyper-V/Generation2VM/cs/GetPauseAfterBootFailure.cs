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

    static class Generation2VMGetPauseAfterBootFailureSample
    {
        /// <summary>
        /// Query the pause after boot failure property for a VM.
        /// </summary>
        /// <param name="serverName">The name of the server on which to perform the action.</param>
        /// <param name="vmName">The name of the VM to query.</param>
        internal static void
        GetPauseAfterBootFailure(
            string serverName,
            string vmName)
        {
            ManagementScope scope = new ManagementScope(@"\\" + serverName + @"\root\virtualization\v2", null);

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                bool pauseAfterBootFailure = (bool)vmSettings["PauseAfterBootFailure"];

                if (pauseAfterBootFailure)
                {
                    Console.WriteLine("VM {0} on server {1} has PauseAfterBootFailure property enabled.", vmName, serverName);
                }
                else
                {
                    Console.WriteLine("VM {0} on server {1} has PauseAfterBootFailure property disabled.", vmName, serverName);
                }
                
                return;
            }
        }
    }
}
