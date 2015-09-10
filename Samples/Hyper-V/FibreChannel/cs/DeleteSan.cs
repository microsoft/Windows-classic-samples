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

    static class DeleteSanSample
    {
        /// <summary>
        /// Deletes a Virtual SAN by its Name.
        /// </summary>
        /// <param name="poolId">Name of the Virtual SAN to be deleted.</param>
        private static void
        DeleteSan(
            string poolId)
        {
            Console.WriteLine("Deleting Virtual SAN - {0} ...", poolId);

            ManagementScope scope = FibreChannelUtilities.GetFcScope();
            using (ManagementObject rpConfigurationService =
                   FibreChannelUtilities.GetResourcePoolConfigurationService(scope))
            using (ManagementBaseObject inParams =
                   rpConfigurationService.GetMethodParameters("DeletePool"))
            {
                inParams["Pool"] = FibreChannelUtilities.GetResourcePoolPath(scope, poolId);

                using (ManagementBaseObject outParams =
                       rpConfigurationService.InvokeMethod(
                           "DeletePool",
                           inParams,
                           null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope, true, true);
                }
            }

            Console.WriteLine("Successfully deleted Virtual SAN - {0}", poolId);
        }

        /// <summary>
        /// Entry point for the DeleteSan sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length != 1 || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: DeleteSan <SanName>\n");
                return;
            }

            try
            {
                DeleteSan(args[0]);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to delete the san. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
