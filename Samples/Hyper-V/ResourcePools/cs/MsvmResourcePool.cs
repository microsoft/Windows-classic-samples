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

    static class MsvmResourcePool
    {
        /// <summary>
        /// Returns the reference path for the specified pool ID.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID.</param>
        /// <returns>The reference path for the Msvm_ResourcePool object.</returns>
        internal static string
        GetResourcePoolPath(
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
                return pool.Path.Path;
            }
        }

        
        /// <summary>
        /// Displays information for the specified pool.
        /// </summary>
        /// <param name="pool">Msvm_ResourcePool MOB.</param>
        internal static void
        DisplayResourcePool(
            ManagementObject pool)
        {
            Console.WriteLine("Msvm_ResourcePool:");

            Console.WriteLine("\tPoolID: {0}", pool.GetPropertyValue("PoolID"));
            Console.WriteLine("\tInstanceID: {0}", pool.GetPropertyValue("InstanceID"));
            Console.WriteLine("\tResourceType: {0}", pool.GetPropertyValue("ResourceType"));
            Console.WriteLine("\tResourceSubtype: {0}", pool.GetPropertyValue("ResourceSubType"));
        }

        /// <summary>
        /// Displays the Msvm_ResourcePool, Msvm_ResourcePoolSettingData and
        /// Msvm_ResourceAllocationSettingsData properties for the specified
        /// resource pool.
        /// </summary>
        /// <param name="resourceDisplayName">The display name for the resource type.</param>
        /// <param name="poolId">The pool ID.</param>
        internal static void
        DisplayPoolVerbose(
            string resourceDisplayName,
            string poolId)
        {
            Console.WriteLine(
                "Displaying the Msvm_ResourcePool, Msvm_ResourcePoolSettingData and " +
                "the Msvm_ResourceAllocationSettingsData properties for the following " +
                "resource pool:\n");

            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject pool = WmiUtilities.GetResourcePool(
                ResourceUtilities.GetResourceType(resourceDisplayName),
                ResourceUtilities.GetResourceSubType(resourceDisplayName),
                poolId,
                scope))
            {
                DisplayResourcePool(pool);

                MsvmResourceAllocationSettingData.DisplayPoolResourceAllocationSettingData(
                    scope,
                    pool);

                MsvmResourcePoolSettingData.DisplayPoolResourcePoolSettingData(
                    scope,
                    pool);
            }
        }

        /// <summary>
        /// Displays the pool IDs for the immediate child pools for the
        /// specified pool.
        /// </summary>
        /// <param name="resourceDisplayName">The display name for the resource type.</param>
        /// <param name="poolId">The pool ID.</param>
        internal static void
        DisplayChildPools(
            string resourceDisplayName,
            string poolId)
        {
            Console.WriteLine(
                "Displaying the Msvm_ResourcePool objects for the child pools of the " +
                "following resource pool:\n" +
                "\tPool Id: " + poolId);

            ResourceUtilities.DisplayResourceInformation(
                resourceDisplayName);

            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject pool = WmiUtilities.GetResourcePool(
                ResourceUtilities.GetResourceType(resourceDisplayName),
                ResourceUtilities.GetResourceSubType(resourceDisplayName),
                poolId,
                scope))
            using (ManagementObjectCollection childPoolCollection =
                pool.GetRelated(
                "Msvm_ResourcePool",
                "Msvm_ElementAllocatedFromPool",
                null,
                null,
                "Dependent",
                "Antecedent",
                false,
                null))
            {
                foreach (ManagementObject childPool in childPoolCollection)
                    using (childPool)
                {
                    DisplayResourcePool(childPool);
                }
            }
        }

        /// <summary>
        /// Displays the pool IDs for the parent pools for the
        /// specified pool.
        /// </summary>
        /// <param name="resourceDisplayName">The display name for the resource type.</param>
        /// <param name="poolId">The pool ID.</param>
        internal static void
        DisplayParentPools(
            string resourceDisplayName,
            string poolId)
        {
            Console.WriteLine(
                "Displaying the Msvm_ResourcePool objects for the parent pools of the " +
                "following resource pool:\n" +
                "\tPool Id: " + poolId);

            ResourceUtilities.DisplayResourceInformation(
                resourceDisplayName);

            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject pool = WmiUtilities.GetResourcePool(
                ResourceUtilities.GetResourceType(resourceDisplayName),
                ResourceUtilities.GetResourceSubType(resourceDisplayName),
                poolId,
                scope))
            using (ManagementObjectCollection childPoolCollection =
                pool.GetRelated(
                "Msvm_ResourcePool",
                "Msvm_ElementAllocatedFromPool",
                null,
                null,
                "Antecedent",
                "Dependent",
                false,
                null))
            {
                foreach (ManagementObject childPool in childPoolCollection)
                    using (childPool)
                {
                    DisplayResourcePool(childPool);
                }
            }
        }
    }
}
