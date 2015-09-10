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

    /// <summary>
    /// Represents a WWPN WWNN Tuple, to identify an FC Port.
    /// </summary>
    internal struct WorldWideName
    {
        public string PortName;
        public string NodeName;
    };

    static class FibreChannelUtilities
    {
        internal static readonly string FcConnectionResourceType = "64764"; //0xFCFC
        internal static readonly string FcConnectionResourceSubType = "Microsoft:Hyper-V:FibreChannel Connection";
        internal static readonly string FcDeviceName = "Fibre Channel Adapter";

        /// <summary>
        /// Get the ManagementScope to connect to Hyper-V WMI.
        /// </summary>
        /// <returns>ManagementScope to connect to Hyper-V WMI.</returns>
        internal static ManagementScope
        GetFcScope()
        {
            return new ManagementScope(@"root\virtualization\v2");
        }

        /// <summary>
        /// Follows the associations from the resource pool to get the default object for the 
        /// resource type exposed by this resource pool.
        /// </summary>
        /// <param name="resourcePool">The resource pool.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <returns>
        /// The default settings for the resource type managed by this resource pool.
        /// </returns>
        internal static ManagementObject
        GetDefaultObjectFromResourcePool(
            ManagementObject resourcePool,
            ManagementScope scope)
        {
            //
            // The default object is associated with the Msvm_AllocationCapabilities object that 
            // is associated to the resource pool.
            //
            string defaultSettingPath = null;

            using (ManagementObjectCollection capabilitiesCollection = 
                   resourcePool.GetRelated("Msvm_AllocationCapabilities",
                                           "Msvm_ElementCapabilities",
                                           null, null, null, null, false, null))
            using (ManagementObject capabilities =
                   WmiUtilities.GetFirstObjectFromCollection(capabilitiesCollection))
            {
                foreach (ManagementObject settingAssociation in 
                         capabilities.GetRelationships("Msvm_SettingsDefineCapabilities"))
                {
                    using (settingAssociation)
                    {
                        if ((ushort)settingAssociation["ValueRole"] == 0)
                        {
                            defaultSettingPath = (string)settingAssociation["PartComponent"];
                            break;
                        }
                    }
                }
            }

            if (defaultSettingPath == null)
            {
                throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                                                            "Unable to find the default settings!"));
            }

            ManagementObject defaultSetting = new ManagementObject(defaultSettingPath);
            defaultSetting.Scope = scope;
            defaultSetting.Get();

            return defaultSetting;
        }

        /// <summary>
        /// Gets the default connection settings object to use when adding new connection settings. 
        /// </summary>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        /// <param name="sanName">The name of the San / pool ID of the Resource Pool.</param>
        /// <returns>The default connection object.</returns>
        internal static ManagementObject
        GetDefaultFcPortAllocationSettingData(
            ManagementScope scope,
            string sanName)
        {
            //
            // First get the resource pool for FC connection objects.
            //
            ObjectQuery query = new ObjectQuery(string.Format(CultureInfo.InvariantCulture,
                "SELECT * FROM Msvm_ResourcePool WHERE ResourceType = {0} AND PoolID = '{1}'",
                FcConnectionResourceType, sanName));

            using (ManagementObjectSearcher queryExecute =
                   new ManagementObjectSearcher(scope, query))
            using (ManagementObject resourcePool =
                   WmiUtilities.GetFirstObjectFromCollection(queryExecute.Get()))
            {
                return GetDefaultObjectFromResourcePool(resourcePool, scope);
            }
        }

        /// <summary>
        /// Uses the WWPN Generator and the Configures WWNN to generate the 2 sets
        /// of WWNs for a Virtual FC Adapter.
        /// </summary>
        /// <param name="scope">ManagementScope for WMI connection.</param>
        /// <param name="wwnA">On success will have the virtual WWN for the Virtual HBA.</param>
        /// <param name="wwnB">On success will have the secondary WWN for the Virtual HBA.</param>
        internal static void
        GenerateWorldWideNames(
            ManagementScope scope,
            ref WorldWideName wwnA,
            ref WorldWideName wwnB)
        {
            using (ManagementObject settings =
                   WmiUtilities.GetVirtualMachineManagementServiceSettings(scope))
            using (ManagementObject managementService =
                   WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementBaseObject inParams = 
                   managementService.GetMethodParameters("GenerateWwpn"))
            {
                // Set WWNN.
                string assignedWwnn = settings["CurrentWWNNAddress"].ToString();
                wwnA.NodeName = assignedWwnn;
                wwnB.NodeName = assignedWwnn;

                // Generate 2 WWPNs for the 2 sets.
                inParams["NumberOfWwpns"] = 2;
                using (ManagementBaseObject outParams =
                       managementService.InvokeMethod("GenerateWwpn", inParams, null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope);

                    // Requested for 2 WWPNs to be generated.
                    string[] wwpnArray = outParams["GeneratedWwpn"] as string[];
                    wwnA.PortName = wwpnArray[0];
                    wwnB.PortName = wwpnArray[1];
                }
            }
        }
        
        /// <summary>
        /// For a valid ExternalFcPort, get the host resource to be added to a virtual SAN.
        /// </summary>
        /// <param name="wwn">The WWN for the ExternalFcPort.</param>
        /// <returns>
        /// WMI Path to the VirtualFcSwitch instance for the ExternalFcPort,
        /// which is the host resource for the virtual SAN.
        /// </returns>
        internal static string
        GetHostResourceFromWwn(
            WorldWideName wwn
            )
        {
            ManagementScope scope = GetFcScope();

            //
            // If the wwn is for a valid ExternalFcPort, we should be able to get
            // the FcEndpoint for the VirtualFcSwitch for the ExternalFcPort.
            // For every valid ExternalFcPort, there is guaranteed to be a corresponding
            // unique VirtualFcSwitch.
            //
            string queryString = string.Format(CultureInfo.InvariantCulture,
                "SELECT * FROM Msvm_FcEndpoint WHERE SystemCreationClassName = '{0}'" +
                " AND WWNN = '{1}' AND WWPN = '{2}'",
                "Msvm_VirtualFcSwitch", wwn.NodeName, wwn.PortName);

            ObjectQuery query = new ObjectQuery(queryString);
            
            using (ManagementObjectSearcher switchEndpointSearcher =
                   new ManagementObjectSearcher(scope, query))
            using (ManagementObject switchEndpoint =
                   WmiUtilities.GetFirstObjectFromCollection(switchEndpointSearcher.Get()))
            {
                string switchQueryString = string.Format(CultureInfo.InvariantCulture,
                    "SELECT * FROM Msvm_VirtualFcSwitch WHERE Name = '{0}'",
                    switchEndpoint["SystemName"]);

                ObjectQuery switchQuery = new ObjectQuery(switchQueryString);
            
                using (ManagementObjectSearcher switchSearcher =
                       new ManagementObjectSearcher(scope, switchQuery))
                using (ManagementObjectCollection fcSwitches = switchSearcher.Get())
                //
                // There is only 1 VirtualFcSwitch for an ExternalFcPort.
                //
                using (ManagementObject fcSwitch =
                       WmiUtilities.GetFirstObjectFromCollection(fcSwitches))
                {
                    return fcSwitch.Path.Path;
                }
            }
        }

        /// <summary>
        /// Returns the Msvm_ResourcePoolConfigurationService instance.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The resource pool configuration service instance.</returns>
        internal static ManagementObject
        GetResourcePoolConfigurationService(
            ManagementScope scope)
        {
            using (ManagementClass resourcePoolConfigurationServiceClass =
                   new ManagementClass("Msvm_ResourcePoolConfigurationService"))
            {
                resourcePoolConfigurationServiceClass.Scope = scope;

                //
                // Msvm_ResourcePoolConfigurationService is a singleton object.
                //
                ManagementObject resourcePoolConfigurationService =
                    WmiUtilities.GetFirstObjectFromCollection(
                        resourcePoolConfigurationServiceClass.GetInstances());

                return resourcePoolConfigurationService;
            }
        }

        /// <summary>
        /// Returns an embedded instance string of an Msvm_ResourcePoolSettingData
        /// object, initialized with the specified pool ID, pool name, notes, and resource
        /// information.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        /// <param name="notes">The Notes for the Virtual SAN to assign.</param>
        /// <returns>
        /// The embedded instance string of an Msvm_ResourcePoolSettingData object.</returns>
        internal static string
        GetSettingsForPool(
            ManagementScope scope,
            string poolId,
            string notes
            )
        {
            using (ManagementClass rpsdClass = new ManagementClass("Msvm_ResourcePoolSettingData"))
            {
                rpsdClass.Scope = scope;
                using (ManagementObject rpsdInstance = rpsdClass.CreateInstance())
                {
                    rpsdInstance["ResourceType"] = FcConnectionResourceType;
                    rpsdInstance["ResourceSubType"] = FcConnectionResourceSubType;
                    rpsdInstance["OtherResourceType"] = string.Empty;
                    rpsdInstance["PoolId"] = poolId;
                    if (notes != null)
                    {
                        rpsdInstance["Notes"] = notes;
                    }
                    rpsdInstance["ElementName"] = @"Friendly name for virtual SAN - " + poolId;

                    string rpsdString = rpsdInstance.GetText(TextFormat.WmiDtd20);
                    return rpsdString;
                }
            }
        }

        /// <summary>
        /// Returns an embedded instance string of a Msvm_ResourceAllocationSettingData
        /// object initialized with the specified pool ID, and resource information.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        /// <param name="hostResources">An array of strings that indicates the
        /// host resources to assign to the pool.</param>
        /// <returns>
        /// The embedded instance string of a Msvm_ResourceAllocationSettingData object.
        /// </returns>
        internal static string
        GetNewPoolAllocationSettings(
            ManagementScope scope,
            string poolId,
            string[] hostResources
            )
        {
            using (ManagementClass rasdClass = new ManagementClass("Msvm_ResourceAllocationSettingData"))
            {
                rasdClass.Scope = scope;

                using (ManagementObject rasdInstance = rasdClass.CreateInstance())
                {
                    rasdInstance["ResourceType"] = FcConnectionResourceType;
                    rasdInstance["ResourceSubType"] = FcConnectionResourceSubType;
                    rasdInstance["OtherResourceType"] = string.Empty;

                    rasdInstance["PoolId"] = poolId;
                    rasdInstance["HostResource"] = hostResources;

                    string rasdString = rasdInstance.GetText(TextFormat.WmiDtd20);

                    return rasdString;
                }
            }
        }

        /// <summary>
        /// Gets the WMI Path to the FC Connection Resource Pool.
        /// </summary>
        /// <param name="scope">The ManagementScope used to connecto to WMI.</param>
        /// <param name="poolId">PoolId of the ResourcePool (or SAN Name).
        /// Should be null or empty for the primordial pool.</param>
        /// <returns>The WMI Path to the Primordial FC Connection Pool.</returns>
        internal static string
        GetResourcePoolPath(
            ManagementScope scope,
            string poolId
            )
        {
            string poolQuery = 
                "SELECT * FROM Msvm_ResourcePool WHERE ResourceType = " + FcConnectionResourceType;

            if (poolId == null || poolId.Length == 0)
            {
                poolQuery += " AND Primordial = TRUE";
            }
            else
            {
                poolQuery += " AND PoolId = '" + poolId + "'";
            }

            ObjectQuery query = new ObjectQuery(poolQuery);
            
            using (ManagementObjectSearcher searcher = new ManagementObjectSearcher(scope, query))
            using (ManagementObjectCollection pools = searcher.Get())
            {
                return WmiUtilities.GetFirstObjectFromCollection(pools).Path.Path;
            }
        }
    }
}
