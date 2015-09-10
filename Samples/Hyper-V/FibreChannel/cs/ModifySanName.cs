// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

namespace Microsoft.Samples.HyperV.FibreChannel
{
    using System;
    using System.Management;
    using System.Globalization;
    using Microsoft.Samples.HyperV.Common;

    static class ModifySanNameSample
    {
        /// <summary>
        /// Renames a Virtual SAN.
        /// </summary>
        /// <param name="poolId">The current name of the Virtual SAN.</param>
        /// <param name="newPoolId">The new name of the Virtual SAN.</param>
        private static void
        ModifySanSettings(
            string poolId,
            string newPoolId
            )
        {
            Console.WriteLine("Modifying a Virtual SAN's settings:");
            Console.WriteLine("\tSAN Name: {0} (change to {1})", poolId, newPoolId);

            ManagementScope scope = FibreChannelUtilities.GetFcScope();

            using (ManagementObject rpConfigurationService =
                   FibreChannelUtilities.GetResourcePoolConfigurationService(scope))
            {
                string resourcePoolSettingData = FibreChannelUtilities.GetSettingsForPool(scope,
                                                                                          newPoolId,
                                                                                          null);
                string poolPath = FibreChannelUtilities.GetResourcePoolPath(scope, poolId);
                using (ManagementBaseObject inParams =
                       rpConfigurationService.GetMethodParameters("ModifyPoolSettings"))
                {
                    inParams["ChildPool"] = poolPath;
                    inParams["PoolSettings"] = resourcePoolSettingData;

                    using (ManagementBaseObject outParams =
                           rpConfigurationService.InvokeMethod(
                               "ModifyPoolSettings",
                               inParams,
                               null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope, true, true);
                    }
                }
            }

            Console.WriteLine("Successfully renamed Virtual SAN: from {0} to {1}", poolId, newPoolId);
        }

        /// <summary>
        /// Entry point for the ModifySanName sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length != 2 || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: ModifySanName <SanName> <NewSanName>");
                return;
            }

            try
            {
                ModifySanSettings(args[0], args[1]);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to modify san name. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
