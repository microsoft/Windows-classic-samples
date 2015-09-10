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

    static class ModifySanPortsSample
    {
        /// <summary>
        /// Modifies the HostResources of a Virtual SAN.
        /// </summary>
        /// <param name="poolId">The Name of the Virtual SAN.</param>
        /// <param name="newHostResources">The new set of HostResources for the Virtual SAN.</param>
        private static void
        ModifySanResources(
            string poolId,
            string[] newHostResources)
        {
            if (newHostResources == null)
            {
                Console.WriteLine("Deleting all resources assigned to Virtual SAN {0} ...", poolId);
            }
            else
            {
                Console.WriteLine("Modifying resources assigned to Virtual SAN {0} ...", poolId);
            }

            ManagementScope scope = FibreChannelUtilities.GetFcScope();
            using (ManagementObject rpConfigurationService =
                   FibreChannelUtilities.GetResourcePoolConfigurationService(scope))
            {
                string poolPath = FibreChannelUtilities.GetResourcePoolPath(scope, poolId);

                string[] parentPoolPathArray = new string[1];
                parentPoolPathArray[0] = FibreChannelUtilities.GetResourcePoolPath(scope, null);

                string[] newPoolAllocationSettingsArray = new string[1];
                newPoolAllocationSettingsArray[0] =
                    FibreChannelUtilities.GetNewPoolAllocationSettings(scope, poolId, newHostResources);

                using (ManagementBaseObject inParams =
                       rpConfigurationService.GetMethodParameters("ModifyPoolResources"))
                {
                    inParams["ChildPool"] = poolPath;
                    inParams["ParentPools"] = parentPoolPathArray;
                    inParams["AllocationSettings"] = newPoolAllocationSettingsArray;

                    using (ManagementBaseObject outParams =
                           rpConfigurationService.InvokeMethod(
                               "ModifyPoolResources",
                               inParams,
                               null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope, true, true);
                    }
                }
            }

            if (newHostResources == null)
            {
                Console.WriteLine("Successfully deleted all resources assigned to Virtual SAN {0}", poolId);
            }
            else
            {
                Console.WriteLine("Successfully modified resources assigned to Virtual SAN {0}", poolId);
            }
        }

        /// <summary>
        /// Entry point for the ModifySanPorts sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length == 0 || args[0] == "/?")
            {
                Console.WriteLine("Usage: ModifySanPorts <SanName> [WWPN WWNN]*");
                return;
            }

            try
            {
                string sanName = args[0];
                List<string> switchPaths = new List<string>();

                if (args.Length >= 3 && args.Length % 2 == 1)
                {
                    for (int index = 2; index < args.Length; index += 2)
                    {
                        WorldWideName wwn = new WorldWideName();
                        wwn.PortName = args[index - 1];
                        wwn.NodeName = args[index];
                        //
                        // Convert the WWN to the path of the corresponding Virtual FC Switch to be used
                        // as HostResources for the ResourcePool.
                        //
                        switchPaths.Add(FibreChannelUtilities.GetHostResourceFromWwn(wwn));
                    }
                }
                else if (args.Length != 1)
                {
                    Console.WriteLine("Usage: ModifySanPorts sanName [WWPN WWNN]*");
                    return;
                }

                ModifySanResources(sanName, switchPaths.ToArray());
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to modify san ports. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
