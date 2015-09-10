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
    
    /// <summary>
    /// Utilities to allow access to the Msvm_ResourcePoolConfigurationService service,
    /// which allow for the creation, modication, and deletion of resource pools.
    /// For the ModifyPoolResource, ModifyPoolSettings, and DeletePool methods, there are
    /// three ways that a resource pool can be referenced: by WMI reference path,
    /// by a resource pool managment object (MOB), and by resource type and pool ID.
    /// </summary>
    static class MsvmResourcePoolConfigurationService
    {
        /// <summary>
        /// Returns the Msvm_ResourcePoolConfigurationService MOB.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The resource pool configuration service MOB.</returns>
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
        /// Creates a resource pool and returns its associated CIM_ResourcePool MOB.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// service management object.</param>
        /// <param name="rPConfigurationService">The resource pool configuration.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="childPoolId">The pool ID to assign.</param>
        /// <param name="childPoolName">The pool name to assign.</param>
        /// <param name="parentPoolIdArray">An array of strings that specify the parent
        /// pool IDs.</param>
        /// <param name="poolName">An array of string arrays that represent the host
        /// resources.</param>
        /// <returns>A Msvm_ResourcePool object.</returns>
        internal static ManagementObject
        CreatePoolHelper(
            ManagementScope scope,
            ManagementObject rPConfigurationService,
            string resourceType,
            string resourceSubType,
            string childPoolId,
            string childPoolName,
            string[] parentPoolIdArray,
            string[][] parentHostResourcesArray)
        {
            if (parentPoolIdArray.Length == 0)
            {
                throw new ManagementException(string.Format(
                    CultureInfo.CurrentCulture,
                    @"At least one parent pool must be specified when creating a 
                    child resource pool (PoolId ""{0}"")", childPoolId));
            }

            if (parentPoolIdArray.Length != parentHostResourcesArray.Length)
            {
                throw new ManagementException(string.Format(
                    CultureInfo.CurrentCulture,
                    @"When creating a child resource pool, a host resource must be 
                    specified for each parent pool. Shared allocations are not 
                    supported (PoolId ""{0}"")", childPoolId));
            }

            string resourcePoolSettingData =
                MsvmResourcePoolSettingData.GetSettingsForPool(
                    scope,
                    resourceType,
                    resourceSubType,
                    childPoolId,
                    childPoolName);

            string[] parentPoolPathArray =
                ResourcePoolUtilities.GetParentPoolArrayFromPoolIds(
                    scope,
                    resourceType,
                    resourceSubType,
                    parentPoolIdArray);

            string[] resourceAllocationSettingDataArray =
                MsvmResourceAllocationSettingData.GetNewPoolAllocationSettingsArray(
                    scope,
                    resourceType,
                    resourceSubType,
                    parentPoolIdArray,
                    parentHostResourcesArray);

            using (ManagementBaseObject inParams =
                   rPConfigurationService.GetMethodParameters(
                       "CreatePool"))
            {
                inParams["PoolSettings"] = resourcePoolSettingData;
                inParams["ParentPools"] = parentPoolPathArray;
                inParams["AllocationSettings"] = resourceAllocationSettingDataArray;

                using (ManagementBaseObject outParams =
                       rPConfigurationService.InvokeMethod(
                           "CreatePool",
                           inParams,
                           null))
                {
                    if (WmiUtilities.ValidateOutput(outParams, scope, true, true))
                    {
                        string poolPath = outParams["Pool"].ToString();

                        return new ManagementObject(
                            scope,
                            new ManagementPath(poolPath),
                            null);
                    }
                    else
                    {
                        return null;
                    }
                }
            }
        }

        /// <summary>
        /// Creates a resource pool.
        /// </summary>
        /// <param name="resourceDisplayName">The display name of the resource type.</param>
        /// <param name="childPoolId">The pool ID to assign.</param>
        /// <param name="childPoolName">The pool name to assign.</param>
        /// <param name="parentPoolIdArray">A delimited string that specifies the parent
        /// pool IDs.</param>
        /// <param name="parentHostResourcesString">A delimited string that represents the host
        /// resources for each parent pool.</param>
        internal static void
        CreatePool(
            string resourceDisplayName,
            string childPoolId,
            string childPoolName,
            string parentPoolIdsString,
            string parentHostResourcesString)
        {
            Console.WriteLine(
                "Creating a resource pool:\n" +
                "\tPool Id: " + childPoolId +
                "\n\tPool Name: " + childPoolName);
            
            ResourceUtilities.DisplayResourceInformation(
                resourceDisplayName);

            ResourcePoolUtilities.DisplayPoolIdAndHostResources(
                parentPoolIdsString,
                parentHostResourcesString);

            string[] poolDelimter = { "[p]" };
            
            //
            // Pool IDs are delimted by "[p], e.g.
            //     "[p]Child Pool A[p][p]Child Pool B[p]"
            //
            string[] parentPoolIdArray = ResourcePoolUtilities.GetOneDimensionalArray(
                parentPoolIdsString,
                poolDelimter);

            string[] hostResourceDelimter = { "[h]" };
            
            //
            // Parent pool host resources are specified by a 2-D array. Each pool is delimited
            // by a "[p]";  Each host resource is delimited by  a "[h]". For example,
            //    "[p][h]Child A, Resource 1[h][h]Child A, Resource 2[h][p][p][h]Child B, Resource 1[h][p]"
            //
            string[][] parentHostResourcesArray = ResourcePoolUtilities.GetTwoDimensionalArray(
                parentHostResourcesString,
                poolDelimter,
                hostResourceDelimter);

            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject rPConfigurationService =
                MsvmResourcePoolConfigurationService.GetResourcePoolConfigurationService(
                    scope))
            {
                CreatePoolHelper(
                    scope,
                    rPConfigurationService,
                    ResourceUtilities.GetResourceType(resourceDisplayName),
                    ResourceUtilities.GetResourceSubType(resourceDisplayName),
                    childPoolId,
                    childPoolName,
                    parentPoolIdArray,
                    parentHostResourcesArray);
            }
        }

        /// <summary>
        /// Modifies the host resources assigned to a resource pool.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// service management object.</param>
        /// <param name="rPConfigurationService">The resource pool configuration</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolPath">The reference path to a resource pool.</param>
        /// <param name="parentPoolIdArray">An array of strings that specify the parent
        /// pool IDs.</param>
        /// <param name="poolName">An array of string arrays that represent the host
        /// resources</param>
        internal static void
        ModifyPoolResourcesByPath(
            ManagementScope scope,
            ManagementObject rPConfigurationService,
            string resourceType,
            string resourceSubType,
            string poolPath,
            string[] parentPoolIdArray,
            string[][] parentHostResourcesArray)
        {
            if (parentPoolIdArray.Length == 0)
            {
                throw new ManagementException(string.Format(
                    CultureInfo.CurrentCulture,
                    @"At least one parent pool must be specified when modifying a 
                    resource pool's host resources (poolPath ""{0}"")", poolPath));
            }

            if (parentPoolIdArray.Length != parentHostResourcesArray.Length)
            {
                throw new ManagementException(string.Format(
                    CultureInfo.CurrentCulture,
                    @"When modifying a child resource pool's host resources, a host 
                    resource must be specified for each parent pool. Shared allocations 
                    are not supported (poolPath ""{0}"")",
                    poolPath));
            }

            string[] parentPoolPathArray =
                ResourcePoolUtilities.GetParentPoolArrayFromPoolIds(
                    scope,
                    resourceType,
                    resourceSubType,
                    parentPoolIdArray);

            string[] resourceAllocationSettingDataArray =
                MsvmResourceAllocationSettingData.GetNewPoolAllocationSettingsArray(
                    scope,
                    resourceType,
                    resourceSubType,
                    parentPoolIdArray,
                    parentHostResourcesArray);

            using (ManagementBaseObject inParams =
                   rPConfigurationService.GetMethodParameters(
                       "ModifyPoolResources"))
            {
                inParams["ChildPool"] = poolPath;
                inParams["ParentPools"] = parentPoolPathArray;
                inParams["AllocationSettings"] = resourceAllocationSettingDataArray;

                using (ManagementBaseObject outParams =
                       rPConfigurationService.InvokeMethod(
                           "ModifyPoolResources",
                           inParams,
                           null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope, true, true);
                }
            }
        }

        /// <summary>
        /// Modifies the host resources assigned to a resource pool.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// service management object.</param>
        /// <param name="rPConfigurationService">The resource pool configuration</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID of a resource pool.</param>
        /// <param name="parentPoolIdArray">An array of strings that specify the parent
        /// pool IDs.</param>
        /// <param name="poolName">An array of string arrays that represent the host
        /// resources</param>
        internal static void
        ModifyPoolResourcesHelper(
            ManagementScope scope,
            ManagementObject rPConfigurationService,
            string resourceType,
            string resourceSubType,
            string poolId,
            string[] parentPoolIdArray,
            string[][] parentHostResourcesArray)
        {
            string poolPath =
                MsvmResourcePool.GetResourcePoolPath(
                        scope,
                        resourceType,
                        resourceSubType,
                        poolId);

            ModifyPoolResourcesByPath(
                scope,
                rPConfigurationService,
                resourceType,
                resourceSubType,
                poolPath,
                parentPoolIdArray,
                parentHostResourcesArray);
        }

        /// <summary>
        /// Modifies the host resources assigned to a resource pool.
        /// </summary>
        /// <param name="resourceDisplayName">The display name of the resource type.</param>
        /// <param name="poolId">The pool ID of the pool to modify.</param>
        /// <param name="parentPoolIdArray">A delimited string that specifies the parent
        /// pool IDs.</param>
        /// <param name="parentHostResourcesString">A delimited string that represents the host
        /// resources for each parent pool.</param>
        internal static void
        ModifyPoolResources(
            string resourceDisplayName,
            string poolId,
            string parentPoolIdsString,
            string parentHostResourcesString)
        {
            Console.WriteLine(
                "Modifying a resource pool's host resources:\n" +
                "\tPool ID: " + poolId);

            ResourceUtilities.DisplayResourceInformation(
                resourceDisplayName);

            ResourcePoolUtilities.DisplayPoolIdAndHostResources(
                parentPoolIdsString,
                parentHostResourcesString);

            string[] poolDelimiter = { "[p]" };
            string[] hostResourceDelimter = { "[h]" };

            string[] parentPoolIdArray = ResourcePoolUtilities.GetOneDimensionalArray(
                parentPoolIdsString,
                poolDelimiter);

            string[][] parentHostResourcesArray = ResourcePoolUtilities.GetTwoDimensionalArray(
                parentHostResourcesString,
                poolDelimiter,
                hostResourceDelimter);

            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject rPConfigurationService =
                MsvmResourcePoolConfigurationService.GetResourcePoolConfigurationService(
                    scope))
            {
                ModifyPoolResourcesHelper(
                    scope,
                    rPConfigurationService,
                    ResourceUtilities.GetResourceType(resourceDisplayName),
                    ResourceUtilities.GetResourceSubType(resourceDisplayName),
                    poolId,
                    parentPoolIdArray,
                    parentHostResourcesArray);
            }
        }

        /// <summary>
        /// Modifies the resource pool's settings.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// service management object.</param>
        /// <param name="rPConfigurationService">The resource pool configuration
        /// <param name="poolPath">The reference path to a resource pool.</param>
        /// <param name="resourcePoolSettingData">An embedded instance string of a
        /// Msvm_ResourcePoolSettingData object.</param>
        /// <returns>A Msvm_ResourcePool object.</returns>
        internal static void
        ModifyPoolSettingsByPath(
            ManagementScope scope,
            ManagementObject rPConfigurationService,
            string poolPath,
            string resourcePoolSettingData)
        {
            using (ManagementBaseObject inParams =
                   rPConfigurationService.GetMethodParameters(
                       "ModifyPoolSettings"))
            {
                inParams["ChildPool"] = poolPath;
                inParams["PoolSettings"] = resourcePoolSettingData;

                using (ManagementBaseObject outParams =
                       rPConfigurationService.InvokeMethod(
                           "ModifyPoolSettings",
                           inParams,
                           null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope, true, true);
                }
            }
        }

        /// <summary>
        /// Modifies the resource pool's settings.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// service management object.</param>
        /// <param name="rPConfigurationService">The resource pool configuration</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID of a resource pool.</param>
        /// <param name="resourcePoolSettingData">An embedded instance string of a
        /// Msvm_ResourcePoolSettingData object.</param>
        /// <returns>A Msvm_ResourcePool object.</returns>
        internal static void
        ModifyPoolSettingsHelper(
            ManagementScope scope,
            ManagementObject rPConfigurationService,
            string resourceType,
            string resourceSubType,
            string poolId,
            string resourcePoolSettingData)
        {
            string poolPath =
                MsvmResourcePool.GetResourcePoolPath(
                        scope,
                        resourceType,
                        resourceSubType,
                        poolId);

            ModifyPoolSettingsByPath(
                scope,
                rPConfigurationService,
                poolPath,
                resourcePoolSettingData);
        }

        /// <summary>
        /// Modifies the resource pool's settings.
        /// </summary>
        /// <param name="resourceDisplayName">The display name of the resource type.</param>
        /// <param name="poolId">The pool ID of a resource pool.</param>
        /// <param name="newPoolId">The new pool ID of the resource pool.</param>
        /// <param name="newPoolName">The new pool Name of the resource pool.</param>
        internal static void
        ModifyPoolSettings(
            string resourceDisplayName,
            string poolId,
            string newPoolId,
            string newPoolName)
        {
            Console.WriteLine(
                "Modifying a resource pool's settings:\n" +
                "\tPool ID: " + poolId + " (change to " + newPoolId + ")\n" +
                "\tPool Name: (change to " + newPoolName + ")");

            ResourceUtilities.DisplayResourceInformation(
                resourceDisplayName);
            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject rPConfigurationService =
                MsvmResourcePoolConfigurationService.GetResourcePoolConfigurationService(
                    scope))
            {
                string resourcePoolSettingData =
                    MsvmResourcePoolSettingData.GetSettingsForPool(
                        scope,
                        ResourceUtilities.GetResourceType(resourceDisplayName),
                        ResourceUtilities.GetResourceSubType(resourceDisplayName),
                        newPoolId,
                        newPoolName);

                ModifyPoolSettingsHelper(
                    scope,
                    rPConfigurationService,
                    ResourceUtilities.GetResourceType(resourceDisplayName),
                    ResourceUtilities.GetResourceSubType(resourceDisplayName),
                    poolId,
                    resourcePoolSettingData);
            }
        }

        /// <summary>
        /// Delete a resource pool.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// service management object.</param>
        /// <param name="rPConfigurationService">The resource pool configuration.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        internal static void
        DeletePoolHelper(
            ManagementScope scope,
            ManagementObject rPConfigurationService,
            string resourceType,
            string resourceSubType,
            string poolId)
        {
            string poolPath =
                MsvmResourcePool.GetResourcePoolPath(
                        scope,
                        resourceType,
                        resourceSubType,
                        poolId);

            using (ManagementBaseObject inParams =
                  rPConfigurationService.GetMethodParameters(
                      "DeletePool"))
            {
                inParams["Pool"] = poolPath;

                using (ManagementBaseObject outParams =
                       rPConfigurationService.InvokeMethod(
                           "DeletePool",
                           inParams,
                           null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope, true, true);
                }
            }
        }

        /// <summary>
        /// Delete a resource pool.
        /// </summary>
        /// <param name="resourceDisplayName">The display name of the resource type to assign.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        internal static void
        DeletePool(
            string resourceDisplayName,
            string poolId)
        {
            Console.WriteLine(
                "Deleting a resource pool:\n" +
                "\tPool Id: " + poolId);

            ResourceUtilities.DisplayResourceInformation(
                resourceDisplayName);

            ManagementScope scope = ResourcePoolUtilities.GetManagementScope();

            using (ManagementObject rPConfigurationService =
                MsvmResourcePoolConfigurationService.GetResourcePoolConfigurationService(
                    scope))
            {
                DeletePoolHelper(
                    scope,
                    rPConfigurationService,
                    ResourceUtilities.GetResourceType(resourceDisplayName),
                    ResourceUtilities.GetResourceSubType(resourceDisplayName),
                    poolId);
            }
        }
    }
}