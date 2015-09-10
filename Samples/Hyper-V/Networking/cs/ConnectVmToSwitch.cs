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

    static class ConnectVmToSwitchSample
    {
        /// <summary>
        /// For the given virtual machine, this sample adds a new Network Adapter device and 
        /// connects it to the specified switch. Note that in order to add a new Network Adapter 
        /// device to the virtual machine, the virtual machine must be in the power off state.
        /// Also note that the maximum number of Network Adapter devices that may be configured
        /// on a virtual machine is 8.
        /// </summary>
        /// <param name="virtualMachineName">The name of the virtual machine.</param>
        /// <param name="switchName">The name of the switch to connect to.</param>
        static void
        ConnectVmToSwitch(
            string virtualMachineName,
            string switchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))

            //
            // Find the Ethernet switch we want to connect to.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))

            //
            // Find the virtual machine we want to connect.
            //
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(virtualMachineName, scope))

            //
            // Get the virtual machine's settings object which is used to make configuration changes.
            //
            using (ManagementObject virtualMachineSettings = WmiUtilities.GetVirtualMachineSettings(virtualMachine))

            //
            // Add a new synthetic Network Adapter device to the virtual machine.
            //
            using (ManagementObject syntheticAdapter = NetworkingUtilities.AddSyntheticAdapter(virtualMachine, scope))

            //
            // Now that we have added a network adapter to the virtual machine we can configure its
            // connection settings.
            //
            using (ManagementObject connectionSettingsToAdd = 
                  NetworkingUtilities.GetDefaultEthernetPortAllocationSettingData(scope))
            {
                connectionSettingsToAdd["Parent"] = syntheticAdapter.Path.Path;
                connectionSettingsToAdd["HostResource"] = new string[] { ethernetSwitch.Path.Path };

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
                "Successfully connected virtual machine '{0}' to switch '{1}'.", 
                virtualMachineName, switchName));
        }

        /// <summary>
        /// Disconnects the virtual machine from the specified switch.
        /// </summary>
        /// <param name="virtualMachineName">The name of the virtual machine.</param>
        /// <param name="switchName">The name of the switch to disconnect from.</param>
        static void
        DisconnectVmFromSwitch(
            string virtualMachineName,
            string switchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))

            //
            // Find the Ethernet switch we want to disconnect from.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))

            //
            // Find the virtual machine we want to disconnect.
            //
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(virtualMachineName, scope))
            {
                //
                // Find all of the connections to this switch.
                //
                IList<ManagementObject> connectionsToSwitch = 
                    NetworkingUtilities.FindConnectionsToSwitch(virtualMachine, ethernetSwitch, scope);

                //
                // Now that we have found all of the connections to the switch we can go through and 
                // disable each connection so that it is not connected to anything. Note that you can also
                // delete the connection object, but disabling the connection is sometimes preferable because
                // it preserves all of the connection's configuration options along with any metrics
                // associated with the connection.
                //
                try
                {
                    foreach (ManagementObject connection in connectionsToSwitch)
                    {
                        connection["EnabledState"] = 3; // 3 means "Disabled"
    
                        using (ManagementBaseObject inParams = 
                               managementService.GetMethodParameters("ModifyResourceSettings"))
                        {
                            inParams["ResourceSettings"] = new string[] { connection.GetText(TextFormat.WmiDtd20) };
        
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
                    // Dispose of the connectionsToSwitch.
                    foreach (ManagementObject connection in connectionsToSwitch)
                    {
                        connection.Dispose();
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully disconnected virtual machine '{0}' from switch '{1}'.",
                virtualMachineName, switchName));
        }

        /// <summary>
        /// Modifies the connections of the specified virtual machine from the provided current switch 
        /// to a new switch.
        /// </summary>
        /// <param name="virtualMachineName">The virtual machine name.</param>
        /// <param name="currentSwitchName">The name of the switch we are disconnecting from.</param>
        /// <param name="newSwitchName">The name of the switch we are connecting to.</param>
        static void
        ModifyConnection(
            string virtualMachineName,
            string currentSwitchName,
            string newSwitchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))

            //
            // Find the Ethernet switch we want to disconnect from and the one we want to connect to.
            //
            using (ManagementObject currentEthernetSwitch = NetworkingUtilities.FindEthernetSwitch(currentSwitchName, scope))
            using (ManagementObject newEthernetSwitch = NetworkingUtilities.FindEthernetSwitch(newSwitchName, scope))

            //
            // Find the virtual machine we want to modify.
            //
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(virtualMachineName, scope))
            {
                //
                // Find the connections to the current switch.
                // Note that this method finds all connections to the switch, including those connections
                // that are configured to dynamically select a switch from a switch resource pool and
                // reconfigures them to always connect to the new switch. If you only want to modify 
                // connections that are configured with a hard-affinity to the original switch, modify the
                // NetworkingUtilities.FindConnectionsToSwitch method.
                //
                IList<ManagementObject> currentConnections = 
                    NetworkingUtilities.FindConnectionsToSwitch(virtualMachine, currentEthernetSwitch, scope);

                //
                // Set each connection to connect to the new switch. If the virtual machine is currently
                // running, then it will immediately connect to the new switch. If it is not running, then
                // it will connect to the switch when it is turned on.
                //
                try
                {
                    foreach (ManagementObject connection in currentConnections)
                    {
                        connection["HostResource"] = new string[] { newEthernetSwitch.Path.Path };
    
                        using (ManagementBaseObject inParams = 
                               managementService.GetMethodParameters("ModifyResourceSettings"))
                        {
                            inParams["ResourceSettings"] = new string[] { connection.GetText(TextFormat.WmiDtd20) };
        
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
                    // Dispose of the connections.
                    foreach (ManagementObject connection in currentConnections)
                    {
                        connection.Dispose();
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully modified virtual machine '{0}' so that every connection to '{1}' is now " +
                "connected to '{2}'.", virtualMachineName, currentSwitchName, newSwitchName));
        }

        /// <summary>
        /// Entry point for the connect virtual machine to switch sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length == 0 || args[0] == "/?")
            {
                DisplayUsage();
                return;
            }

            string sample = args[0];
            if (string.Equals(sample, "Connect", StringComparison.OrdinalIgnoreCase))
            {
                if (args.Length != 3)
                {
                    DisplayUsage();
                    return;
                }

                try
                {
                    ConnectVmToSwitch(args[1], args[2]);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Failed to connect to the switch. Error message details:\n");
                    Console.WriteLine(ex.Message);
                }
            }
            else if (string.Equals(sample, "Disconnect", StringComparison.OrdinalIgnoreCase))
            {
                if (args.Length != 3)
                {
                    DisplayUsage();
                    return;
                }

                try
                {
                    DisconnectVmFromSwitch(args[1], args[2]);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Failed to disconnect the virtual machine from the switch. Error message details:\n");
                    Console.WriteLine(ex.Message);
                }
            }
            else if (string.Equals(sample, "Modify", StringComparison.OrdinalIgnoreCase))
            {
                if (args.Length != 4)
                {
                    Console.WriteLine("Must supply the names of both the switch we are disconnecting from and " +
                        "the switch we are connecting to.\n");
                    Console.WriteLine("Example: ConnectVmToSwitch Modify MyVm CurrentSwitch NewSwitch");
                    return;
                }

                try
                {
                    ModifyConnection(args[1], args[2], args[3]);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Failed to modify the virtual machine's connection to the new switch. Error message details:\n");
                    Console.WriteLine(ex.Message);
                }
            }
            else
            {
                DisplayUsage();
            }
        }

        /// <summary>
        /// Displays the command line usage for the sample.
        /// </summary>
        static void
        DisplayUsage()
        {
            Console.WriteLine("Usage: ConnectVmToSwitch (Connect|Disconnect|Modify) VirtualMachineName SwitchName [NewSwitchName]\n");
            Console.WriteLine("When connecting, this sample adds a new Network Adapter to the virtual machine, " +
                "which requires that the virtual machine is off");
            Console.WriteLine("When disconnecting, or modifying a current connection, the virtual machine can be running\n");
            Console.WriteLine("The NewSwitchName parameter is only used when modifying a connection from the current SwitchName to the NewSwitchName\n");
            Console.WriteLine("Example: ConnectVmToSwitch Connect MyVm MySwitch");
        }
    }
}
