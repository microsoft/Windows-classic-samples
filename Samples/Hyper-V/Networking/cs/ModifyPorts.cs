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
    using Microsoft.Samples.HyperV.Common;

    static class ModifyPortsSample
    {
        /// <summary>
        /// Given a switch that is currently externally connected, modifies the switch's port
        /// so that it binds to a new external adapter.
        /// </summary>
        /// <param name="switchName">The name of an existing switch with an external connection.</param>
        /// <param name="newExternalAdapterName">The new exteranl adapter to bind the switch to.</param>
        static void
        ModifyPorts(
            string switchName,
            string newExternalAdapterName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            ObjectQuery externalAdapterQuery = new ObjectQuery(string.Format(CultureInfo.InvariantCulture,
                "select * from Msvm_ExternalEthernetPort where Name=\"{0}\"", newExternalAdapterName));

            //
            // Find the switch that we want to modify.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))

            // 
            // Get the external adapter we are connecting to. 
            //
            using (ManagementObjectSearcher externalAdapterExecute = new ManagementObjectSearcher(scope, externalAdapterQuery))
            using (ManagementObjectCollection externalAdapterCollection = externalAdapterExecute.Get())
            {
                if (externalAdapterCollection.Count == 0)
                {
                    throw new ManagementException("There is no external adapter with the name: " + newExternalAdapterName);
                }

                string externalAdapterPath;
                string externalPortSettingsText = null;

                using (ManagementObject externalAdapter = WmiUtilities.GetFirstObjectFromCollection(externalAdapterCollection))
                {
                    externalAdapterPath = externalAdapter.Path.Path;
                }

                //
                // Find the switch's current external port.
                //

                using (ManagementObjectCollection ports = ethernetSwitch.GetRelated("Msvm_EthernetSwitchPort",
                    "Msvm_SystemDevice",
                    null, null, null, null, false, null))
                foreach (ManagementObject port in ports)
                using (port)
                {
                    //
                    // The port's connection settings are stored on its related 
                    // Msvm_EthernetPortAllocationSettingData object.
                    // The external port is the one connected to a Msvm_ExternalEthernetPort 
                    // through its HostResource property.
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

                            if (string.Equals(hostResourcePath.ClassName, "Msvm_ExternalEthernetPort",
                                StringComparison.OrdinalIgnoreCase))
                            {
                                //
                                // Validate that it isn't already connected to the external adapter we
                                // are trying to change it to.
                                //
                                if (string.Equals(hostResourcePath.Path, externalAdapterPath,
                                    StringComparison.OrdinalIgnoreCase))
                                {
                                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                                        "The switch '{0}' is already connected to '{1}'.", 
                                        switchName, newExternalAdapterName));
                                }

                                //
                                // Now that we have the switch's external port, we can modify it so that it 
                                // is connected to newExternalAdapterName.
                                // 

                                portSettings["HostResource"] = new string[] { externalAdapterPath };
                                externalPortSettingsText = portSettings.GetText(TextFormat.WmiDtd20);
                                break;
                            }
                        }
                    }
                }

                if (externalPortSettingsText == null)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "The switch '{0}' is not connected to an external network.", switchName));
                }

                using (ManagementObject switchService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))
                using (ManagementBaseObject inParams = switchService.GetMethodParameters("ModifyResourceSettings"))
                {
                    inParams["ResourceSettings"] = new string[] { externalPortSettingsText };
    
                    using (ManagementBaseObject outParams = 
                           switchService.InvokeMethod("ModifyResourceSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully bound the switch '{0}' to the external network '{1}'.",
                switchName, newExternalAdapterName));
        }

        /// <summary>
        /// Entry point for the ModifyPorts sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length != 2 || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: ModifyPorts SwitchName ExternalNetwork\n");
                Console.WriteLine("Modifies a switch so that its external port connects to a different external network adapter.");
                Console.WriteLine("SwitchName must be the name of a switch that is currently externally connected.\n");
                Console.WriteLine("Example: ModifyPorts MySwitch MyNetworkConnection");
                return;
            }

            try
            {
                ModifyPorts(args[0], args[1]);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to modify the ports of the switch. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
