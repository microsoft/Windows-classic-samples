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

    static class CreateSanSample
    {
        /// <summary>
        /// Creates a Virtual SAN with the name (poolId) and host resources.
        /// </summary>
        /// <param name="poolId">The name of the virtual SAN.</param>
        /// <param name="notes">User defined notes for the virtual SAN.</param>
        /// <param name="hostResources">The host resources to be assigned to the virtual SAN.</param>
        internal static void
        CreateSan(
            string poolId,
            string notes,
            string[] hostResources)
        {
            Console.WriteLine("Creating Virtual SAN - {0} ...", poolId);

            ManagementScope scope = FibreChannelUtilities.GetFcScope();
            string resourcePoolSettingData =
                FibreChannelUtilities.GetSettingsForPool(scope, poolId, notes);

            //
            // Fibre Channel Connection Resource Pools have only 1 parent, the primordial pool.
            //
            string[] parentPoolPathArray = new string[1];
            parentPoolPathArray[0] = FibreChannelUtilities.GetResourcePoolPath(scope, null);

            string[] resourceAllocationSettingDataArray = new string[1];
            resourceAllocationSettingDataArray[0] =
                FibreChannelUtilities.GetNewPoolAllocationSettings(scope, poolId, hostResources);

            using (ManagementObject rpConfigurationService =
                   FibreChannelUtilities.GetResourcePoolConfigurationService(scope))
            using (ManagementBaseObject inParams =
                   rpConfigurationService.GetMethodParameters("CreatePool"))
            {
                inParams["PoolSettings"] = resourcePoolSettingData;
                inParams["ParentPools"] = parentPoolPathArray;
                inParams["AllocationSettings"] = resourceAllocationSettingDataArray;

                using (ManagementBaseObject outParams =
                       rpConfigurationService.InvokeMethod(
                           "CreatePool",
                           inParams,
                           null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope, true, true);
                }
            }

            Console.WriteLine("Successfully Created Virtual SAN: {0}", poolId);
        }

        /// <summary>
        /// Entry point for the CreateSan sample. 
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length < 3 || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: CreateSan <SanName> <[WWPN WWNN]+> [SanNotes]");
                return;
            }

            try
            {
                string sanName = args[0];
                string sanNotes = @"Notes for virtual SAN - " + sanName;
                List<string> hostResources = new List<string>();

                if (args.Length % 2 == 0)
                {
                    sanNotes = args[args.Length - 1];
                }

                for (int index = 2; index < args.Length; index += 2)
                {
                    WorldWideName wwn = new WorldWideName();
                    wwn.PortName = args[index - 1];
                    wwn.NodeName = args[index];
                    //
                    // Convert the WWN to the path of the corresponding Virtual FC Switch to be used
                    // as HostResources for the ResourcePool.
                    //
                    hostResources.Add(FibreChannelUtilities.GetHostResourceFromWwn(wwn));
                }

                CreateSan(sanName, sanNotes, hostResources.ToArray());
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to create san. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
