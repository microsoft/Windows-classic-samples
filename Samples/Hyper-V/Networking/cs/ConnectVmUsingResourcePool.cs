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

    static class ConnectVmUsingResourcePoolSample
    {
        /// <summary>
        /// For the given virtual machine, this sample adds a new Network Adapter device and 
        /// creates a dynamic connection to the specified Ethernet connection resource pool.
        /// When the virtual machine is powered on, Hyper-V will dynamically connect the virtual
        /// machine to one of the switches that have been added to that resource pool.
        /// Note that in order to add a new Network Adapter device to the virtual machine, the
        /// virtual machine must be in the power off state. Also note that the maximum number of
        /// Network Adapter devices that may be configured on a virtual machine is 8.
        /// </summary>
        /// <param name="virtualMachineName">The name of the virtual machine.</param>
        /// <param name="resourcePoolName">The name of the resource pool to use for the connection.</param>
        static void
        ConnectVmUsingResourcePool(
            string virtualMachineName,
            string resourcePoolName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))

            //
            // Find the virtual machine we want to connect.
            //
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(virtualMachineName, scope))

            //
            // As a sanity check, verify that the specified resource pool exists by querying for it.
            //
            using (ManagementObject resourcePool = WmiUtilities.GetResourcePool(
                "33",
                "Microsoft:Hyper-V:Ethernet Connection",
                resourcePoolName,
                scope))

            //
            // Get the virtual machine's settings object which is used to make configuration changes.
            //
            using (ManagementObject virtualMachineSettings = WmiUtilities.GetVirtualMachineSettings(virtualMachine))

            //
            // Add a new synthetic Network Adapter device to the virtual machine.
            //
            using (ManagementObject syntheticAdapter = NetworkingUtilities.AddSyntheticAdapter(virtualMachine, scope))

            //
            // Now that we have added a network adapter to the virtual machine, we can configure its
            // connection settings.
            //
            using (ManagementObject connectionSettingsToAdd =
                  NetworkingUtilities.GetDefaultEthernetPortAllocationSettingData(scope))
            {
                connectionSettingsToAdd["Parent"] = syntheticAdapter.Path.Path;

                //
                // Rather than specifying which switch to connect to, we specify which pool to look in.
                //
                connectionSettingsToAdd["PoolId"] = resourcePoolName;

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
                           managementService.InvokeMethod("AddResourceSettings", addConnectionInParams, null))
                    {
                        WmiUtilities.ValidateOutput(addConnectionOutParams, scope);
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully created dynamic connection from virtual machine '{0}' to resource pool '{1}'.",
                virtualMachineName, resourcePoolName));
        }

        /// <summary>
        /// Entry point for the connect virtual machine using resource pool sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length == 0 || args[0] == "/?")
            {
                Console.WriteLine("Usage: ConnectVmUsingResourcePool VirtualMachineName PoolID \n");
                Console.WriteLine("Example: ConnectVmUsingResourcePool MyVm MyPool");
                return;
            }

            try
            {
                ConnectVmUsingResourcePool(args[0], args[1]);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to create the connection. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
