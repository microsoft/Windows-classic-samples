// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Networking
{
    using System;
    using System.Management;
    using System.Globalization;
    using System.Collections.Generic;
    using Microsoft.Samples.HyperV.Common;

    static class SyntheticEthernetPortPropertiesSample
    {
        static class PropertyNames
        {
            public const string ClusterMonitored = "ClusterMonitored";
        }

        /// <summary>
        /// Modifies ClusterMonitored property of a synthetic ethernet port.
        /// </summary>
        /// <param name="virtualMachineName">The name of the virtual machine.</param>
        /// <param name="onOff">Whether to enable or disable cluster monitoring.</param>
        static void
        ModifyClusterMonitored(
            string virtualMachineName,
            bool onOff)
        {
            // Get management scope
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            int syntheticPortCount = 0;

            // Get virtual system management service
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))

            // Find the virtual machine we want to modify the synthetic ethernet ports of.
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(virtualMachineName, scope))

            // Find the virtual system setting data
            using (ManagementObject virtualMachineSettings = WmiUtilities.GetVirtualMachineSettings(virtualMachine))

            // Find all synthetic ethernet ports for the given virtual machine.
            using (ManagementObjectCollection syntheticPortSettings = virtualMachineSettings.GetRelated(
                                                                "Msvm_SyntheticEthernetPortSettingData",
                                                                "Msvm_VirtualSystemSettingDataComponent",
                                                                null, null, null, null, false, null))
            {
                syntheticPortCount = syntheticPortSettings.Count;

                try
                {
                    // Modify each port setting to update the property.
                    foreach (ManagementObject syntheticEthernetPortSetting in syntheticPortSettings)
                    {
                        syntheticEthernetPortSetting[PropertyNames.ClusterMonitored] = onOff;

                        // Now apply the changes to the Hyper-V server.
                        using (ManagementBaseObject inParams = managementService.GetMethodParameters("ModifyResourceSettings"))
                        {
                            inParams["ResourceSettings"] = new string[] { syntheticEthernetPortSetting.GetText(TextFormat.CimDtd20) };

                            using (ManagementBaseObject outParams =
                                    managementService.InvokeMethod("ModifyResourceSettings", inParams, null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }
                        }
                    }
                }
                finally
                {
                    // Dispose of the synthetic ethernet port settings.
                    foreach (ManagementObject syntheticEthernetPortSetting in syntheticPortSettings)
                    {
                        syntheticEthernetPortSetting.Dispose();
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully modified virtual machine '{0}' so that each of its {1} synthetic ethernet port(s) " +
                "has {2} property modified to {3}.",
                virtualMachineName, syntheticPortCount, PropertyNames.ClusterMonitored, onOff));
        }

        /// <summary>
        /// Entry point for the synthetic ethernet port properties sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length < 3 || args[0] == "/?")
            {
                // Right now only support boolean type of property.
                Console.WriteLine("Usage: SyntheticEthernetPortProperties VmName PropertyName PropertyValue\n");
                Console.WriteLine("For {0} property:\n", PropertyNames.ClusterMonitored);
                Console.WriteLine("\tIf \"on\" is specified for property value we turn on cluster monitoring for each");
                Console.WriteLine("\tsynthetic ethernet port from the specified virtual machine");
                Console.WriteLine("\tIf \"off\" is specified we turn off cluster monitoring for each");
                Console.WriteLine("\tsynthetic ethernet port from the specified virtual machine");
                Console.WriteLine("Example: SyntheticEthernetPortProperties MyVm {0} on", PropertyNames.ClusterMonitored);
                return;
            }

            if (string.Equals(args[1], PropertyNames.ClusterMonitored, StringComparison.CurrentCultureIgnoreCase))
            {
                string operation = args[2];

                bool onOff = false;
                if (string.Equals(operation, "on", StringComparison.CurrentCultureIgnoreCase))
                {
                    onOff = true;
                }
                else if (string.Equals(operation, "off", StringComparison.CurrentCultureIgnoreCase))
                {
                    onOff = false;
                }
                else
                {
                    Console.WriteLine("Third parameter must be \"on\" or \"off\" for property {0}.", PropertyNames.ClusterMonitored);
                    return;
                }

                try
                {
                    ModifyClusterMonitored(args[0], onOff);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Failed to set {0} property. Error message details:\n", PropertyNames.ClusterMonitored);
                    Console.WriteLine(ex.Message);
                }
            }
            else
            {
                // Note: future new properties can be added.
                Console.WriteLine("Unsupported property {0}.\n", args[1]);

                // Print supported property list.
                Console.WriteLine("Supported properties:\n");
                Console.WriteLine("\t{0}\n", PropertyNames.ClusterMonitored);
            }
        }
    }
}
