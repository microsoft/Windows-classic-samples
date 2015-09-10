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
    
    static class ResourcePoolUtilities
    {
        /// <summary>
        /// Returns an embedded instance string of an Msvm_ResourcePoolSettingData
        /// object initialized with the specified pool ID, pool name, and resource
        /// information.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolId">The pool ID to assign.</param>
        /// <param name="poolName">The pool name to assign.</param>
        /// <returns>The embedded instance string of an
        /// Msvm_ResourcePoolConfigurationService object.</returns>
        internal static ManagementScope
        GetManagementScope()
        {
            return new ManagementScope(@"root\virtualization\v2");
        }

        /// <summary>
        /// Returns an string array of reference paths for each specified pool ID.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <param name="rPConfigurationService">The Resource Pool
        /// Configuration Service MOB.</param>
        /// <param name="resourceType">The resource type to assign.</param>
        /// <param name="resourceSubType">The resource subtype to assign.</param>
        /// <param name="poolIdArray">An ArrayList of pool IDs.</param>
        /// resources.</param>
        /// <returns>An array of reference paths for Msvm_ResourcePool objects.</returns>
        internal static string[]
        GetParentPoolArrayFromPoolIds(
            ManagementScope scope,
            string resourceType,
            string resourceSubType,
            string[] poolIdArray)
        {
            List<string> pathList = new List<string>();

            foreach (string poolId in poolIdArray)
            {
                pathList.Add(
                    MsvmResourcePool.GetResourcePoolPath(
                        scope,
                        resourceType,
                        resourceSubType,
                        poolId));
            }

            return pathList.ToArray();
        }

        /// <summary>
        /// Extracts an array of strings from the specified string. Each string
        /// is delimited by the specified delimiter.
        /// 
        /// This can be used to parse command line arguments that contain an
        /// array of values.
        /// 
        /// For example, if "[p]" is the delimiter, the following string:
        ///     [p]Child 1[p][p]Child 2[p]
        /// 
        /// Would yield a string array with the following entries:
        ///     "Child 1"
        ///     "Child 2"
        /// </summary>
        /// <param name="delimitedString">A string containing one or more delimited
        /// pooId strings.</param>
        /// <param name="delimiter">A string array specifying the delimiter.</param>
        /// <returns>An array of parent strings.
        /// </returns>
        internal static string[]
        GetOneDimensionalArray(
            string delimitedString,
            string[] delimiter)
        {
            string[] poolIdArray = delimitedString.Split(
                delimiter,
                StringSplitOptions.RemoveEmptyEntries);

            if (poolIdArray.Length == 0)
            {
                string[] primordialPoolId = new string[] { string.Empty };

                return primordialPoolId;
            }

            return poolIdArray;
        }

        /// <summary>
        /// Returns an array of string arrays that represent an array of host resources
        /// for each pool. This can be used to parse command line arguments.
        /// A pool's host resources are delimited by "[p]". Each individual host
        /// resource is delimited by [h]. For example:
        /// </summary>
        /// <param name="delimitedString">A string containing one or more delimited
        /// parent strings.</param>
        /// <returns>An array of parent strings.</returns>
        /// <summary>
        /// Extracts an array of string arrays from the specified string. Each string
        /// is delimited by the specified delimiters.
        /// 
        /// This can be used to parse command line arguments that contain
        /// a 2-D array of values.
        /// 
        /// For example, if "[p]" is the delimiter for the first dimension and "[h]"
        /// is the delimiter for the second dimension, the following string:
        ///     [p][h]Pool 1, Resource 1[h][h]Pool 2, Resource 2[h][p][h]Pool2, Resource 1[h][p]
        /// 
        /// Would yield a string array with the following entries:
        ///     {
        ///         { "Pool 1, Resource 1", "Pool 1 Resource 2"},
        ///         { "Pool 2, Resource 1" }
        ///     }
        /// <param name="dimension1Delimiter">A string array specifying the delimiter for the
        /// first dimension.</param>
        /// <param name="dimension2Delimiter">A string array specifying the delimiter for the
        /// second dimension.</param>
        /// <returns>An array of string arrays.
        /// </returns>
        internal static string[][]
        GetTwoDimensionalArray(
            string delimitedString,
            string[] dimension1Delimiter,
            string[] dimension2Delimiter)
        {
            string[] hostResourceArray = delimitedString.Split(
                dimension1Delimiter,
                StringSplitOptions.RemoveEmptyEntries);

            List<string[]> poolHostResourceList = new List<string[]>();

            foreach (string resource in hostResourceArray)
            {
                poolHostResourceList.Add(
                    resource.Split(
                    dimension2Delimiter,
                    StringSplitOptions.RemoveEmptyEntries));
            }

            return poolHostResourceList.ToArray();
        }

        
        /// <summary>
        /// Displays an array of poolId and host resource for the specified
        /// delimited data.
        /// </summary>
        /// <param name="poolIdString">A string containing one or more delimited
        /// pool Id strings.</param>
        /// <param name="hostResourcesString">A delimited string containing a 2 dimensional
        /// array of host resource strings.</param>
        internal static void
        DisplayPoolIdAndHostResources(
            string poolIdString,
            string hostResourcesString)
        {
            string[] poolDelimiter = { "[p]" };
            string[] hostResourceDelimiter = { "[p]" };

            string[] poolIdArray = ResourcePoolUtilities.GetOneDimensionalArray(
                poolIdString,
                poolDelimiter);

            string[][] hostResourcesArray = ResourcePoolUtilities.GetTwoDimensionalArray(
                hostResourcesString,
                poolDelimiter,
                hostResourceDelimiter);

            if (poolIdArray.Length != hostResourcesArray.Length)
            {
                throw new ManagementException("Number of pool Ids (" + poolIdArray.Length +
                    ") specified does not equal the number of host resources (" +
                    hostResourcesArray.Length + ").");
            }

            for (int index = 0; index < poolIdArray.Length; ++index)
            {
                string poolId = poolIdArray[index];
                string[] hostResources = hostResourcesArray[index];

                Console.WriteLine("\tParent pool Id " + poolId + ":");

                foreach (string resource in hostResources)
                {
                    Console.WriteLine("\t\tResource: " + resource);
                }
            }
        }
    }
}
