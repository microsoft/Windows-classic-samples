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

    static class AdvancedConnectionPropertiesSample
    {
        /// <summary>
        /// Adds a feature setting to all of the specified virtual machine's connections.
        /// </summary>
        /// <param name="virtualMachineName">The name of the virtual machine.</param>
        /// <param name="featureType">The type of feature to add.</param>
        static void
        AddFeatureSettings(
            string virtualMachineName,
            NetworkingUtilities.PortFeatureType featureType)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            int connectionsCount = 0;
            string featureId = NetworkingUtilities.GetPortFeatureId(featureType);

            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))

            //
            // Find the virtual machine whose connections we want to modify.
            //
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(virtualMachineName, scope))

            //
            // Find all connections for the given virtual machine.
            //
            using (ManagementObjectCollection connections = NetworkingUtilities.FindConnections(
                    virtualMachine, scope))

            //
            // Find the default feature setting for the specified feature type (e.g. Security or ACL).
            //
            using (ManagementObject defaultFeatureSetting =
                  NetworkingUtilities.GetDefaultFeatureSetting(featureId, scope))
            {
                //
                // Set the default feature setting's properties, which will differ depending on which type
                // of feature setting it is.
                //
                switch (featureType)
                {
                    case NetworkingUtilities.PortFeatureType.Security:
                        //  
                        // For example, using a security feature setting, we can disallow MAC spoofing.
                        //
                        defaultFeatureSetting["AllowMacSpoofing"] = false;
                        break;

                    case NetworkingUtilities.PortFeatureType.Acl:
                        //
                        // For example, using an ACL feature setting, we can meter an IP range.
                        //

                        //
                        // Action may be 1 - Allow, 2 - Deny, or 3 - Meter. Here we set up a metering ACL.
                        //
                        defaultFeatureSetting["Action"] = 3;

                        //
                        // Direction may be 1 - Incoming or 2 - Outgoing. Here we set up an ACL on Outgoing traffic.
                        //
                        defaultFeatureSetting["Direction"] = 2;

                        //
                        // Applicability may be 1 - Local or 2 - Remote. Here we set up an ACL on the local endpoint.
                        //
                        defaultFeatureSetting["Applicability"] = 1;

                        //
                        // AclType may be 1 - MAC, 2 - IPv4, or 3 - IPv6. Here we set up an IPv4 ACL.
                        //
                        defaultFeatureSetting["AclType"] = 2;
                        defaultFeatureSetting["RemoteAddress"] = "*.*";
                        defaultFeatureSetting["RemoteAddressPrefixLength"] = 32;
                        break;

                    case NetworkingUtilities.PortFeatureType.Offload:
                        //
                        // For example, using an Offload feature setting, we can enable IOV.
                        //
                        defaultFeatureSetting["IOVOffloadWeight"] = 100;
                        break;

                    default:
                        //
                        // Invalid featureType Argument
                        //
                        throw new ArgumentOutOfRangeException(featureType.ToString());
                }
                string defaultFeatureSettingText = defaultFeatureSetting.GetText(TextFormat.WmiDtd20);

                connectionsCount = connections.Count;

                //
                // Now add the feature setting to each connection.
                //
                try
                {
                    foreach (ManagementObject ethernetConnectionSetting in connections)
                    {
                        using (ManagementBaseObject inParams = 
                               managementService.GetMethodParameters("AddFeatureSettings"))
                        {
                            inParams["AffectedConfiguration"] = ethernetConnectionSetting.Path.Path;
                            inParams["FeatureSettings"] = new string[] { defaultFeatureSettingText };

                            using (ManagementBaseObject outParams = 
                                   managementService.InvokeMethod("AddFeatureSettings", inParams, null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }
                        }
                    }
                }
                finally
                {
                    // Dispose of the connections.
                    foreach (ManagementObject ethernetConnectionSetting in connections)
                    {
                        ethernetConnectionSetting.Dispose();
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully modified virtual machine '{0}' so that each of its {1} connection(s) " + 
                "has an advanced {2} setting added.",
                virtualMachineName, connectionsCount, featureType.ToString()));
        }

        /// <summary>
        /// Modifies any security feature settings on the specified virtual machine's connections.
        /// </summary>
        /// <param name="virtualMachineName">The name of the virtual machine.</param>
        static void
        ModifyFeatureSettings(
            string virtualMachineName,
            NetworkingUtilities.PortFeatureType featureType)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            int connectionsCount = 0;
            
            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))

            //
            // Find the virtual machine we want to modify the connections of.
            //
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(virtualMachineName, scope))

            //
            // Find all connections for the given virtual machine.
            //
            using (ManagementObjectCollection connections = NetworkingUtilities.FindConnections(
                    virtualMachine, scope))
            {
                connectionsCount = connections.Count;
    
                //
                // Now find the security feature setting data object associated with each connection, if any.
                //
                try
                {
                    string featureClassName;
                    switch (featureType)
                    {
                        case NetworkingUtilities.PortFeatureType.Security:
                            featureClassName = "Msvm_EthernetSwitchPortSecuritySettingData";
                            break;

                        case NetworkingUtilities.PortFeatureType.Offload:
                            featureClassName = "Msvm_EthernetSwitchPortOffloadSettingData";
                            break;

                        default:
                            throw new ArgumentOutOfRangeException(featureType.ToString());
                    }

                    foreach (ManagementObject ethernetConnectionSetting in connections)
                    {
                        using (ManagementObjectCollection featureSettingCollection =
                            ethernetConnectionSetting.GetRelated(featureClassName,
                                "Msvm_EthernetPortSettingDataComponent",
                                null, null, null, null, false, null))
                        {
                            //
                            // For the security/offload feature, each connection can have no more than one instance 
                            // of the feature setting, so we can simply take the first feature setting associated 
                            // with each connection. For other features, it may be necessary to do extra work.
                            //
                            if (featureSettingCollection.Count == 0)
                            {
                                continue;
                            }

                            string featureText;
                            using (ManagementObject featureSetting =
                                   WmiUtilities.GetFirstObjectFromCollection(featureSettingCollection))
                            {
                                switch (featureType)
                                {
                                    case NetworkingUtilities.PortFeatureType.Security:
                                        //
                                        // Modify the feature setting's properties. Here, we enable MAC spoofing.
                                        //
                                        featureSetting["AllowMacSpoofing"] = true;
                                        break;

                                    case NetworkingUtilities.PortFeatureType.Offload:
                                        //
                                        // Modify the feature setting's properties. Here, we modify IOVQueuePairsRequested.
                                        //
                                        featureSetting["IOVQueuePairsRequested"] = 2;
                                        break;
                                }
                                featureText = featureSetting.GetText(TextFormat.WmiDtd20);            
                            }

                            // Now apply the changes to the Hyper-V server.
                            using (ManagementBaseObject inParams =
                                   managementService.GetMethodParameters("ModifyFeatureSettings"))
                            {
                                inParams["FeatureSettings"] = new string[] { featureText };

                                using (ManagementBaseObject outParams =
                                       managementService.InvokeMethod("ModifyFeatureSettings", inParams, null))
                                {
                                    WmiUtilities.ValidateOutput(outParams, scope);
                                }
                            }
                        }
                    }
                }
                finally
                {
                    // Dispose of the connections.
                    foreach (ManagementObject ethernetConnectionSetting in connections)
                    {
                        ethernetConnectionSetting.Dispose();
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully modified virtual machine '{0}' so that each of its {1} connection(s) " +
                "has its advanced {2} settings modified.",
                virtualMachineName, connectionsCount, featureType.ToString()));
        }

        /// <summary>
        /// Finds and removes any feature settings of the specified type on the specified virtual machine's connections.
        /// </summary>
        /// <param name="virtualMachineName">The name of the virtual machine.</param>
        /// <param name="featureType">The type of the feature to be removed.</param>
        static void
        RemoveFeatureSettings(
            string virtualMachineName,
            NetworkingUtilities.PortFeatureType featureType)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            int connectionsCount = 0;

            using (ManagementObject managementService = WmiUtilities.GetVirtualMachineManagementService(scope))

            //
            // Find the virtual machine whose connections we want to modify.
            //
            using (ManagementObject virtualMachine = WmiUtilities.GetVirtualMachine(virtualMachineName, scope))
            
            //
            // Find all connections for the given virtual machine.
            //
            using (ManagementObjectCollection connections = NetworkingUtilities.FindConnections(
                    virtualMachine, scope))
            {
                string featureSettingClass;

                switch (featureType)
                {
                    case NetworkingUtilities.PortFeatureType.Security:
                        featureSettingClass = "Msvm_EthernetSwitchPortSecuritySettingData";
                        break;

                    case NetworkingUtilities.PortFeatureType.Offload:
                        featureSettingClass = "Msvm_EthernetSwitchPortOffloadSettingData";
                        break;

                    case NetworkingUtilities.PortFeatureType.Acl:
                        featureSettingClass = "Msvm_EthernetSwitchPortAclSettingData";
                        break;

                    default:
                        throw new ArgumentOutOfRangeException(featureType.ToString());
                }
                connectionsCount = connections.Count;

                //
                // Now find the feature setting data object associated with each connection, if any.
                //
                try
                {
                    foreach (ManagementObject ethernetConnectionSetting in connections)
                    {
                        List<string> featureSettingPaths = new List<string>();

                        using (ManagementObjectCollection featureSettingCollection =
                            ethernetConnectionSetting.GetRelated(featureSettingClass,
                                "Msvm_EthernetPortSettingDataComponent",
                                null, null, null, null, false, null))
                        {
                            if (featureSettingCollection.Count == 0)
                            {
                                continue;
                            }

                            foreach (ManagementObject featureSetting in featureSettingCollection)
                            using (featureSetting)
                            {
                                featureSettingPaths.Add(featureSetting.Path.Path);
                            }
                        }
    
                        using (ManagementBaseObject inParams = 
                               managementService.GetMethodParameters("RemoveFeatureSettings"))
                        {
        
                            //
                            // We specify which feature setting to remove by giving the WMI path to the feature setting object.
                            //
                            inParams["FeatureSettings"] = featureSettingPaths.ToArray();
        
                            using (ManagementBaseObject outParams = 
                                   managementService.InvokeMethod("RemoveFeatureSettings", inParams, null))
                            {
                                WmiUtilities.ValidateOutput(outParams, scope);
                            }
                        }
                    }
                }
                finally
                {
                    // Dispose of the connections.
                    foreach (ManagementObject ethernetConnectionSetting in connections)
                    {
                        ethernetConnectionSetting.Dispose();
                    }
                }
            }

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Successfully modified virtual machine '{0}' to remove advanced {2} settings from " + 
                "each of its {1} connection(s).",
                virtualMachineName, connectionsCount, featureType.ToString()));
        }

        /// <summary>
        /// Entry point for the advanced connection properties sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length == 0 || args[0] == "/?")
            {
                Console.WriteLine("Usage: AdvancedConnectionProperties (Add|Modify|Remove) VmName FeatureType \n");
                Console.WriteLine("If \"Add\" is specified we add a feature setting of the specified type for each");
                Console.WriteLine("connection from the specified virtual machine");
                Console.WriteLine("If \"Modify\" is specified we change a feature setting of the specified type for");
                Console.WriteLine("each connection from the specified virtual machine");
                Console.WriteLine("If \"Remove\" is specified we remove any feature settings of the specified type");
                Console.WriteLine("from all connections from the specified virtual machine");
                Console.WriteLine("Example: AdvancedConnectionProperties Add MyVirtualMachine Security");
                return;
            }

            string type = args[0];
            
            if (string.Equals(type, "add", StringComparison.CurrentCultureIgnoreCase))
            {
                if (args.Length != 3)
                {
                    Console.WriteLine("Must specify the virtual machine and the property type to add");
                    Console.WriteLine("(Acl, Security or Offload)\n");
                    Console.WriteLine("Example: AdvancedConnectionProperties Add MyVirtualMachine Security");
                    return;
                }
                else
                {
                    NetworkingUtilities.PortFeatureType featureType;

                    if (!Enum.TryParse<NetworkingUtilities.PortFeatureType>(args[2].ToString(), true, out featureType) ||
                        (featureType != NetworkingUtilities.PortFeatureType.Acl &&
                         featureType != NetworkingUtilities.PortFeatureType.Offload &&
                         featureType != NetworkingUtilities.PortFeatureType.Security))
                    {
                        Console.WriteLine("Valid property types to add are Acl, Security or Offload\n");
                        Console.WriteLine("Example: AdvancedConnectionProperties Add MyVirtualMachine Security");
                        return;
                    }

                    try
                    {
                        AddFeatureSettings(args[1], featureType);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Failed to add feature settings. Error message details:\n");
                        Console.WriteLine(ex.Message);
                    }
                }
            }
            else if (string.Equals(type, "modify", StringComparison.CurrentCultureIgnoreCase))
            {
                if (args.Length != 3)
                {
                    Console.WriteLine("Must specify the virtual machine and the property type to modify");
                    Console.WriteLine("(Security or Offload)\n");
                    Console.WriteLine("Example: AdvancedConnectionProperties Modify MyVirtualMachine Security");
                    return;
                }
                else
                {
                    NetworkingUtilities.PortFeatureType featureType;

                    if (!Enum.TryParse<NetworkingUtilities.PortFeatureType>(args[2], true, out featureType) ||
                        (featureType != NetworkingUtilities.PortFeatureType.Offload &&
                         featureType != NetworkingUtilities.PortFeatureType.Security))
                    {
                        Console.WriteLine("Valid property types to modify are Security or Offload.\n");
                        Console.WriteLine("Example: AdvancedConnectionProperties Remove MyVirtualMachine Security");
                        return;
                    }

                    try
                    {
                        ModifyFeatureSettings(args[1], featureType);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Failed to modify security feature settings. Error message details:\n");
                        Console.WriteLine(ex.Message);
                    }
                }
            }
            else if (string.Equals(type, "remove", StringComparison.CurrentCultureIgnoreCase))
            {
                if (args.Length != 3)
                {
                    Console.WriteLine("Must specify the virtual machine and the property type to remove");
                    Console.WriteLine("(Acl, Security or Offload)\n");
                    Console.WriteLine("Example: AdvancedConnectionProperties Remove MyVirtualMachine Security");
                    return;
                }
                else
                {
                    NetworkingUtilities.PortFeatureType featureType;

                    if (!Enum.TryParse<NetworkingUtilities.PortFeatureType>(args[2], true, out featureType) ||
                        (featureType != NetworkingUtilities.PortFeatureType.Acl &&
                         featureType != NetworkingUtilities.PortFeatureType.Offload &&
                         featureType != NetworkingUtilities.PortFeatureType.Security))
                    {
                        Console.WriteLine("Valid property types to remove are Acl, Security or Offload.\n");
                        Console.WriteLine("Example: AdvancedConnectionProperties Remove MyVirtualMachine Security");
                        return;
                    }

                    try
                    {
                        RemoveFeatureSettings(args[1], featureType);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Failed to remove feature settings. Error message details:\n");
                        Console.WriteLine(ex.Message);
                    }
                }
            }
            else
            {
                Console.WriteLine("First parameter must be \"add\", \"modify\", or \"remove\"");
                return;
            }
        }
    }
}
