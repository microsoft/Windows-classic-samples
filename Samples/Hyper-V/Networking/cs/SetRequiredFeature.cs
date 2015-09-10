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

    static class SetRequiredFeatureSample
    {
        /// <summary>
        /// Modifies the list of required features on all connections associated to the specified
        /// virtual machine.
        /// </summary>
        /// <param name="vmName">The name of the virtual machine.</param>
        /// <param name="featureName">The name of the feature to add or remove from the list.</param>
        /// <param name="required">True if the feature should be added to the list, false if it should be removed.</param>
        static void
        SetRequiredFeature(
            string vmName,
            string featureName,
            bool required)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            List<string> connectionsToModify = new List<string>();
           
            //
            // Find the feature capability object.
            //
            using(ManagementObject feature = NetworkingUtilities.FindFeatureByName(featureName, scope))

            //
            // Find all the Ethernet connections associated with the virtual machine.
            //
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(vmName, scope))
            using (ManagementObjectCollection connectionCollection = NetworkingUtilities.FindConnections(virtualMachine, scope))
            {
                foreach (ManagementObject connection in connectionCollection)
                    using (connection)
                {
                    //
                    // Look at the current list of required features, and find out whether the 
                    // specified feature is already present in the list. Each element of the 
                    // list is the WMI path to a feature capability object.
                    //
                    string[] requiredFeatures = (string[])connection["RequiredFeatures"];
                    int featureIndex = -1;

                    for (int idx = 0; idx < requiredFeatures.Length; ++idx)
                    {
                        using (ManagementObject requiredFeature = new ManagementObject(requiredFeatures[idx]))
                        {
                            requiredFeature.Get();

                            if (String.Equals(
                                (string)requiredFeature["ElementName"],
                                featureName,
                                StringComparison.OrdinalIgnoreCase))
                            {
                                featureIndex = idx;
                                break;
                            }
                        }
                    }

                    if (((featureIndex == -1) && !required)
                        || ((featureIndex != -1) && required))
                    {
                        //
                        // The feature is not required and not present in the list,
                        // or required and already present in the list. In either case
                        // there's nothing left to do for this connection.
                        //
                        continue;
                    }

                    string[] newRequiredFeatures = null;

                    if (required)
                    {
                        //
                        // Append the new feature at the end of list.
                        //
                        newRequiredFeatures = new string[requiredFeatures.Length + 1];

                        for (int idx = 0; idx < requiredFeatures.Length; ++idx)
                        {
                            newRequiredFeatures[idx] = requiredFeatures[idx];
                        }

                        newRequiredFeatures[newRequiredFeatures.Length - 1] = feature.Path.Path;
                    }
                    else
                    {
                        //
                        // Remove the feature from the list.
                        //
                        newRequiredFeatures = new string[requiredFeatures.Length - 1];

                        for (int idx = 0; idx < featureIndex; ++idx)
                        {
                            newRequiredFeatures[idx] = requiredFeatures[idx];
                        }

                        for (int idx = featureIndex; idx < newRequiredFeatures.Length; ++idx)
                        {
                            newRequiredFeatures[idx] = requiredFeatures[idx + 1];
                        }
                    }

                    connection["RequiredFeatures"] = newRequiredFeatures;
                    connectionsToModify.Add(connection.GetText(TextFormat.WmiDtd20));
                }
            }

            if (connectionsToModify.Count > 0)
            {
                using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))
                using (ManagementBaseObject inParams = managementService.GetMethodParameters("ModifyResourceSettings"))
                {
                    inParams["ResourceSettings"] = connectionsToModify.ToArray();
        
                    using (ManagementBaseObject outParams = 
                            managementService.InvokeMethod("ModifyResourceSettings", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Feature '{0}' is {1} the list of required features for virtual machine '{2}'.",
                featureName,
                required ? "added to" : "removed from",
                vmName));
        }

        /// <summary>
        /// Displays usage information for this sample.
        /// </summary>
        internal static void
        ShowUsage()
        {
            Console.WriteLine(
                "Usage: SetRequiredFeature VmName FeatureName (True/False)\n" +
                "\n" + 
                "Example: SetRequiredFeature MyVM \"Ethernet Switch Port Offload Settings\" true");
        }

        /// <summary>
        /// Entry point for this sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if ((args.Length != 3) || (args.Length > 0 && args[0] == "/?"))
            {
                ShowUsage();
                return;
            }

            try
            {
                string vmName = args[0];
                string featureName = args[1];
                bool required = String.Equals(args[2], "true", StringComparison.OrdinalIgnoreCase);

                SetRequiredFeature(vmName, featureName, required);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to modify switch extension. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
