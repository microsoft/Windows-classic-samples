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

    static class EnhancedSessionPolicySample
    {
        /// <summary>
        /// Set or queries the enabled state of enhanced session mode on the specified server.
        /// </summary>
        /// <param name="serverName">The name of the server on which to perform the operation.</param>
        /// <param name="set">TRUE if the enabled state is to be set, FALSE if is to be queried.</param>
        /// <param name="enable">The new state.</param>
        internal static void
        Enable(
            string serverName,
            bool set,
            bool enable)
        {
            ManagementScope scope = new ManagementScope(@"\\" + serverName + @"\root\virtualization\v2", null);

            using (ManagementObject settings = WmiUtilities.GetVirtualMachineManagementServiceSettings(scope))
            using (ManagementObject service = WmiUtilities.GetVirtualMachineManagementService(scope))
            {
                if (set)
                {
                    using (ManagementBaseObject inParams = service.GetMethodParameters("ModifyServiceSettings"))
                    {
                        settings["EnhancedSessionModeEnabled"] = enable;
                        inParams["SettingData"] = settings.GetText(TextFormat.WmiDtd20);

                        using (ManagementBaseObject outParams =
                               service.InvokeMethod("ModifyServiceSettings",
                                                    inParams,
                                                    null))
                        {
                            WmiUtilities.ValidateOutput(outParams, scope, true, true);
                        }
                    }

                    Console.WriteLine("Successfully {0} enhanced session mode.", enable ? "enabled" : "disabled");
                    return;
                }

                bool enabled = (bool)settings["EnhancedSessionModeEnabled"];

                if (enabled)
                {
                    Console.WriteLine("Enhanced Session Mode is enabled");
                }
                else
                {
                    Console.WriteLine("Enhanced Session Mode is disabled");
                }
            }
        }
    }
}
