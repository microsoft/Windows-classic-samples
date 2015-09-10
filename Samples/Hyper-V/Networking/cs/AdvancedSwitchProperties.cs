// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Networking
{
    using System;
    using System.Management;
    using System.Globalization;
    using System.Collections.Generic;
    using Microsoft.Samples.HyperV.Common;

    static class AdvancedSwitchPropertiesSample
    {
        /// <summary>
        /// Adds a bandwidth feature setting to the specified virtual switch.
        /// </summary>
        /// <param name="switchName">The name of the virtual switch.</param>
        static void
        AddFeatureSettings(
            string switchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            string featureId = NetworkingUtilities.GetSwitchFeatureId(NetworkingUtilities.SwitchFeatureType.Bandwidth);

            using (ManagementObject managementService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))

            //
            // Find the specified switch.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))
            using (ManagementObject ethernetSwitchSetting = WmiUtilities.GetFirstObjectFromCollection(
                ethernetSwitch.GetRelated("Msvm_VirtualEthernetSwitchSettingData",
                    "Msvm_SettingsDefineState",
                    null, null, null, null, false, null)))

            //
            // Add a new bandwidth setting instance.
            //
            using (ManagementObject bandwidthSetting =
                  NetworkingUtilities.GetDefaultFeatureSetting(featureId, scope))
            {
                //
                // Set the default bandwidth reservation to 10 Mbps.
                //
                bandwidthSetting["DefaultFlowReservation"] = 12500000; // in bytes/sec

                using (ManagementBaseObject inParams =
                        managementService.GetMethodParameters("AddFeatureSettings"))
                {
                    inParams["AffectedConfiguration"] = ethernetSwitchSetting.Path.Path;
                    inParams["FeatureSettings"] = new string[] { bandwidthSetting.GetText(TextFormat.WmiDtd20) };

                    using (ManagementBaseObject outParams =
                            managementService.InvokeMethod("AddFeatureSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully added bandwidth feature setting to virtual switch '{0}'.",
                switchName));
        }

        /// <summary>
        /// Modifies the existing bandwidth feature setting to the specified virtual switch.
        /// </summary>
        /// <param name="switchName">The name of the virtual switch.</param>
        static void
        ModifyFeatureSettings(
            string switchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject managementService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))

            //
            // Find the specified switch.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))
            using (ManagementObject ethernetSwitchSetting = WmiUtilities.GetFirstObjectFromCollection(
                ethernetSwitch.GetRelated("Msvm_VirtualEthernetSwitchSettingData",
                    "Msvm_SettingsDefineState",
                    null, null, null, null, false, null)))
            
            //
            // Find the existing bandwidth setting and modify it.
            //
            using (ManagementObject bandwidthSetting = WmiUtilities.GetFirstObjectFromCollection(
                ethernetSwitchSetting.GetRelated("Msvm_VirtualEthernetSwitchBandwidthSettingData",
                    "Msvm_VirtualEthernetSwitchSettingDataComponent",
                    null, null, null, null, false, null)))
            {
                //
                // Increase the current reservation by 1 Mbps.
                //
                bandwidthSetting["DefaultFlowReservation"] = 
                    (UInt64)bandwidthSetting["DefaultFlowReservation"] + 125000; // in bytes/sec

                using (ManagementBaseObject inParams =
                        managementService.GetMethodParameters("ModifyFeatureSettings"))
                {
                    inParams["FeatureSettings"] = new string[] { bandwidthSetting.GetText(TextFormat.WmiDtd20) };

                    using (ManagementBaseObject outParams =
                            managementService.InvokeMethod("ModifyFeatureSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully modified bandwidth feature setting on virtual switch '{0}'.",
                switchName));
        }

        /// <summary>
        /// Modifies the existing bandwidth feature setting to the specified virtual switch.
        /// </summary>
        /// <param name="switchName">The name of the virtual switch.</param>
        static void
        RemoveFeatureSettings(
            string switchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            using (ManagementObject managementService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))

            //
            // Find the specified switch.
            //
            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))
            using (ManagementObject ethernetSwitchSetting = WmiUtilities.GetFirstObjectFromCollection(
                ethernetSwitch.GetRelated("Msvm_VirtualEthernetSwitchSettingData",
                    "Msvm_SettingsDefineState",
                    null, null, null, null, false, null)))

            //
            // Find the existing bandwidth setting and remove it.
            //
            using (ManagementObject bandwidthSetting = WmiUtilities.GetFirstObjectFromCollection(
                ethernetSwitchSetting.GetRelated("Msvm_VirtualEthernetSwitchBandwidthSettingData",
                    "Msvm_VirtualEthernetSwitchSettingDataComponent",
                    null, null, null, null, false, null)))
            {
                using (ManagementBaseObject inParams =
                        managementService.GetMethodParameters("RemoveFeatureSettings"))
                {
                    inParams["FeatureSettings"] = new string[] { bandwidthSetting.Path.Path };

                    using (ManagementBaseObject outParams =
                            managementService.InvokeMethod("RemoveFeatureSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully removed bandwidth feature setting from virtual switch '{0}'.",
                switchName));
        }

        /// <summary>
        /// Displays usage information for this sample.
        /// </summary>
        internal static void
        ShowUsage()
        {
            Console.WriteLine(
                "Usage: AdvancedSwitchProperties (Add|Modify|Remove) SwitchName\n" +
                "\n" +
                "Add   : Adds bandwidth settings to the specified switch to configure\n" +
                "        the default absolute bandwidth reservation.\n" +
                "Modify: Change the current bandwidth settings for the specified switch\n" +
                "        (increases the current default reservation).\n" +
                "Remove: Remove the current bandwidth settings from the specified switch.");
        }

        /// <summary>
        /// Entry point for the advanced connection properties sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length != 2 || args[0] == "/?")
            {
                ShowUsage();
                return;
            }

            string operation = args[0].ToLowerInvariant();
            string switchName = args[1];

            try
            {
                switch (operation)
                {
                    case "add":
                        AddFeatureSettings(switchName);
                        break;

                    case "modify":
                        ModifyFeatureSettings(switchName);
                        break;

                    case "remove":
                        RemoveFeatureSettings(switchName);
                        break;

                    default:
                        ShowUsage();
                        return;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to {0} switch settings. Error message details:\n", operation);
                Console.WriteLine(ex.Message);
            }
        }
    }
}
