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
    using Microsoft.Samples.HyperV.Common;

    static class SupportsTrunkModeSample
    {   
        /// <summary>
        /// Determines whether the external adapter supports trunk mode. The external adapter
        /// must be currently connected to a switch.
        /// </summary>
        /// <param name="externalAdapterName">The name of the external adapter.</param>
        static void
        SupportsTrunkMode(
            string externalAdapterName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            bool supportsTrunkMode = false;

            // 
            // Get the external adapter. 
            //
            using (ManagementObject externalAdapter = 
                   NetworkingUtilities.FindExternalAdapter(externalAdapterName, scope))

            //
            // From the external adapter we need to follow the associations to get to the 
            // Msvm_VLANEndpoint object that has the property we can query to see if the 
            // adapter supports trunk mode. Note however, that these associated objects only
            // exist if the adapter is connected to a switch. Until the adapter is connected to 
            // a switch there is not a way through the Hyper-V WMI API to determine whether it 
            // supports trunk mode.
            //
            using (ManagementObjectCollection lanEndpointCollection = externalAdapter.GetRelated("Msvm_LanEndpoint"))
            {
                if (lanEndpointCollection.Count == 0)
                {
                    throw new ManagementException("This external adapter is not connected to any switch. " + 
                        "Cannot determine trunk mode support.");
                }

                using (ManagementObject lanEndpoint = WmiUtilities.GetFirstObjectFromCollection(
                    lanEndpointCollection))

                using (ManagementObject otherLanEndpoint = WmiUtilities.GetFirstObjectFromCollection(
                    lanEndpoint.GetRelated("Msvm_LanEndpoint")))

                using (ManagementObject vlanEndpoint = WmiUtilities.GetFirstObjectFromCollection(
                    otherLanEndpoint.GetRelated("Msvm_VLANEndpoint")))
                {
                    //
                    // Now that we have the VLAN Endpoint, we can check its SupportedEndpointModes 
                    // property.
                    //
                    ushort[] supportedEndpointModes = (ushort[])vlanEndpoint["SupportedEndpointModes"];
                    foreach (ushort supportedMode in supportedEndpointModes)
                    {
                        if (supportedMode == 5) // 5 means "TrunkMode"
                        {
                            supportsTrunkMode = true;
                            break;
                        }
                    }
                }
            }

            if (supportsTrunkMode)
            {
                Console.WriteLine("Successfully determined that the external adapter '{0}' does support trunk mode.",
                    externalAdapterName);
            }
            else
            {
                Console.WriteLine("Successfully determined that the external adapter '{0}' does NOT support trunk mode.",
                    externalAdapterName);
            }
        }

        /// <summary>
        /// Entry point for the SupportTrunkMode sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length == 0 || args[0] == "/?")
            {
                Console.WriteLine("Usage: SupportsTrunkMode ExternalNetwork\n");
                Console.WriteLine("Example: SupportsTrunkMode MyNetwork");
                return;
            }

            try
            {
                SupportsTrunkMode(args[0]);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to determine if the external adapter supports trunk mode. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
