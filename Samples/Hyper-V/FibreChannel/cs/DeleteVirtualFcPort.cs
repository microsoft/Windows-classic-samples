// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

namespace Microsoft.Samples.HyperV.FibreChannel
{
    using System;
    using System.Collections.Generic;
    using System.Management;
    using System.Globalization;
    using Microsoft.Samples.HyperV.Common;

    static class DeleteVirtualFcPortSample
    {
        /// <summary>
        /// Removes a synthetic FC Adapter device of a virtual machine. Note that the virtual
        /// machine must be in the power off state.
        /// </summary>
        /// <param name="virtualMachine">The name of the virtual machine.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>A collection of Msvm_SyntheticFcPortSettingData instances.</returns>
        private static string[]
        GetResourcesToRemove(
            ManagementObject managementService,
            ManagementObject virtualMachine,
            WorldWideName wwnA,
            WorldWideName wwnB)
        {
            using (ManagementObject virtualMachineSettings =
                   WmiUtilities.GetVirtualMachineSettings(virtualMachine))
            {
                ManagementObjectCollection fcPortCollection = virtualMachineSettings.GetRelated(
                    "Msvm_SyntheticFcPortSettingData",
                    "Msvm_VirtualSystemSettingDataComponent",
                    null, null, null, null, false, null);

                List<string> portsToRemove = new List<string>();
                foreach (ManagementObject fcPort in fcPortCollection)
                {
                    string primaryWwpn = fcPort["VirtualPortWWPN"].ToString();
                    string primaryWwnn = fcPort["VirtualPortWWNN"].ToString();
                    string secondaryWwpn = fcPort["SecondaryWWPN"].ToString();
                    string secondaryWwnn = fcPort["SecondaryWWNN"].ToString();

                    if (string.Equals(primaryWwpn, wwnA.PortName, StringComparison.OrdinalIgnoreCase) &&
                        string.Equals(primaryWwnn, wwnA.NodeName, StringComparison.OrdinalIgnoreCase) &&
                        string.Equals(secondaryWwpn, wwnB.PortName, StringComparison.OrdinalIgnoreCase) &&
                        string.Equals(secondaryWwnn, wwnB.NodeName, StringComparison.OrdinalIgnoreCase))
                    {
                        portsToRemove.Add(fcPort.Path.Path);
                    }
                }
                return portsToRemove.ToArray();
            }
        }

        /// <summary>
        /// Deletes virtual FC ports, specified by the two sets of WorldWideNames, for a particular VM.
        /// </summary>
        /// <param name="virtualMachineName">Name of the VM whose virtual FC Ports we want to delete</param>
        /// <param name="wwnA">Virtual Port WWN</param>
        /// <param name="wwnB">Secondary WWN.</param>
        private static void
        DeleteVirtualFcPort(
            string virtualMachineName,
            WorldWideName wwnA,
            WorldWideName wwnB)
        {
            Console.WriteLine("Removing virtual FC port(s) with following WWNs from VM {0}:", virtualMachineName);
            Console.WriteLine("\tVirtualPortWWPN {0}, VirtualPortWWNN {1}", wwnA.PortName, wwnA.NodeName);
            Console.WriteLine("\tSecondaryWWPN {0}, SecondaryWWNN {1}", wwnB.PortName, wwnB.NodeName);

            ManagementScope scope = FibreChannelUtilities.GetFcScope();

            using (ManagementObject managementService =
                            WmiUtilities.GetVirtualMachineManagementService(scope))
            //
            // Find the virtual machine we want to connect.
            //
            using (ManagementObject virtualMachine =
                            WmiUtilities.GetVirtualMachine(virtualMachineName, scope))
            {
                string[] portsToRemove = GetResourcesToRemove(managementService,
                                                              virtualMachine,
                                                              wwnA,
                                                              wwnB);

                if (portsToRemove.Length == 0)
                {
                    Console.WriteLine("The specified world wide names were not found in VM {0}", virtualMachineName);
                    return;
                }

                using (ManagementBaseObject inParams =
                                managementService.GetMethodParameters("RemoveResourceSettings"))
                {
                    inParams["ResourceSettings"] = portsToRemove;

                    using (ManagementBaseObject addAdapterOutParams =
                                managementService.InvokeMethod("RemoveResourceSettings",
                                                                inParams,
                                                                null))
                    {
                       WmiUtilities.ValidateOutput(addAdapterOutParams, scope, true, true);
                    }
                }
            }

            Console.WriteLine("Successfully deleted the virtual FC port on VM {0}.", virtualMachineName);
        }

        /// <summary>
        /// Entry point for the DeleteVirtualFcPort sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length != 5 || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: DeleteVirtualFcPort <VM Name> <WWPN-A> <WWNN-A> <WWPN-B> <WWNN-B>");
                return;
            }

            try
            {
                WorldWideName wwnA = new WorldWideName();
                wwnA.PortName = args[1];
                wwnA.NodeName = args[2];

                WorldWideName wwnB = new WorldWideName();
                wwnB.PortName = args[3];
                wwnB.NodeName = args[4];

                DeleteVirtualFcPort(args[0], wwnA, wwnB);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to delete virtual FC port. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
