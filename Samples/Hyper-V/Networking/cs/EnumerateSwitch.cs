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

    static class EnumerateSwitchSample
    {
        /// <summary>
        /// The type of network connection for a switch.
        /// </summary>
        enum SwitchConnectionType
        {
            Private,
            Internal,
            ExternalOnly,
            External
        }

        /// <summary>
        /// The type of connection for a single port.
        /// </summary>
        enum PortConnectionType
        {
            Nothing,
            Internal,
            External,
            VirtualMachine
        }

        /// <summary>
        /// Container for information about a port.
        /// </summary>
        struct PortInfo
        {
            public PortConnectionType Type;
            public string ConnectedName;
            public List<NetworkingUtilities.PortFeatureType> FeatureList;
        }

        /// <summary>
        /// Container for information about the switch.
        /// </summary>
        struct SwitchInfo
        {
            public string Name;
            public SwitchConnectionType Type;
            public List<PortInfo> PortList;
            public List<NetworkingUtilities.SwitchFeatureType> SwitchFeatureList;
        }
        
        /// <summary>
        /// Enumerates information about a switch and outputs it to the console. The information 
        /// enumerated includes: the connection type of the switch, the connected virtual machines,
        /// and the features of the switch's ports.
        /// </summary>
        /// <param name="switchName">The name of the switch to enumerate.</param>
        static void
        EnumerateSwitch(
            string switchName)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            
            //
            // Initialize our SwitchInfo structure to hold information about the switch.
            //
            SwitchInfo ethernetSwitchInfo = new SwitchInfo();
            ethernetSwitchInfo.Name = switchName;
            ethernetSwitchInfo.Type = SwitchConnectionType.Private;
            ethernetSwitchInfo.PortList = new List<PortInfo>();
            ethernetSwitchInfo.SwitchFeatureList = new List<NetworkingUtilities.SwitchFeatureType>();

            using (ManagementObject ethernetSwitch = NetworkingUtilities.FindEthernetSwitch(switchName, scope))
            {
                //
                // Enumerate the switch's ports.
                //
                using (ManagementObjectCollection portCollection = ethernetSwitch.GetRelated("Msvm_EthernetSwitchPort",
                    "Msvm_SystemDevice",
                    null, null, null, null, false, null))
                {
                    foreach (ManagementObject port in portCollection)
                    using (port)
                    {
                        //
                        // Initialize a PortInfo structure to hold information about this port.
                        //
                        PortInfo portInfo = new PortInfo();
                        portInfo.Type = PortConnectionType.Nothing;
                        portInfo.FeatureList = new List<NetworkingUtilities.PortFeatureType>();

                        //
                        // The port's connection settings are stored on its related 
                        // Msvm_EthernetPortAllocationSettingData object.
                        //
                        using (ManagementObject portSettings = WmiUtilities.GetFirstObjectFromCollection(
                            port.GetRelated("Msvm_EthernetPortAllocationSettingData",
                                "Msvm_ElementSettingData",
                                null, null, null, null, false, null)))
                        {
                            //
                            // Determine the port's connection type.
                            //
                            portInfo.Type = DeterminePortType(portSettings);

                            if (portInfo.Type == PortConnectionType.VirtualMachine)
                            {
                                // Get the name of the connected virtual machine.
                                using (ManagementObject virtualMachineSettings = WmiUtilities.GetFirstObjectFromCollection(
                                    portSettings.GetRelated("Msvm_VirtualSystemSettingData",
                                        "Msvm_VirtualSystemSettingDataComponent",
                                        null, null, null, null, false, null)))
                                {
                                    portInfo.ConnectedName = (string)virtualMachineSettings["ElementName"];
                                }
                            }
                            else if (portInfo.Type == PortConnectionType.External)
                            {
                                // Get the name of the external connection.
                                using (ManagementObject externalAdapter = new ManagementObject(
                                    ((string[])portSettings["HostResource"])[0]))
                                {
                                    portInfo.ConnectedName = (string)externalAdapter["ElementName"];
                                }
                            }

                            //
                            // Now determine which advanced properties are configured for this port. 
                            // Each Feature has its own class definition and is related to the portSettings
                            // through the Msvm_EthernetPortSettingDataComponent association.
                            //
                            using (ManagementObjectCollection portFeatureCollection =
                                    portSettings.GetRelated("Msvm_EthernetSwitchPortFeatureSettingData",
                                        "Msvm_EthernetPortSettingDataComponent",
                                        null, null, null, null, false, null))
                            {
                                foreach (ManagementObject portFeature in portFeatureCollection)
                                    using (portFeature)
                                    {
                                        portInfo.FeatureList.Add(DeterminePortFeatureType(portFeature));
                                    }
                            }
                        }

                        ethernetSwitchInfo.PortList.Add(portInfo);
                    }
                }

                //
                // Then enumerate the switch's features.
                //
                using (ManagementObject ethernetSwitchSetting = WmiUtilities.GetFirstObjectFromCollection(
                    ethernetSwitch.GetRelated("Msvm_VirtualEthernetSwitchSettingData",
                        "Msvm_SettingsDefineState",
                        null, null, null, null, false, null)))
                using (ManagementObjectCollection switchFeatures = ethernetSwitchSetting.GetRelated(
                    "Msvm_EthernetSwitchFeatureSettingData",
                    "Msvm_VirtualEthernetSwitchSettingDataComponent",
                    null, null, null, null, false, null))
                foreach (ManagementObject switchFeature in switchFeatures)
                using (switchFeature)
                {
                    ethernetSwitchInfo.SwitchFeatureList.Add(DetermineSwitchFeatureType(switchFeature));
                }
            }

            //
            // Now that we have enumerated all of the switch's ports, we can determine the 
            // switch's connection type.
            //
            ethernetSwitchInfo.Type = DetermineSwitchConnectionType(ethernetSwitchInfo.PortList);

            //
            // We now have all of the information we need - output it to the console.
            //
            OutputSwitchInfo(ethernetSwitchInfo);
        }

        /// <summary>
        /// Determine the connection type of the port.
        /// </summary>
        /// <param name="portSettings">The port configuration object.</param>
        /// <returns>The port's connection type.</returns>
        static PortConnectionType
        DeterminePortType(
            ManagementObject portSettings)
        {
            string[] hostResource = (string[])portSettings["HostResource"];

            if (hostResource != null && hostResource.Length > 0)
            {
                ManagementPath hostResourcePath = new ManagementPath(hostResource[0]);

                if (string.Equals(hostResourcePath.ClassName, "Msvm_ComputerSystem",
                    StringComparison.OrdinalIgnoreCase))
                {
                    //
                    // The port is connected to the host computer system, which means that this 
                    // is an internal connection.
                    //
                    return PortConnectionType.Internal;
                }
                else if (string.Equals(hostResourcePath.ClassName, "Msvm_ExternalEthernetPort",
                         StringComparison.OrdinalIgnoreCase))
                {
                    //
                    // The port is connected to an external Ethernet adapter.
                    //
                    return PortConnectionType.External;
                }
            }

            //
            // If it wasn't connected to the internal or external network, determine whether it 
            // is connected to a virtual machine.
            //
            string parent = (string)portSettings["Parent"];

            if (!string.IsNullOrEmpty(parent))
            {
                ManagementPath parentPath = new ManagementPath(parent);

                if (string.Equals(parentPath.ClassName, "Msvm_SyntheticEthernetPortSettingData",
                    StringComparison.OrdinalIgnoreCase) ||
                    string.Equals(parentPath.ClassName, "Msvm_EmulatedEthernetPortSettingData",
                    StringComparison.OrdinalIgnoreCase))
                {
                    return PortConnectionType.VirtualMachine;
                }
            }

            //
            // This port is not connected to anything.
            //
            return PortConnectionType.Nothing;
        }

        /// <summary>
        /// Determines the type of the port feature.
        /// </summary>
        /// <param name="portFeature">The port feature.</param>
        /// <returns>The feature's type.</returns>
        static NetworkingUtilities.PortFeatureType
        DeterminePortFeatureType(
            ManagementObject portFeature)
        {
            NetworkingUtilities.PortFeatureType type = NetworkingUtilities.PortFeatureType.Unknown;

            string portFeatureClass = portFeature.Path.ClassName;
            switch (portFeatureClass)
            {
                case "Msvm_EthernetSwitchPortOffloadSettingData":
                    type = NetworkingUtilities.PortFeatureType.Offload;
                    break;

                case "Msvm_EthernetSwitchPortVlanSettingData":
                    type = NetworkingUtilities.PortFeatureType.Vlan;
                    break;

                case "Msvm_EthernetSwitchPortAclSettingData":
                    type = NetworkingUtilities.PortFeatureType.Acl;
                    break;

                case "Msvm_EthernetSwitchPortBandwidthSettingData":
                    type = NetworkingUtilities.PortFeatureType.Bandwidth;
                    break;

                case "Msvm_EthernetSwitchPortSecuritySettingData":
                    type = NetworkingUtilities.PortFeatureType.Security;
                    break;
                    
                case "Msvm_EthernetSwitchPortProfileSettingData":
                    type = NetworkingUtilities.PortFeatureType.Profile;
                    break;
            }

            return type;
        }

        /// <summary>
        /// Determines the type of the switch feature.
        /// </summary>
        /// <param name="switchFeature">The switch feature.</param>
        /// <returns>The feature's type.</returns>
        static NetworkingUtilities.SwitchFeatureType
        DetermineSwitchFeatureType(
            ManagementObject switchFeature)
        {
            NetworkingUtilities.SwitchFeatureType type = NetworkingUtilities.SwitchFeatureType.Unknown;

            string switchFeatureClass = switchFeature.Path.ClassName;
            switch (switchFeatureClass)
            {
                case "Msvm_VirtualEthernetSwitchBandwidthSettingData":
                    type = NetworkingUtilities.SwitchFeatureType.Bandwidth;
                    break;
            }

            return type;
        }

        /// <summary>
        /// Determines the switch's connection type based upon the connection types of its ports.
        /// </summary>
        /// <param name="switchPorts">The switch's ports.</param>
        /// <returns>The switch's connection type.</returns>
        static SwitchConnectionType
        DetermineSwitchConnectionType(
            List<PortInfo> switchPorts)
        {
            SwitchConnectionType type = SwitchConnectionType.Private;

            bool internallyConnected = false;
            bool externallyConnected = false;

            //
            // Loop through the switch's ports to see if there are any internal and/or external
            // connections.
            //
            foreach (PortInfo portInfo in switchPorts)
            {
                if (portInfo.Type == PortConnectionType.Internal)
                {
                    internallyConnected = true;
                }
                else if (portInfo.Type == PortConnectionType.External)
                {
                    externallyConnected = true;
                }
            }

            //
            // Based on the internal and/or external connections of our ports, we can determine
            // the switch's connection type.
            //
            if (internallyConnected && externallyConnected)
            {
                type = SwitchConnectionType.External;
            }
            else if (internallyConnected)
            {
                type = SwitchConnectionType.Internal;
            }
            else if (externallyConnected)
            {
                type = SwitchConnectionType.ExternalOnly;
            }

            return type;
        }

        /// <summary>
        /// Outputs the information gathered on the switch to the console.
        /// </summary>
        /// <param name="switchInfo">The information gathered on the switch.</param>
        static void
        OutputSwitchInfo(
            SwitchInfo switchInfo)
        {
            Console.WriteLine("Successfully enumerated the ports and port features of the switch '{0}'\n",
                switchInfo.Name);
            Console.WriteLine("The switch's connection type is:");
            Console.WriteLine("    {0}\n", switchInfo.Type);

            //
            // If the switch is connected to an external network, output the name of the network.
            //
            if (switchInfo.Type == SwitchConnectionType.External ||
                switchInfo.Type == SwitchConnectionType.ExternalOnly)
            {
                Console.WriteLine("The switch is connected to the following network:");
                foreach (PortInfo portInfo in switchInfo.PortList)
                {
                    if (portInfo.Type == PortConnectionType.External)
                    {
                        Console.WriteLine("    {0}", portInfo.ConnectedName);
                    }
                }
                Console.WriteLine();
            }

            //
            // Output the virtual machines that are currently connected to this switch.
            //
            Console.WriteLine("The switch is connected to the following virtual machines:");
            
            bool atLeastOneVirtualMachine = false;
            foreach (string virtualMachineName in GetConnectedVirtualMachineList(switchInfo.PortList))
            {
                atLeastOneVirtualMachine = true;
                Console.WriteLine("    {0}", virtualMachineName);
            }

            if (!atLeastOneVirtualMachine)
            {
                Console.WriteLine("    The switch is not connected to any virtual machines");
            }

            Console.WriteLine();

            //
            // Output the advanced features that have been configured for the switch.
            //
            Console.WriteLine("The switch has the following advanced features: ");

            if (switchInfo.SwitchFeatureList.Count > 0)
            {
                foreach (NetworkingUtilities.SwitchFeatureType featureType in switchInfo.SwitchFeatureList)
                {
                    Console.WriteLine("    {0}", featureType.ToString());
                }
            }
            else
            {
                Console.WriteLine("    The switch has no advanced features");
            }

            Console.WriteLine();

            //
            // Now output information about features for each port.
            //
            Console.WriteLine("The switch is connected to {0} ports.", switchInfo.PortList.Count);

            if (switchInfo.PortList.Count > 0)
            {
                Console.WriteLine("Outputting port features for each port:\n");

                int count = 1;
                foreach (PortInfo portInfo in switchInfo.PortList)
                {
                    switch (portInfo.Type)
                    {
                        case PortConnectionType.VirtualMachine:
                            Console.WriteLine("    Port {0} connected to virtual machine '{1}' has the following features:",
                                count, portInfo.ConnectedName);
                            break;

                        case PortConnectionType.External:
                            Console.WriteLine("    Port {0} connected to the external network '{1}' has the following features:",
                                count, portInfo.ConnectedName);
                            break;

                        case PortConnectionType.Internal:
                            Console.WriteLine("    Port {0} connected to the internal network has the following features:",
                                count);
                            break;

                        case PortConnectionType.Nothing:
                            Console.WriteLine("    Port {0} not connected to anything has the following features:",
                                count);
                            break;
                    }

                    if (portInfo.FeatureList.Count > 0)
                    {
                        foreach (NetworkingUtilities.PortFeatureType featureType in portInfo.FeatureList)
                        {
                            Console.WriteLine("       {0}", featureType.ToString());
                        }
                    }
                    else
                    {
                        Console.WriteLine("       The port has no advanced features");
                    }

                    count++;
                    Console.WriteLine();
                }
            }
        }

        /// <summary>
        /// Helper method called by OutputSwitchInfo in order to build up a sorted list of virtual
        /// machine names that the switch is connected to.
        /// </summary>
        /// <param name="portList">The ports of the switch.</param>
        /// <returns>The sorted list of virtual machines that are connected to the switch.</returns>
        static IEnumerable<string>
        GetConnectedVirtualMachineList(
            List<PortInfo> portList)
        {
            //
            // The SortedSet class will both sort the list of virtual machine names and 
            // ignore any duplicates.
            //
            SortedSet<string> virtualMachineNames = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (PortInfo portInfo in portList)
            {
                if (portInfo.Type == PortConnectionType.VirtualMachine &&
                    !string.IsNullOrEmpty(portInfo.ConnectedName))
                {
                    virtualMachineNames.Add(portInfo.ConnectedName);
                }
            }

            return virtualMachineNames;
        }

        /// <summary>
        /// Entry point for the EnumerateSwitch sample.
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length == 0 || args[0] == "/?")
            {
                Console.WriteLine("Usage: EnumerateSwitch SwitchName\n");
                Console.WriteLine("Example: EnumerateSwitch MySwitch");
                return;
            }

            try
            {
                EnumerateSwitch(args[0]);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to enumerate the switch. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
