// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.VmOperations
{
    using System;
    using System.Globalization;
    using System.Management;
    using System.Collections.Generic;
    using Microsoft.Samples.HyperV.Common;

    static class LinuxAffinityUtilities
    {
        /// <summary>
        /// Finds the first VM matching vmName and injects a NMI into it, displaying
        /// any warnings produced.
        /// </summary>
        /// <param name="vmName">The name of the VM.</param>
        internal static void
        InjectNmi(
            string vmName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            {
                Console.WriteLine("Injecting NMI into Virtual Machine \"{0}\" ({1})...",
                        vm["ElementName"], vm["Name"]);

                using (ManagementBaseObject outParams =
                    vm.InvokeMethod("InjectNonMaskableInterrupt", null, null))
                {
                    if (WmiUtilities.ValidateOutput(outParams, scope))
                    {
                        Console.WriteLine("Inject NMI succeeded.\n");
                    }
                }
            }
        }

        /// <summary>
        /// Configures the MMIO gap size of the specified VM to the gapSize value,
        /// displaying any warnings produced.
        /// </summary>
        /// <param name="vmName">The name of the VM.</param>
        /// <param name="gapSize">The size of the MMIO gap in MB</param>
        internal static void 
        ConfigureMmioGap(
            string vmName, 
            uint gapSize)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObject vssd = WmiUtilities.GetVirtualMachineSettings(vm))
            using (ManagementObject vmms = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementBaseObject inParams = vmms.GetMethodParameters("ModifySystemSettings"))
            {
                Console.WriteLine("Configuring MMIO gap size of Virtual Machine \"{0}\" ({1}) " +
                        "to {2} MB...", vm["ElementName"], vm["Name"], gapSize);

                vssd["LowMmioGapSize"] = gapSize;
                inParams["SystemSettings"] = vssd.GetText(TextFormat.CimDtd20);

                using (ManagementBaseObject outParams =
                    vmms.InvokeMethod("ModifySystemSettings", inParams, null))
                {
                    if (WmiUtilities.ValidateOutput(outParams, scope))
                    {
                        Console.WriteLine("Configuring MMIO gap size succeeded.\n");
                    }
                }
            }
        }
    }
}
