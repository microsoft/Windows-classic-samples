// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.ResourcePools
{
    using System;
    using System.Collections;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;
 
    static class MsvmResourcePoolSettingData
    {
        /// <summary>
        /// Returns a MOB for the Msvm_ResourcePoolSettingData
        /// object associated with the specified resource pool.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        /// <returns>The MOB for a Msvm_ResourcePoolSettingData object.</returns>
        internal static ManagementObject
        GetPoolResourcePoolSettingData(
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
            using (ManagementObjectCollection rpsdCollection =
                pool.GetRelated(
                "Msvm_ResourcePoolSettingData",
                "Msvm_SettingsDefineState",
                null,
                null,
                "SettingData",
                "ManagedElement",
                false,
                null))
            {
                //
                // There will always only be one RPSD for a given resource pool.
                //
                if (rpsdCollection.Count != 1)
                {
                    if (resourceType == "1")
                    {
                        throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                            "A single Msvm_ResourcePoolSettingData derived instance could not be found for " +
                            "ResourceType \"{0}\", OtherResourceType \"{1}\" and PoolId \"{2}\"",
                            resourceType, resourceSubType, poolId));
                    }
                    else
                    {
                        throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                            "A single Msvm_ResourcePoolSettingData derived instance could not be found for " +
                            "ResourceType \"{0}\", ResourceSubtype \"{1}\" and PoolId \"{2}\"",
                            resourceType, resourceSubType, poolId));
                    }
                }

                foreach (ManagementObject rpsd in rpsdCollection)
                {
                    return rpsd;
                }

                return null;
            }
        }

        /// <summary>
        /// Returns an embedded instance string of an Msvm_ResourcePoolSettingData
        /// object, initialized with the specified pool ID, pool name, and resource
        /// information.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        /// <param name="poolName">The pool name to assign.</param>
        /// <returns>The embedded instance string of an
        /// Msvm_ResourcePoolSettingData object.</returns>
        internal static string
        GetSettingsForPool(
            ManagementScope scope,
            string resourceType,
            string resourceSubType,
            string poolId,
            string poolName)
        {
            using (ManagementClass rpsdClass = new ManagementClass("Msvm_ResourcePoolSettingData"))
            {
                rpsdClass.Scope = scope;

                using (ManagementObject rpsdMob = rpsdClass.CreateInstance())
                {
                    rpsdMob["ResourceType"] = resourceType;

                    if (resourceType == "1")
                    {
                        rpsdMob["OtherResourceType"] = resourceSubType;
                        rpsdMob["ResourceSubType"] = string.Empty;
                    }
                    else
                    {
                        rpsdMob["OtherResourceType"] = string.Empty;
                        rpsdMob["ResourceSubType"] = resourceSubType;
                    }

                    rpsdMob["PoolId"] = poolId;
                    rpsdMob["ElementName"] = poolName;

                    string rpsdString = rpsdMob.GetText(TextFormat.WmiDtd20);

                    return rpsdString;
                }
            }
        }

        /// <summary>
        /// Displays the Msvm_ResourcePoolSettingData
        /// object associated with the specified resource pool.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="pool">Msvm_ResourcePool MOB.</param>
        internal static void
        DisplayPoolResourcePoolSettingData(
            ManagementScope scope,
            ManagementObject pool)
        {
            using (ManagementObject rpsd =
                MsvmResourcePoolSettingData.GetPoolResourcePoolSettingData(
                    scope,
                    pool.GetPropertyValue("ResourceType").ToString(),
                    pool.GetPropertyValue("ResourceSubType").ToString(),
                    pool.GetPropertyValue("PoolId").ToString()))
            {
                Console.WriteLine("Msvm_ResourcePoolSettingData:");
                Console.WriteLine("\tPoolId: {0}", rpsd.GetPropertyValue("PoolId"));
                Console.WriteLine("\tElementName: {0}", rpsd.GetPropertyValue("ElementName"));
                Console.WriteLine("\tInstanceID: {0}", rpsd.GetPropertyValue("InstanceID"));
                Console.WriteLine("\tResourceType: {0}", rpsd.GetPropertyValue("ResourceType"));
                Console.WriteLine("\tResourceSubType: {0}", rpsd.GetPropertyValue("ResourceSubType"));
            }
        }

        /// <summary>
        /// Displays the Msvm_ResourcePoolSettingData
        /// object associated with the specified resource pool.
        /// </summary>
        /// <param name="resourceDisplayName">The display name for the resourcetype.</param>
        /// <param name="poolId">The pool ID.</param>
        internal static void
        DisplayPoolResourcePoolSettingData(
            string resourceDisplayName,
            string poolId)
        {
            Console.WriteLine(
                "Displaying the Msvm_ResourcePoolSettingData properties for the following " +
                "resource pool:\n" +
                "\tPool Id: " + poolId);

            ResourceUtilities.DisplayResourceInformation(resourceDisplayName);

            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject rpsd =
                MsvmResourcePoolSettingData.GetPoolResourcePoolSettingData(
                    scope,
                    ResourceUtilities.GetResourceType(resourceDisplayName),
                    ResourceUtilities.GetResourceSubType(resourceDisplayName),
                    poolId))
            {
                Console.WriteLine("Msvm_ResourcePoolSettingData:");

                Console.WriteLine("\tElementName: " + rpsd.GetPropertyValue("ElementName").ToString());
            }
        }
    }
}
