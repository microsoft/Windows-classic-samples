// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

namespace Microsoft.Samples.HyperV.FibreChannel
{
    using System;
    using System.Management;
    using System.Globalization;
    using Microsoft.Samples.HyperV.Common;

    static class CreateVirtualFcPortSample
    {
        /// <summary>
        /// Gets the default synthetic adapter settings object to use when adding a new adapter 
        /// to a virtual machine.
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The default synthetic adapter.</returns>
        private static ManagementObject
        GetDefaultSyntheticFcAdapter(
            ManagementScope scope)
        {
            ObjectQuery query = new ObjectQuery("SELECT * FROM Msvm_ResourcePool WHERE " +
                "ResourceSubType = 'Microsoft:Hyper-V:Synthetic FibreChannel Port' " +
                "AND Primordial = True");

            using (ManagementObjectSearcher queryExecute =
                   new ManagementObjectSearcher(scope, query))
            using (ManagementObject resourcePool =
                   WmiUtilities.GetFirstObjectFromCollection(queryExecute.Get()))
            {
                return FibreChannelUtilities.GetDefaultObjectFromResourcePool(resourcePool, scope);
            }
        }

        /// <summary>
        /// Adds a new synthetic FC Adapter device to the virtual machine. Note that the virtual
        /// machine must be in the power off state. Also note that the maximum number of FC
        /// Adapter devices that may be configured on a virtual machine is 4.
        /// </summary>
        /// <param name="virtualMachine">The name of the virtual machine.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The newly added synthetic adapter.</returns>
        private static ManagementObject
        AddSyntheticFcAdapter(
            ManagementObject virtualMachine,
            ManagementScope scope,
            WorldWideName wwnA,
            WorldWideName wwnB)
        {
            using (ManagementObject managementService =
                   WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementObject virtualMachineSettings =
                   WmiUtilities.GetVirtualMachineSettings(virtualMachine))
            //
            // Get the default Synthetic Adapter object, then modify its properties.
            //
            using (ManagementObject adapterToAdd = GetDefaultSyntheticFcAdapter(scope))
            {
                adapterToAdd["ElementName"] = FibreChannelUtilities.FcDeviceName;
                adapterToAdd["VirtualPortWWPN"] = wwnA.PortName;
                adapterToAdd["VirtualPortWWNN"] = wwnA.NodeName;
                adapterToAdd["SecondaryWWPN"] = wwnB.PortName;
                adapterToAdd["SecondaryWWNN"] = wwnB.NodeName;

                //
                // Now add it to the virtual machine.
                //
                using (ManagementBaseObject addAdapterInParams =
                       managementService.GetMethodParameters("AddResourceSettings"))
                {
                    addAdapterInParams["AffectedConfiguration"] = virtualMachineSettings.Path.Path;
                    addAdapterInParams["ResourceSettings"] = new string[] { adapterToAdd.GetText(
                                                                            TextFormat.WmiDtd20) };

                    using (ManagementBaseObject addAdapterOutParams =
                           managementService.InvokeMethod("AddResourceSettings",
                                                          addAdapterInParams,
                                                          null))
                    {
                        WmiUtilities.ValidateOutput(addAdapterOutParams, scope);

                        //
                        // Get the created FC adapter from the output parameters.
                        //
                        ManagementObject addedAdapter;
                        if (addAdapterOutParams["ResultingResourceSettings"] != null)
                        {
                            addedAdapter = new ManagementObject(
                                ((string[])addAdapterOutParams["ResultingResourceSettings"])[0]);

                            addedAdapter.Get();
                        }
                        else
                        {
                            using (ManagementObject job =
                                   new ManagementObject((string)addAdapterOutParams["Job"]))
                            {
                                addedAdapter =
                                    WmiUtilities.GetFirstObjectFromCollection(job.GetRelated(null,
                                        "Msvm_AffectedJobElement", null, null, null, null, false, null));
                            }
                        }

                        return addedAdapter;
                    }
                }
            }
        }

        /// <summary>
        /// For the given virtual machine, this sample adds a new FC Adapter device and 
        /// connects it to the specified SAN. Note that in order to add a new FC Adapter 
        /// device to the virtual machine, the virtual machine must be in the power off state.
        /// Also note that the maximum number of FC Adapter devices that may be configured
        /// on a virtual machine is 4.
        /// </summary>
        /// <param name="virtualMachineName">The name of the virtual machine.</param>
        /// <param name="sanName">The name of the virtual SAN to connect to.</param>
        /// <param name="wwnA">Virtual Port Address.</param>
        /// <param name="wwnB">Secondary Port Address.</param>
        private static void
        CreateVirtualFcPort(
            string virtualMachineName,
            string sanName,
            WorldWideName wwnA,
            WorldWideName wwnB)
        {
            Console.WriteLine("Adding Virtual FC Port to VM {0} :", virtualMachineName);
            Console.WriteLine("\tVirtualWWPN: {0}, VirtualWWNN {1}", wwnA.PortName, wwnA.NodeName);
            Console.WriteLine("\tSecondaryWWPN: {0}, SecondaryWWNN {1}", wwnB.PortName, wwnB.NodeName);
            Console.WriteLine("\tProviding connectivity to Virtual SAN {0}", sanName);

            ManagementScope scope = FibreChannelUtilities.GetFcScope();

            using (ManagementObject managementService = 
                   WmiUtilities.GetVirtualMachineManagementService(scope))

            //
            // Find the virtual machine we want to connect.
            //
            using (ManagementObject virtualMachine = 
                   WmiUtilities.GetVirtualMachine(virtualMachineName, scope))

            //
            // Get the virtual machine's settings object which is used
            // to make configuration changes.
            //
            using (ManagementObject virtualMachineSettings =
                   WmiUtilities.GetVirtualMachineSettings(virtualMachine))

            //
            // Add a new synthetic FC Adapter device to the virtual machine.
            //
            using (ManagementObject syntheticAdapter = AddSyntheticFcAdapter(virtualMachine,
                                                                             scope,
                                                                             wwnA,
                                                                             wwnB))

            //
            // Now that we have added a FC adapter to the virtual machine we can configure its
            // connection settings.
            //
            using (ManagementObject connectionSettingsToAdd =
                   FibreChannelUtilities.GetDefaultFcPortAllocationSettingData(scope, sanName))
            {
                connectionSettingsToAdd["PoolID"] = sanName;
                connectionSettingsToAdd["Parent"] = syntheticAdapter.Path.Path;

                //
                // Now add the connection settings.
                //
                using (ManagementBaseObject addConnectionInParams =
                       managementService.GetMethodParameters("AddResourceSettings"))
                {
                    addConnectionInParams["AffectedConfiguration"] = virtualMachineSettings.Path.Path;
                    addConnectionInParams["ResourceSettings"] =
                        new string[] { connectionSettingsToAdd.GetText(TextFormat.WmiDtd20) };

                    using (ManagementBaseObject addConnectionOutParams =
                           managementService.InvokeMethod("AddResourceSettings",
                                                          addConnectionInParams,
                                                          null))
                    {
                        WmiUtilities.ValidateOutput(addConnectionOutParams, scope, true, true);
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully created virtual FC port on VM '{0}' and connected to san '{1}'.",
                virtualMachineName, sanName));
        }

        /// <summary>
        /// Entry point for the CreateVirtualFcPort sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length < 2 || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: CreateVirtualFcPort <VM Name> <SAN Name> [WWPN-A WWNN-A WWPN-B WWNN-B]");
                Console.WriteLine();
                Console.WriteLine("Note:  The 2 sets of WWNs for the virtual port are optional and if omitted");
                Console.WriteLine("       will be auto generated using the WWPN Generator and the CurrentWWNNAddress.");
                return;
            }

            try
            {
                WorldWideName wwnA = new WorldWideName();
                WorldWideName wwnB = new WorldWideName();
                if (args.Length == 6)
                {
                    wwnA.PortName = args[2];
                    wwnA.NodeName = args[3];
                    wwnB.PortName = args[4];
                    wwnB.NodeName = args[5];
                }
                else
                {
                    Console.WriteLine("Auto Generating WWNs for Virtual FC Port to be added...");
                    FibreChannelUtilities.GenerateWorldWideNames(
                        FibreChannelUtilities.GetFcScope(),
                        ref wwnA,
                        ref wwnB);
                }

                CreateVirtualFcPort(args[0], args[1], wwnA, wwnB);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to create a virtual FC port. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
