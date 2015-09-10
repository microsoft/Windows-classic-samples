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

    static class NetworkingUtilities
    {
        /// <summary>
        /// Advanced port feature type.
        /// </summary>
        internal enum PortFeatureType
        {
            Unknown,
            Acl,
            Bandwidth,
            Offload,
            Profile,
            Security,
            Vlan
        }

        /// <summary>
        /// Advanced switch feature type.
        /// </summary>
        internal enum SwitchFeatureType
        {
            Unknown,
            Bandwidth
        }

        /// <summary>
        /// Gets the Msvm_VirtualEthernetSwitchManagementService object.
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The switch management service object.</returns>
        internal static ManagementObject
        GetEthernetSwitchManagementService(
            ManagementScope scope)
        {
            using (ManagementClass switchServiceClass = 
                   new ManagementClass("Msvm_VirtualEthernetSwitchManagementService"))
            {
                switchServiceClass.Scope = scope;
                ManagementObject switchService = WmiUtilities.GetFirstObjectFromCollection(
                    switchServiceClass.GetInstances());
    
                return switchService;
            }
        }

        /// <summary>
        /// Finds the ethernet switch with the specified name.
        /// </summary>
        /// <param name="switchName">The name of the switch to find.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The ethernet switch object.</returns>
        internal static ManagementObject
        FindEthernetSwitch(
            string switchName,
            ManagementScope scope)
        {
            ObjectQuery query = new ObjectQuery(string.Format(CultureInfo.InvariantCulture,
               "select * from Msvm_VirtualEthernetSwitch where ElementName = \"{0}\"",
               switchName));

            using (ManagementObjectSearcher queryExecute = new ManagementObjectSearcher(scope, query))
            using (ManagementObjectCollection queryResults = queryExecute.Get())
            {    
                if (queryResults.Count == 0)
                {
                    throw new ManagementException("There is no switch with name: " + switchName);
                }
    
                return WmiUtilities.GetFirstObjectFromCollection(queryResults);
            }
        }

        /// <summary>
        /// Finds the external adapter with the specified name.
        /// </summary>
        /// <param name="externalAdapterName">The name of the external adapter.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The external adapter.</returns>
        internal static ManagementObject
        FindExternalAdapter(
            string externalAdapterName, 
            ManagementScope scope)
        {
            ObjectQuery externalAdapterQuery = new ObjectQuery(string.Format(CultureInfo.InvariantCulture,
                "select * from Msvm_ExternalEthernetPort where Name=\"{0}\"", externalAdapterName));

            using (ManagementObjectSearcher externalAdapterExecute = new ManagementObjectSearcher(scope, externalAdapterQuery))
            using (ManagementObjectCollection externalAdapterCollection = externalAdapterExecute.Get())
            {
                if (externalAdapterCollection.Count == 0)
                {
                    throw new ManagementException("There is no external adapter with the name: " + externalAdapterName);
                }
    
                return WmiUtilities.GetFirstObjectFromCollection(externalAdapterCollection);
            }
        }

        /// <summary>
        /// Gets the default connection settings object to use when adding new connection settings. 
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The default connection object.</returns>
        internal static ManagementObject
        GetDefaultEthernetPortAllocationSettingData(
            ManagementScope scope)
        {
            //
            // First get the resource pool for ethernet connection objects.
            //
            ObjectQuery query = new ObjectQuery("select * from Msvm_ResourcePool where ResourceType = 33 and Primordial = True");

            using (ManagementObjectSearcher queryExecute = new ManagementObjectSearcher(scope, query))
            using (ManagementObject resourcePool = WmiUtilities.GetFirstObjectFromCollection(queryExecute.Get()))
            {
                return GetDefaultObjectFromResourcePool(resourcePool, scope);
            }
        }

        /// <summary>
        /// Gets the default synthetic adapter settings object to use when adding a new adapter 
        /// to a virtual machine.
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The default synthetic adapter.</returns>
        internal static ManagementObject
        GetDefaultSyntheticAdapter(
            ManagementScope scope)
        {
            ObjectQuery query = new ObjectQuery("select * from Msvm_ResourcePool where " +
                "ResourceSubType = 'Microsoft:Hyper-V:Synthetic Ethernet Port' and Primordial = True");

            using (ManagementObjectSearcher queryExecute = new ManagementObjectSearcher(scope, query))
            using (ManagementObject resourcePool = WmiUtilities.GetFirstObjectFromCollection(queryExecute.Get()))
            {
                return GetDefaultObjectFromResourcePool(resourcePool, scope);
            }
        }

        /// <summary>
        /// Gets the unique ID corresponding to the given port feature type.
        /// </summary>
        /// <param name="featureType">An enumeration value representing the desired port feature type.</param>
        /// <returns>The unique ID of the feature.</returns>
        internal static string
        GetPortFeatureId(
            PortFeatureType featureType)
        {
            string featureId;

            switch (featureType)
            {
                case PortFeatureType.Acl:
                    featureId = "998BEF4A-5D55-492A-9C43-8B2F5EAE9F2B";
                    break;
                
                case PortFeatureType.Bandwidth:
                    featureId = "24AD3CE1-69BD-4978-B2AC-DAAD389D699C";
                    break;

                case PortFeatureType.Offload:
                    featureId = "C885BFD1-ABB7-418F-8163-9F379C9F7166";
                    break;
            
                case PortFeatureType.Profile:
                    featureId = "9940CD46-8B06-43BB-B9D5-93D50381FD56";
                    break;

                case PortFeatureType.Security:
                    featureId = "776E0BA7-94A1-41C8-8F28-951F524251B5";
                    break;

                case PortFeatureType.Vlan:
                    featureId = "952C5004-4465-451C-8CB8-FA9AB382B773";
                    break;
          
                default:
                    throw new ManagementException("The given port feature type is unrecognized.");
            }

            return featureId;
        }

        /// <summary>
        /// Gets the unique ID corresponding to the given switch feature type.
        /// </summary>
        /// <param name="featureType">An enumeration value representing the desired switch feature type.</param>
        /// <returns>The unique ID of the feature.</returns>
        internal static string
        GetSwitchFeatureId(
            SwitchFeatureType featureType)
        {
            string featureId;

            switch (featureType)
            {
                case SwitchFeatureType.Bandwidth:
                    featureId = "3EB2B8E8-4ABF-4DBF-9071-16DD47481FBE";
                    break;

                default:
                    throw new ManagementException("The given switch feature type is unrecognized.");
            }

            return featureId;
        }

        /// <summary>
        /// Finds the default feature setting object for the specified feature.
        /// </summary>
        /// <param name="featureId">The unique identifier of the feature.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The default settings for the feature.</returns>
        internal static ManagementObject
        GetDefaultFeatureSetting(
            string featureId,
            ManagementScope scope)
        {
            //
            // Iterate through all features to find the requested feature, then follow the association
            // to retrieve the default feature setting data for that feature.
            //
            string defaultFeatureSettingPath = null;

            //
            // Find the features enabled on the switch.
            //
            using (ManagementClass featureCapabilitiesClass = new ManagementClass("Msvm_EthernetSwitchFeatureCapabilities"))
            {
                featureCapabilitiesClass.Scope = scope;

                using (ManagementObjectCollection featureCapabilitiesCollection =
                    featureCapabilitiesClass.GetInstances())
                foreach (ManagementObject featureCapabilities in featureCapabilitiesCollection)
                using (featureCapabilities)
                {    
                    //
                    // Find the feature capabilities object for the specified feature by examining the
                    // feature's unique identifier.
                    //
                    if (string.Equals((string)featureCapabilities["FeatureId"], featureId,
                        StringComparison.OrdinalIgnoreCase))
                    {
                        //
                        // Follow the associations from the feature capabilities object to obtain a copy of the
                        // default feature setting data object.
                        //
                        using (ManagementObjectCollection featureSettingAssociationCollection =
                                featureCapabilities.GetRelationships("Msvm_FeatureSettingsDefineCapabilities"))
                        foreach (ManagementObject featureSettingAssociation in
                                    featureSettingAssociationCollection)
                        using (featureSettingAssociation)
                        {
                            if ((ushort)featureSettingAssociation["ValueRole"] == 0)
                            {
                                //
                                // We found the default feature setting association, and its PartComponent
                                // property is the path to the default feature setting.
                                //
                                defaultFeatureSettingPath = (string)featureSettingAssociation["PartComponent"];
                                break;
                            }
                        }

                        break;
                    }
                }
            }

            if (defaultFeatureSettingPath == null)
            {
                throw new ManagementException("Unable to find the default feature settings!");
            }

            ManagementObject defaultFeatureSetting = new ManagementObject(defaultFeatureSettingPath);

            defaultFeatureSetting.Scope = scope;
            defaultFeatureSetting.Get();
            return defaultFeatureSetting;
        }

        /// <summary>
        /// Adds a new synthetic Network Adapter device to the virtual machine. Note that the virtual
        /// machine must be in the power off state. Also note that the maximum number of Network
        /// Adapter devices that may be configured on a virtual machine is 8.
        /// </summary>
        /// <param name="virtualMachine">The name of the virtual machine.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The newly added synthetic adapter.</returns>
        internal static ManagementObject
        AddSyntheticAdapter(
            ManagementObject virtualMachine,
            ManagementScope scope)
        {
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementObject virtualMachineSettings = WmiUtilities.GetVirtualMachineSettings(virtualMachine))
            //
            // Get the default Synthetic Adapter object, then modify its properties.
            //
            using (ManagementObject adapterToAdd = GetDefaultSyntheticAdapter(scope))
            {
                adapterToAdd["VirtualSystemIdentifiers"] = new string[] { Guid.NewGuid().ToString("B") };
                adapterToAdd["ElementName"] = "Network Adapter";
                adapterToAdd["StaticMacAddress"] = false;

                //
                // Now add it to the virtual machine.
                //
                using (ManagementBaseObject addAdapterInParams = managementService.GetMethodParameters("AddResourceSettings"))
                {
                    addAdapterInParams["AffectedConfiguration"] = virtualMachineSettings.Path.Path;
                    addAdapterInParams["ResourceSettings"] = new string[] { adapterToAdd.GetText(TextFormat.WmiDtd20) };

                    using (ManagementBaseObject addAdapterOutParams =
                           managementService.InvokeMethod("AddResourceSettings", addAdapterInParams, null))
                    {
                        WmiUtilities.ValidateOutput(addAdapterOutParams, scope);

                        //
                        // Get the created network adapter from the output parameters.
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
                            using (ManagementObject job = new ManagementObject((string)addAdapterOutParams["Job"]))
                            {
                                addedAdapter = WmiUtilities.GetFirstObjectFromCollection(job.GetRelated(null,
                                    "Msvm_AffectedJobElement", null, null, null, null, false, null));
                            }
                        }

                        return addedAdapter;
                    }
                }
            }
        }

        /// <summary>
        /// Finds all connections of the virtual machine.
        /// </summary>
        /// <param name="virtualMachine">The virtual machine.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The virtual machine's connection objects.</returns>
        internal static ManagementObjectCollection
        FindConnections(
            ManagementObject virtualMachine,
            ManagementScope scope)
        {
            using (ManagementObject virtualMachineSettings = WmiUtilities.GetVirtualMachineSettings(virtualMachine))
            {
                return virtualMachineSettings.GetRelated("Msvm_EthernetPortAllocationSettingData",
                    "Msvm_VirtualSystemSettingDataComponent",
                    null, null, null, null, false, null);
            }
        }

        /// <summary>
        /// Finds any connections of the virtual machine that are connected to the specified switch. 
        /// This includes connections that are configured with a hard-affinity to the specified 
        /// switch as well as connections that are configured to dynamically connect to a switch
        /// and are currently connected to the specified switch.
        /// </summary>
        /// <param name="virtualMachine">The virtual machine.</param>
        /// <param name="ethernetSwitch">The switch to find connections for.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The list of the virtual machine's connection objects that are either configured 
        /// to connect to the switch, or are currently connected to the switch.</returns>
        internal static IList<ManagementObject>
        FindConnectionsToSwitch(
            ManagementObject virtualMachine,
            ManagementObject ethernetSwitch,
            ManagementScope scope)
        {
            List<ManagementObject> connectionsToSwitch = new List<ManagementObject>();

            //
            // Get all connection objects for the virtual machine, then filter those that are connected
            // to the switch.
            // 
            using (ManagementObjectCollection connectionCollection =
                FindConnections(virtualMachine, scope))
            foreach (ManagementObject connection in connectionCollection)
            {
                //
                // Determine whether this connection has a hard affinity to the switch.
                //

                string[] hostResource = (string[])connection["HostResource"];
                if (hostResource != null && hostResource.Length > 0 &&
                    string.Equals(hostResource[0], ethernetSwitch.Path.Path, StringComparison.OrdinalIgnoreCase))
                {
                    //
                    // This connection is set to always connect to this switch.
                    //

                    connectionsToSwitch.Add(connection);
                    continue;
                }

                //
                // If the connection does not have a hard affinity to this switch, it could 
                // still be currently connected to this switch if it is configured to dynamically
                // connect to a switch and the virtual machine has been started.
                //

                using (ManagementObjectCollection connectedPortCollection = 
                       connection.GetRelated("Msvm_EthernetSwitchPort",
                           "Msvm_ElementSettingData",
                           null, null, null, null, false, null))
                {
                    if (connectedPortCollection.Count > 0)
                    {
                        using (ManagementObject connectedPort = 
                               WmiUtilities.GetFirstObjectFromCollection(connectedPortCollection))
                        {
                            if (string.Equals((string)connectedPort["SystemName"], (string)ethernetSwitch["Name"],
                                StringComparison.OrdinalIgnoreCase))
                            {
                                //
                                // This connection was configured to dynamically connect to a switch when 
                                // the virtual machine was started and is currently connected to the switch
                                // we want to disconnect from.
                                //

                                connectionsToSwitch.Add(connection);
                            }
                            else
                            {
                                connection.Dispose();
                            }
                        }
                    }
                }
            }

            if (connectionsToSwitch.Count == 0)
            {
                throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                    "The virtual machine '{0}' is not connected to the switch '{1}'.",
                    virtualMachine["ElementName"], ethernetSwitch["ElementName"]));
            }

            return connectionsToSwitch;
        }

        /// <summary>
        /// Returns a feature capability object that represents the specified switch feature.
        /// </summary>
        /// <param name="featureName">The name of the switch feature.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The feature capability object that represents the specified feature.</returns>
        internal static ManagementObject
        FindFeatureByName(
            string featureName,
            ManagementScope scope)
        {
            ObjectQuery query = new ObjectQuery(string.Format(
               CultureInfo.InvariantCulture,
               "select * from Msvm_EthernetSwitchFeatureCapabilities where ElementName = \"{0}\"",
               featureName));

            using (ManagementObjectSearcher queryExecute = new ManagementObjectSearcher(scope, query))
            using (ManagementObjectCollection queryResults = queryExecute.Get())
            {
                if (queryResults.Count == 0)
                {
                    throw new ManagementException(string.Format(
                        CultureInfo.CurrentCulture,
                        "Could not find feature '{0}'",
                        featureName));
                }

                return WmiUtilities.GetFirstObjectFromCollection(queryResults);
            }
        }

        /// <summary>
        /// Follows the associations from the resource pool to get the default object for the 
        /// resource type exposed by this resource pool.
        /// </summary>
        /// <param name="resourcePool">The resource pool.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>The default settings for the resource type managed by this resource pool.</returns>
        public static ManagementObject
        GetDefaultObjectFromResourcePool(
            ManagementObject resourcePool,
            ManagementScope scope)
        {
            //
            // The default object is associated with the Msvm_AllocationCapabilities object that 
            // is associated to the resource pool.
            //
            ManagementObject defaultSettingAssociation = null;

            using (ManagementObjectCollection capabilitiesCollection = resourcePool.GetRelated("Msvm_AllocationCapabilities",
                "Msvm_ElementCapabilities",
                null, null, null, null, false, null))
            using (ManagementObject capabilities = WmiUtilities.GetFirstObjectFromCollection(capabilitiesCollection))
            {
                foreach (ManagementObject settingAssociation in capabilities.GetRelationships("Msvm_SettingsDefineCapabilities"))
                {
                    if ((ushort)settingAssociation["ValueRole"] == 0)
                    {
                        defaultSettingAssociation = settingAssociation;
                        break;
                    }
                    else
                    {
                        settingAssociation.Dispose();
                    }
                }
            }

            if (defaultSettingAssociation == null)
            {
                throw new ManagementException("Unable to find the default settings!");
            }

            string defaultSettingPath = (string)defaultSettingAssociation["PartComponent"];
            defaultSettingAssociation.Dispose();

            ManagementObject defaultSetting = new ManagementObject(defaultSettingPath);
            defaultSetting.Scope = scope;
            defaultSetting.Get();

            return defaultSetting;
        }
    }
}
