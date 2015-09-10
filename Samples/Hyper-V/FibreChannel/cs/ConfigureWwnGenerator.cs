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

    static class ConfigureWwnGenerator
    {
        /// <summary>
        /// The WWN Generator uses the MinimumWWPNAddress, MaximumWWPNAddress
        /// and CurrentWWNNAddress of the Msvm_VirtualSystemManagementServiceSettingData.
        /// These properties are modified through the ModifyServiceSettings() method
        /// of the Msvm_VirtualSystemManagementService class.
        /// </summary>
        /// <param name="minWwpn">The Minimum WWPN for the WWPN range.</param>
        /// <param name="maxWwpn">The Maximum WWPN for the WWPN range.</param>
        /// <param name="newWwnn">The new WWNN value. If null, do not modify the CurrentWWNNAddress.</param>
        private static void
        ConfigWwnGenerator(
            string minWwpn,
            string maxWwpn,
            string newWwnn)
        {
            Console.WriteLine("Trying to configure the WWN generator with:");
            Console.WriteLine("\tMinimumWWPNAddress = {0}", minWwpn);
            Console.WriteLine("\tMaximumWWPNAddress = {0}", maxWwpn);

            if (newWwnn == null)
            {
                Console.WriteLine("\tThe CurrentWWNNAddress will not be modified.");
            }
            else
            {
                Console.WriteLine("\tCurrentWWNNAddress = {0}", newWwnn);
            }

            ManagementScope scope = FibreChannelUtilities.GetFcScope();

            using (ManagementObject service =
                   WmiUtilities.GetVirtualMachineManagementService(scope))
            using (ManagementObject settings =
                   WmiUtilities.GetVirtualMachineManagementServiceSettings(scope))
            using (ManagementBaseObject inParams =
                   service.GetMethodParameters("ModifyServiceSettings"))
            {
                settings["MinimumWWPNAddress"] = minWwpn;
                settings["MaximumWWPNAddress"] = maxWwpn;
                if (newWwnn != null)
                {
                    settings["CurrentWWNNAddress"] = newWwnn;
                }

                inParams["SettingData"] = settings.GetText(TextFormat.WmiDtd20);
                using (ManagementBaseObject outParams =
                       service.InvokeMethod("ModifyServiceSettings",
                                            inParams,
                                            null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope, true, true);
                }
            }

            Console.WriteLine("Successfully configured the WWN Generator.");
        }

        /// <summary>
        /// Entry point for the CreateSan sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length < 2 || (args.Length > 0 && args[0] == "/?"))
            {
                Console.WriteLine("Usage: ConfigureWwnGenerator <Min WWPN> <Max WWPN> [New WWNN]");
                return;
            }

            try
            {
                string minWwpn = args[0];
                string maxWwpn = args[1];
                string newWwnn = null;

                if (args.Length > 2)
                {
                    newWwnn = args[2];
                }

                ConfigWwnGenerator(minWwpn, maxWwpn, newWwnn);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to configure WWN generator. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}