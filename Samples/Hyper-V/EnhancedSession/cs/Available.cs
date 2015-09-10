// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.EnhancedSession
{
    using System;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    static class EnhancedSessionIsAvailableSample
    {
        const UInt16 CimEnabledStateEnabled = 2;
        const UInt16 CimEnabledStateDisabled = 3;
        const UInt16 CimEnabledStateEnabledButOffline = 6;

        /// <summary>
        /// Query the availablity of enhanced session mode on a specified VM.
        /// </summary>
        /// <param name="serverName">The name of the server on which to perform the action.</param>
        /// <param name="vmName">The name of the VM to query.</param>
        internal static void
        IsAvailable(
            string serverName,
            string vmName)
        {
            ManagementScope scope = new ManagementScope(@"\\" + serverName + @"\root\virtualization\v2", null);

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            {
                UInt16 enhancedModeState = (UInt16)vm.GetPropertyValue("EnhancedSessionModeState");

                if (enhancedModeState == CimEnabledStateEnabled)
                {
                    Console.WriteLine("Enhanced mode is allowed and currently available on VM {0} on server {1}", vmName, serverName);
                }
                else if (enhancedModeState == CimEnabledStateDisabled)
                {
                    Console.WriteLine("Enhanced mode is not allowed on VM {0} on server {1}", vmName, serverName);
                }
                else if (enhancedModeState == CimEnabledStateEnabledButOffline)
                {
                    Console.WriteLine("Enhanced mode is allowed and but not currently available on VM {0} on server {1}", vmName, serverName);
                }
                else
                {
                    Console.WriteLine("Enhanced mode state on VM {0} on server {1} is {2}", vmName, serverName, enhancedModeState);
                }
            }
        }
    }
}
