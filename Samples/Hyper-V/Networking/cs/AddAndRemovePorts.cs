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

    static class AddAndRemovePortsSample
    {
        /// <summary>
        /// Adds an external port to the specified switch that is connected to the specified external 
        /// network.
        /// </summary>
        /// <param name="switchName">The name of the switch to add ports to.</param>
        /// <param name="externalAdapterName">The name of the external network to connect to.</param>
        static void
        AddPorts(
            string switchName,
            string externalAdapterName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            string ethernetSwitchSettingPath;
            string portToCreateText;

            ObjectQuery externalAdapterQuery = new ObjectQuery(string.Format(CultureInfo.InvariantCulture,
                "select * from Msvm_ExternalEthernetPort where Name=\"{0}\"", externalAdapterName));
            
            //
            // When modifying the switch, we need to pass in its configuration object.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))
            using (ManagementObject ethernetSwitchSetting = WmiUtilities.GetFirstObjectFromCollection(
                ethernetSwitch.GetRelated("Msvm_VirtualEthernetSwitchSettingData",
                    "Msvm_SettingsDefineState",
                    null, null, null, null, false, null)))
            {
                ethernetSwitchSettingPath = ethernetSwitchSetting.Path.Path;
            }

            //
            // Get the external adapter that we want to add a connection to.
            //
            using (ManagementObjectSearcher externalAdapterExecute = 
                   new ManagementObjectSearcher(scope, externalAdapterQuery))
            using (ManagementObjectCollection externalAdapterCollection = externalAdapterExecute.Get())
            {
                if (externalAdapterCollection.Count == 0)
                {
                    throw new ManagementException(string.Format(CultureInfo.InvariantCulture,
                        "There is no external adapter with the name {0}.", 
                        externalAdapterName));
                }

                using (ManagementObject externalAdapter = 
                       WmiUtilities.GetFirstObjectFromCollection(externalAdapterCollection))

                //
                // Get the default Msvm_EthernetPortAllocationSettingData instance, which we can use to 
                // configure our external port connection for the switch.
                // Use the same switch name, appended with "_External", for the port name. 
                // You can use any port name that you like.
                //
                using (ManagementObject portToCreate = 
                       NetworkingUtilities.GetDefaultEthernetPortAllocationSettingData(scope))
                {
                    portToCreate["ElementName"] = switchName + "_External";
                    portToCreate["HostResource"] = new string[] { externalAdapter.Path.Path };

                    portToCreateText = portToCreate.GetText(TextFormat.WmiDtd20);
                }
            }
    
            //
            // Now add the port connection to the switch.
            //
            using (ManagementObject switchService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))
            using (ManagementBaseObject inParams =
                    switchService.GetMethodParameters("AddResourceSettings"))
            {
                inParams["AffectedConfiguration"] = ethernetSwitchSettingPath;
                inParams["ResourceSettings"] = new string[] { portToCreateText };

                using (ManagementBaseObject outParams =
                        switchService.InvokeMethod("AddResourceSettings", inParams, null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope);
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                            "We added an external port connection to '{0}' on the switch '{1}'.",
                            externalAdapterName, switchName));
        }

        /// <summary>
        /// Finds and removes the external and internal ports of the specified switch. After this 
        /// method executes, the switch will be "Private".
        /// </summary>
        /// <param name="switchName">The name of the switch to remove ports from.</param>
        static void
        RemovePorts(
            string switchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            List<string> portsToDelete = new List<string>();
            
            //
            // Find the switch that we want to modify.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))

            //
            // Find its internal and/or external ports to delete, but don't delete any ports that 
            // are connected to virtual machines.
            // Internal ports can be recognized because their HostResource property is set to the 
            // host computer. 
            // Likewise, external ports can be recognized through their HostResource property, 
            // which is always set to the Msvm_ExternalEthernetPort it is connected to.
            //
            using (ManagementObjectCollection ports = ethernetSwitch.GetRelated("Msvm_EthernetSwitchPort",
                "Msvm_SystemDevice",
                null, null, null, null, false, null))
            {
                foreach (ManagementObject port in ports)
                using (port)
                {
                    //
                    // The port's connection settings are stored on its related 
                    // Msvm_EthernetPortAllocationSettingData object.
                    //
                    using (ManagementObject portSettings = WmiUtilities.GetFirstObjectFromCollection(
                        port.GetRelated("Msvm_EthernetPortAllocationSettingData",
                            "Msvm_ElementSettingData",
                            null, null, null, null, false, null)))
                    {
                        string[] hostResource = (string[])portSettings["HostResource"];

                        if (hostResource != null && hostResource.Length > 0)
                        {
                            ManagementPath hostResourcePath = new ManagementPath(hostResource[0]);

                            if (string.Equals(hostResourcePath.ClassName, "Msvm_ComputerSystem",
                                StringComparison.OrdinalIgnoreCase))
                            {
                                // This is the internal port. Add it to the list to delete.
                                portsToDelete.Add(portSettings.Path.Path);
                            }
                            else if (string.Equals(hostResourcePath.ClassName, "Msvm_ExternalEthernetPort",
                                     StringComparison.OrdinalIgnoreCase))
                            {
                                // This is the external port. Add it to the list to delete.
                                portsToDelete.Add(portSettings.Path.Path);
                            }
                        }
                    }
                }
            }

            if (portsToDelete.Count == 0)
            {
                throw new ManagementException(string.Format(CultureInfo.InvariantCulture,
                    "The switch '{0}' does not have any internal or external ports to remove.", 
                    switchName));
            }

            // 
            // Now that we have the ports we want to delete we can call the RemoveResourceSettings
            // method.
            //
            using (ManagementObject switchService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))
            using (ManagementBaseObject inParams = switchService.GetMethodParameters("RemoveResourceSettings"))
            {
                inParams["ResourceSettings"] = portsToDelete.ToArray();
    
                using (ManagementBaseObject outParams = switchService.InvokeMethod("RemoveResourceSettings", inParams, null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope);
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "We removed the internal and external ports from switch '{0}' successfully.", switchName));
        }

        /// <summary>
        /// Entry point for the add remove sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length == 0 || args[0] == "/?")
            {
                Console.WriteLine("Usage: AddAndRemovePorts (Add|Remove) SwitchName [ExternalNetwork]\n");
                Console.WriteLine("If \"Add\" is specified we add an external connection to ExternalNetwork");
                Console.WriteLine("If \"Remove\" is specified we remove the ports of the specified switch\n");
                Console.WriteLine("Example: AddAndRemovePorts Add MySwitch MyNetworkConnection");
                return;
            }

            bool addPorts = true;

            try
            {
                string type = args[0];
                if (string.Equals(type, "add", StringComparison.CurrentCultureIgnoreCase))
                {
                    if (args.Length < 3)
                    {
                        Console.WriteLine("Must specify both the switch and the external network connection to add\n");
                        Console.WriteLine("Example: AddAndRemovePorts Add MySwitch MyNetworkConnection");
                        return;
                    }

                    AddPorts(args[1], args[2]);
                }
                else if (string.Equals(type, "remove", StringComparison.CurrentCultureIgnoreCase))
                {
                    if (args.Length < 2)
                    {
                        Console.WriteLine("Must supply the name of the switch to remove ports from\n");
                        Console.WriteLine("Example: AddAndRemovePorts Remove MySwitch");
                        return;
                    }

                    addPorts = false;
                    RemovePorts(args[1]);
                }
                else
                {
                    Console.WriteLine("First parameter must be either \"Add\" or \"Remove\"");
                    return;
                }
            }
            catch (Exception ex)
            {
                if (addPorts)
                {
                    Console.WriteLine("Failed to add the external connection to the switch. Error message details:\n");
                }
                else
                {
                    Console.WriteLine("Failed to remove the port connections from the switch. Error message details:\n");
                }

                Console.WriteLine(ex.Message);
            }
        }
    }
}
