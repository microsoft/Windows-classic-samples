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

    static class Generation2VMSetBootOrderSample
    {
        /// <summary>
        /// Set the boot order for a VM.
        /// </summary>
        /// <param name="serverName">The name of the server on which to perform the action.</param>
        /// <param name="vmName">The name of the VM on which to set boot order.</param>
        /// <param name="first">The boot order entry to move to the head of the boot order list.</param>
        internal static void
        SetGeneration2BootOrder(
            string serverName,
            string vmName,
            string first)
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

                string[] prevBootOrder = (string[])vmSettings["BootSourceOrder"];
                string[] newBootOrder = new string[prevBootOrder.Length];

                //
                // Rebuild the order with the specified entry first
                //

                int index = 1;

                foreach (string entry in prevBootOrder)
                {
                    ManagementPath entryPath = new ManagementPath(entry);

                    using (ManagementObject entryObject = new ManagementObject(entryPath))
                    {
                        string entryDevicePath = entryObject["FirmwareDevicePath"].ToString();

                        if (string.Equals(first, entryDevicePath, StringComparison.OrdinalIgnoreCase))
                        {
                            newBootOrder[0] = entry;
                        }
                        else
                        {
                            newBootOrder[index++] = entry;
                        }
                    }
                }

                //
                // Modify the virtual system to use the new order.
                //

                vmSettings["BootSourceOrder"] = newBootOrder;

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

                    Console.WriteLine("Successfully modified boot order");
                    return;
                }
            }
        }
    }
}
