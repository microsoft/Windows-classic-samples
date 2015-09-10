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

    static class Generation2VMSetBootProtocolSample
    {
        /// <summary>
        /// Set the boot protocol for a VM.
        /// </summary>
        /// <param name="serverName">The name of the server on which to perform the action.</param>
        /// <param name="vmName">The name of the VM on which to set boot order.</param>
        /// <param name="first">The preferred network boot protocol.</param>
        internal static void
        SetGeneration2BootProtocol(
            string serverName,
            string vmName,
            UInt16 protocol)
        {
            ManagementScope scope = new ManagementScope(@"\\" + serverName + @"\root\virtualization\v2", null);

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vmSettings = WmiUtilities.GetVirtualMachineSettings(vm))
            {
                string virtualSystemSubType = (string)vmSettings["VirtualSystemSubType"];

                if (!string.Equals(virtualSystemSubType, "Microsoft:Hyper-V:SubType:2", StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine("VM {0} on server {1} is not generation 2", vmName, serverName);
                    return;
                }

                //
                // Modify the virtual system to use the new protocol.
                //

                vmSettings["NetworkBootPreferredProtocol"] = protocol;

                using (ManagementObject service = WmiUtilities.GetVirtualMachineManagementService(scope))
                using (ManagementBaseObject inParams = service.GetMethodParameters("ModifySystemSettings"))
                {
                    inParams["SystemSettings"] = vmSettings.GetText(TextFormat.WmiDtd20);

                    using (ManagementBaseObject outParams =
                            service.InvokeMethod("ModifySystemSettings",
                                                inParams,
                                                null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope, true, true);
                    }

                    Console.WriteLine("Successfully modified boot protocol");
                    return;
                }
            }
        }
    }
}
