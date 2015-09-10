// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.ResourcePools
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    static class MsvmResourceAllocationSettingData
    {
        /// <summary>
        /// Returns a MOB for the Msvm_ResourceAllocationSettingData
        /// object associated with the specified resource pool.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        /// <returns>The MOB for a Msvm_ResourceAllocationSettingData object.</returns>
        internal static ManagementObject
        GetAllocationSettingsForPool(
            ManagementScope scope,
            string resourceType,
            string resourceSubType,
            string poolId)
        {
            using (ManagementObject pool = WmiUtilities.GetResourcePool(
                resourceType,
                resourceSubType,
                poolId,
                scope))
            {
                if ((bool)pool.GetPropertyValue("Primordial"))
                    return null;

                using (ManagementObjectCollection rasdCollection =
                    pool.GetRelated(
                    "CIM_ResourceAllocationSettingData",
                    "Msvm_SettingsDefineState",
                    null,
                    null,
                    "SettingData",
                    "ManagedElement",
                    false,
                    null))
                {
                    foreach (ManagementObject rasd in rasdCollection)
                    {
                        return rasd;
                    }

                    return null;
                }
            }
        }

        /// <summary>
        /// Returns an embedded instance string of a Msvm_ResourceAllocationSettingData
        /// object initialized with the specified pool ID, pool name, and resource
        /// information.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        /// <param name="hostResource">An array of strings that indicates the
        /// host resources to assign to the pool.</param>
        /// <returns>The embedded instance string of a
        /// Msvm_ResourceAllocationSettingData object.</returns>
        internal static string
        GetNewPoolAllocationSettings(
            ManagementScope scope,
            string resourceType,
            string resourceSubType,
            string poolId,
            string[] hostResources)
        {
            using (ManagementClass rasdClass = new ManagementClass("Msvm_ResourceAllocationSettingData"))
            {
                rasdClass.Scope = scope;

                using (ManagementObject rasdMob = rasdClass.CreateInstance())
                {
                    rasdMob["ResourceType"] = resourceType;

                    if (resourceType == "1")
                    {
                        rasdMob["OtherResourceType"] = resourceSubType;
                        rasdMob["ResourceSubType"] = string.Empty;
                    }
                    else
                    {
                        rasdMob["OtherResourceType"] = string.Empty;
                        rasdMob["ResourceSubType"] = resourceSubType;
                    }

                    rasdMob["PoolId"] = poolId;
                    rasdMob["HostResource"] = hostResources;

                    string rasdString = rasdMob.GetText(TextFormat.WmiDtd20);

                    return rasdString;
                }
            }
        }

        /// <summary>
        /// Returns a string array of embedded instance strings of Msvm_ResourceAllocationSettingData
        /// objects for each specified pool ID.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="rPConfigurationService">The Resource Pool
        /// Configuration Service MOB.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolIdArray">A string array of pool IDs.</param>
        /// <param name="hostResourcesArray">An array of string arrays that specify host
        /// resources.</param>
        /// <returns>An array of embedded instance strings of
        /// Msvm_ResourceAllocationSettingData objects.</returns>
        internal static string[]
        GetNewPoolAllocationSettingsArray(
            ManagementScope scope,
            string resourceType,
            string resourceSubType,
            string[] poolIdArray,
            string[][] hostResourcesArray)
        {
            List<string> rasdList = new List<string>();

            for (uint index = 0; index < poolIdArray.Length; ++index)
            {
                rasdList.Add(GetNewPoolAllocationSettings(
                    scope,
                    resourceType,
                    resourceSubType,
                    poolIdArray[index],
                    hostResourcesArray[index]));
            }

            return rasdList.ToArray();
        }

        /// <summary>
        /// Returns the default Msvm_ResourceAllocationSettingData MOB.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="pool">Mvm_ResourcePool MOB.</param name>
        /// <param name="valueRole"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRole property.</param name>
        /// <param name="valueRange"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRange property.</param name>
        /// <returns>Msvm_ResourceAllocationSettingData MOB.</returns>
        public static ManagementObject GetDefaultAllocationSettings(
            ManagementScope scope,
            ManagementObject pool)
        {
            return GetPrototypeAllocationSettings(
                scope,
                pool,
                "0",
                "0");
        }

        /// <summary>
        /// Returns the minimum Msvm_ResourceAllocationSettingData MOB.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="pool">Mvm_ResourcePool MOB.</param name>
        /// <param name="valueRole"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRole property.</param name>
        /// <param name="valueRange"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRange property.</param name>
        /// <returns>Msvm_ResourceAllocationSettingData MOB.</returns>
        public static ManagementObject GetMinimumAllocationSettings(
            ManagementScope scope,
            ManagementObject pool)
        {
            return GetPrototypeAllocationSettings(
                scope,
                pool,
                "3",
                "1");
        }

        /// <summary>
        /// Returns the maximum Msvm_ResourceAllocationSettingData MOB.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="pool">Mvm_ResourcePool MOB.</param name>
        /// <param name="valueRole"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRole property.</param name>
        /// <param name="valueRange"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRange property.</param name>
        /// <returns>Msvm_ResourceAllocationSettingData MOB.</returns>
        public static ManagementObject GetMaximumAllocationSettings(
            ManagementScope scope,
            ManagementObject pool)
        {
            return GetPrototypeAllocationSettings(
                scope,
                pool,
                "3",
                "2");
        }

        /// <summary>
        /// Returns the incremental Msvm_ResourceAllocationSettingData MOB.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="pool">Mvm_ResourcePool MOB.</param name>
        /// <param name="valueRole"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRole property.</param name>
        /// <param name="valueRange"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRange property.</param name>
        /// <returns>Msvm_ResourceAllocationSettingData MOB.</returns>
        public static ManagementObject GetIncrementalAllocationSettings(
            ManagementScope scope,
            ManagementObject pool)
        {
            return GetPrototypeAllocationSettings(
                scope,
                pool,
                "3",
                "3");
        }

        /// <summary>
        /// Returns the specified Msvm_ResourceAllocationSettingData MOB.
        /// </summary>
        /// <remarks>Role/Range: 0/0 - Default; 3/1 Minimum; 3/2 Maximum; 3/3 Incremental</remarks>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="pool">Mvm_ResourcePool MOB.</param name>
        /// <param name="valueRole"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRole property.</param name>
        /// <param name="valueRange"MvmCim_SettingsDefineCapabilities_ResourcePool ValueRange property.</param name>
        /// <returns>Msvm_ResourceAllocationSettingData MOB.</returns>
        internal static ManagementObject GetPrototypeAllocationSettings(
            ManagementScope scope,
            ManagementObject pool,
            string valueRole,
            string valueRange)
        {
            foreach (ManagementObject allocationCapability in pool.GetRelated("Msvm_AllocationCapabilities",
                "Msvm_ElementCapabilities",
                null, null, null, null, false, null))
            using (allocationCapability)
            {
                foreach (ManagementObject relationship in allocationCapability.GetRelationships("Cim_SettingsDefineCapabilities"))
                using (relationship)
                {
                    if (relationship["ValueRole"].ToString() == valueRole &&
                        relationship["ValueRange"].ToString() == valueRange)
                    {
                        ManagementObject rasd = new ManagementObject(
                            pool.Scope,
                            new ManagementPath(relationship["PartComponent"].ToString()),
                            null);

                        return rasd;
                    }
                }
            }

            return null;
        }

        /// <summary>
        /// Displays the ResourceAllocationSettingData
        /// object associated with the specified resource pool.
        /// </summary>
        /// <param name="rasd">A Msvm_ResourceAllocationSettingData MOB.</param>
        internal static void
        DisplayPoolResourceAllocationSettingData(
            ManagementObject rasd)
        {
            Console.WriteLine("Msvm_ResourceAllocationSettingData:");

            if (rasd == null)
            {
                Console.WriteLine("\tA primordial pool does not have an associated " +
                    "Msvm_ResourceAllocationSettingData object.");

                return;
            }

            Console.WriteLine("\tElementName: {0}", rasd.GetPropertyValue("ElementName"));
            Console.WriteLine("\tCaption: {0}", rasd.GetPropertyValue("Caption"));
            Console.WriteLine("\tInstanceID: {0}", rasd.GetPropertyValue("InstanceID"));
            Console.WriteLine("\tResourceType: {0}", rasd.GetPropertyValue("ResourceType"));
            Console.WriteLine("\tResourceSubType: {0}", rasd.GetPropertyValue("ResourceSubType"));

            string[] hostResources = (string[])rasd.GetPropertyValue("HostResource");

            if (hostResources != null)
            {
                Console.WriteLine("\tHostResources:");

                foreach (string resource in hostResources)
                {
                    Console.WriteLine("\t\t" + resource);
                }
            }
        }

        /// <summary>
        /// Displays the ResourceAllocationSettingData
        /// object associated with the specified resource pool.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="pool">Msvm_ResourcePool MOB.</param>
        internal static void
        DisplayPoolResourceAllocationSettingData(
            ManagementScope scope,
            ManagementObject pool)
        {
            using (ManagementObject rasd =
                MsvmResourceAllocationSettingData.GetAllocationSettingsForPool(
                    scope,
                    pool.GetPropertyValue("ResourceType").ToString(),
                    pool.GetPropertyValue("ResourceSubType").ToString(),
                    pool.GetPropertyValue("PoolId").ToString()))
            {
                DisplayPoolResourceAllocationSettingData(rasd);
            }
        }

        /// <summary>
        /// Displays the Msvm_ResourceAllocationSettingData
        /// object associated with the specified resource pool.
        /// </summary>
        /// <param name="resourceDisplayName">The display name for the resourcetype.</param>
        /// <param name="poolId">The pool ID.</param>
        internal static void
        DisplayPoolResourceAllocationSettingData(
            string resourceDisplayName,
            string poolId)
        {
            Console.WriteLine(
                "Displaying the Msvm_ResourceAllocationSettingData properties for the following " +
                "resource pool:\n" +
                "\tPool Id: " + poolId);

            ResourceUtilities.DisplayResourceInformation(resourceDisplayName);

            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject rasd =
                MsvmResourceAllocationSettingData.GetAllocationSettingsForPool(
                    scope,
                    ResourceUtilities.GetResourceType(resourceDisplayName),
                    ResourceUtilities.GetResourceSubType(resourceDisplayName),
                    poolId))
            {
                DisplayPoolResourceAllocationSettingData(rasd);
            }
        }

        /// <summary>
        /// Displays the valid default, minimum, maximum, and incremental
        /// Msvm_ResourceAllocationSettingData objects for the specified resource.
        /// </summary>
        /// <param name="resourceDisplayName">The display name for the resource type.</param>
        /// <param name="poolId">The pool ID.</param>
        internal static void
        DisplayValidResourceAllocationSettingDataSettings(
            string resourceDisplayName,
            string poolId)
        {
            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject pool = WmiUtilities.GetResourcePool(
                ResourceUtilities.GetResourceType(resourceDisplayName),
                ResourceUtilities.GetResourceSubType(resourceDisplayName),
                poolId,
                scope))
            using (ManagementObject defaultRasd = GetDefaultAllocationSettings(
                scope,
                pool))
            using (ManagementObject minimumRasd = GetMinimumAllocationSettings(
                scope,
                pool))
            using (ManagementObject maximumRasd = GetMaximumAllocationSettings(
                scope,
                pool))
            using (ManagementObject incrementalRasd = GetIncrementalAllocationSettings(
                scope,
                pool))
            {
                Console.WriteLine("(default)");
                DisplayPoolResourceAllocationSettingData(defaultRasd);

                Console.WriteLine("(minimum)");
                DisplayPoolResourceAllocationSettingData(minimumRasd);

                Console.WriteLine("(maximum)");
                DisplayPoolResourceAllocationSettingData(maximumRasd);

                Console.WriteLine("(incremental)");
                DisplayPoolResourceAllocationSettingData(incrementalRasd);
            }
        }
    }
}
