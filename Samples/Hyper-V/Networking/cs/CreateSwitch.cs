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
    using Microsoft.Samples.HyperV.Common;

    static class CreateSwitchSample
    {
        /// <summary>
        /// Creates a switch with no ports. This is referred to as a "private" switch inside of 
        /// the Hyper-V UI. Any virtual machines that connect to such a switch can only communicate
        /// with other virtual machines that are running on the same host and that have also connected 
        /// to the switch.
        /// </summary>
        /// <param name="switchName">The name of the switch to create.</param>
        /// <param name="switchNotes">Some notes to associate with the switch.</param>
        static void
        CreatePrivateSwitch(
            string switchName, 
            string switchNotes)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            CreateSwitch(switchName, switchNotes, null, scope);

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "The private switch '{0}' was created successfully.", switchName));
        }

        /// <summary>
        /// Creates a switch with one port connected to the hosting computer system. This is 
        /// referred to as an "internal" switch inside of the Hyper-V UI. Any virtual machines 
        /// that connect to such a switch can communicate with the host machine and any other
        /// virtual machines that are running on the same host and that have also connected to the switch.
        /// </summary>
        /// <param name="switchName">The name of the switch to create.</param>
        /// <param name="switchNotes">Some notes to associate with the switch.</param>
        static void
        CreateInternalSwitch(
            string switchName,
            string switchNotes)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            string[] ports;

            // 
            // Get the hosting computer system. When connecting an internal port, we specify the
            // path to the hosting computer system as what the port should connect to.
            //
            using (ManagementObject hostComputerSystem = WmiUtilities.GetHostComputerSystem(scope))

            //
            // Get the default Msvm_EthernetPortAllocationSettingData instance that we can use to 
            // configure our internal port connection for the switch.
            // Use the same switch name, appended with "_Internal", for the port name. 
            // You can use any port name that you like.
            //
            using (ManagementObject portToCreate = 
                   NetworkingUtilities.GetDefaultEthernetPortAllocationSettingData(scope))
            {
                portToCreate["ElementName"] = switchName + "_Internal";
                portToCreate["HostResource"] = new string[] { hostComputerSystem.Path.Path };

                //
                // Now create the switch with the internal port.
                //
                ports = new string[] { portToCreate.GetText(TextFormat.WmiDtd20) };
            }

            CreateSwitch(switchName, switchNotes, ports, scope);

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "The internal switch '{0}' was created successfully.", switchName));
        }

        /// <summary>
        /// Creates a switch with one port connected to an external adapter. This is referred to 
        /// as an "external" switch inside of the Hyper-V UI. Any virtual machines that connect 
        /// to such a switch can communicate externally and with any other virtual machines running 
        /// on the same host that have also connected to the switch.
        /// </summary>
        /// <warning>
        /// We recommend that a configuration have multiple NICs and not specify the external 
        /// adapter your host machine is using for its external connection because specifying the
        /// external adapter will disrupt network traffic to your host machine.
        /// </warning>
        /// <param name="externalAdapterName">The external adapter to connect to.</param>
        /// <param name="switchName">The name of the switch to create.</param>
        /// <param name="switchNotes">Some notes to associate with the switch.</param>
        static void
        CreateExternalOnlySwitch(
            string externalAdapterName,
            string switchName,
            string switchNotes)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            string[] ports;

            // 
            // Get the external adapter we are connecting to. 
            //
            using (ManagementObject externalAdapter = 
                   NetworkingUtilities.FindExternalAdapter(externalAdapterName, scope))

            //
            // Get the default Msvm_EthernetPortAllocationSettingData instance that we can use to 
            // configure our external port connection for the switch.
            // Use the same switch name, appended with "_External", for the port name. 
            // You can use any port name that you like.
            //
            using (ManagementObject portToCreate =
                   NetworkingUtilities.GetDefaultEthernetPortAllocationSettingData(scope))
            {

                portToCreate["ElementName"] = switchName + "_External";
                portToCreate["HostResource"] = new string[] { externalAdapter.Path.Path };

                //
                // Now create the switch with the external port.
                //
                ports = new string[] { portToCreate.GetText(TextFormat.WmiDtd20) };
            }

            CreateSwitch(switchName, switchNotes, ports, scope);

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "The external-only switch '{0}' was created successfully.", switchName));
        }

        /// <summary>
        /// Creates a switch with two ports. One port connected to an external adapter and another 
        /// port is connected to the host computer system. This is referred to as an "external" 
        /// switch inside of the Hyper-V UI. Any virtual machines which connect to such a switch 
        /// can communicate externally, with the host computer system, and with any 
        /// other virtual machines running on the same host that have also connected to the switch.
        /// </summary>
        /// <warning>
        /// We recommend that a configuration have multiple NICs and not specify the external 
        /// adapter your host machine is using for its external connection because specifying
        /// the external adapter will disrupt network traffic to your host machine.
        /// </warning>
        /// <param name="externalAdapterName">The external adapter to connect to.</param>
        /// <param name="switchName">The name of the switch to create.</param>
        /// <param name="switchNotes">Some notes to associate with the switch.</param>
        static void
        CreateExternalSwitch(
            string externalAdapterName,
            string switchName,
            string switchNotes)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");
            string[] ports;

            // 
            // Get the external adapter we are connecting to. 
            //
            using (ManagementObject externalAdapter = 
                   NetworkingUtilities.FindExternalAdapter(externalAdapterName, scope))

            // 
            // Get the hosting computer system. The internal port we create needs to be configured
            // to connect to the hosting computer system.
            //
            using (ManagementObject hostComputerSystem = WmiUtilities.GetHostComputerSystem(scope))

            //
            // Get the default Msvm_EthernetPortAllocationSettingData instance that we can use to 
            // configure our external port connection for the switch.
            // Use the same switch name, appended with "_External", for the port name. 
            // You can use any port name that you like.
            //
            using (ManagementObject externalPortToCreate =
                   NetworkingUtilities.GetDefaultEthernetPortAllocationSettingData(scope))
            {
                externalPortToCreate["ElementName"] = switchName + "_External";
                externalPortToCreate["HostResource"] = new string[] { externalAdapter.Path.Path };

                //
                // Clone the externalPort connection and configure it for the internal port.
                // Use the same switch name, appended with "_Internal", for the port name. 
                // You can use any port name that you like.
                //
                using (ManagementObject internalPortToCreate = 
                      (ManagementObject)externalPortToCreate.Clone())
                {
                    internalPortToCreate["ElementName"] = switchName + "_Internal";
                    internalPortToCreate["HostResource"] = new string[] { hostComputerSystem.Path.Path };

                    //
                    // Copy the MAC address of the external adapter to the internal switch port.
                    //
                    internalPortToCreate["Address"] = externalAdapter["PermanentAddress"];
    
                    //
                    // Now create the switch with the two ports.
                    //
                    ports = new string[] { 
                        externalPortToCreate.GetText(TextFormat.WmiDtd20), 
                        internalPortToCreate.GetText(TextFormat.WmiDtd20) };
                }
            }

            CreateSwitch(switchName, switchNotes, ports, scope);
            
            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "The external switch '{0}' was created successfully.", switchName));
        }

        /// <summary>
        /// Creates the switch with the ports as configured by the calling method.
        /// </summary>
        /// <param name="switchName">The name of the switch to create.</param>
        /// <param name="switchNotes">Some notes to associate with the switch.</param>
        /// <param name="ports">The ports to create on the new switch. This can be null if the 
        /// switch is supposed to be 'Private'.</param>
        /// <param name="scope">The scope to use when connecting to WMI.</param>
        static void
        CreateSwitch(
            string switchName,
            string switchNotes, 
            string[] ports, 
            ManagementScope scope)
        {
            string switchSettingText;

            //
            // Create the configuration object we pass into the CreateSwitch method in order to 
            // specify details about the switch to create, such as its name and any notes.
            //
            using (ManagementClass switchSettingClass = 
                   new ManagementClass("Msvm_VirtualEthernetSwitchSettingData"))
            {
                switchSettingClass.Scope = scope;
                using (ManagementObject switchSetting = switchSettingClass.CreateInstance())
                {
                    switchSetting["ElementName"] = switchName;
                    switchSetting["Notes"] = new string[] { switchNotes };

                    //
                    // You can also configure other aspects of the switch before you create it, 
                    // such as its IOVPreferred property.
                    //
                    // switchSetting["IOVPreferred"] = true;

                    switchSettingText = switchSetting.GetText(TextFormat.WmiDtd20);
                }
            }

            //
            // Now create the switch with the specified ports.
            //
            using (ManagementObject switchService = NetworkingUtilities.GetEthernetSwitchManagementService(scope))
            using (ManagementBaseObject inParams = switchService.GetMethodParameters("DefineSystem"))
            {
                inParams["SystemSettings"] = switchSettingText;
                inParams["ResourceSettings"] = ports;
    
                using (ManagementBaseObject outParams = 
                        switchService.InvokeMethod("DefineSystem", inParams, null))
                {
                    WmiUtilities.ValidateOutput(outParams, scope);
                }
            }
        }

        /// <summary>
        /// Entry point for the CreateSwitch sample. 
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length == 0 || args[0] == "/?")
            {
                Console.WriteLine("Usage: CreateSwitch Type [ExternalNetwork] [SwitchName] [SwitchNotes]");
                Console.WriteLine("Type can be one of the follow: Private, Internal, ExternalOnly, External");
                Console.WriteLine("ExternalNetwork must be supplied if the type is ExternalOnly or External");
                Console.WriteLine("Example: CreateSwitch External MyNetworkConnection MySwitch \"some notes about the switch\"");
                return;
            }

            try
            {
                string type = args[0];
                if (string.Equals(type, "private", StringComparison.CurrentCultureIgnoreCase))
                {
                    string switchName = (args.Length > 1) ? args[1] : "New Private Switch";
                    string notes = (args.Length > 2) ? args[2] : null;
                    CreatePrivateSwitch(switchName, notes);
                }
                else if (string.Equals(type, "internal", StringComparison.CurrentCultureIgnoreCase))
                {
                    string switchName = (args.Length > 1) ? args[1] : "New Internal Switch";
                    string notes = (args.Length > 2) ? args[2] : null;
                    CreateInternalSwitch(switchName, notes);
                }
                else if (string.Equals(type, "externalOnly", StringComparison.CurrentCultureIgnoreCase) || 
                         string.Equals(type, "external", StringComparison.CurrentCultureIgnoreCase))
                {
                    if (args.Length == 1)
                    {
                        Console.WriteLine("Must supply the external network to connect to.");
                        Console.WriteLine("Example: CreateSwitch ExternalOnly MyNetwork MySwitch Notes");
                        return;
                    }
    
                    string switchName = (args.Length > 2) ? args[2] : "New ExternalOnly Switch";
                    string notes = (args.Length > 3) ? args[3] : null;
    
                    if (string.Equals(type, "externalOnly", StringComparison.CurrentCultureIgnoreCase))
                    {
                        CreateExternalOnlySwitch(args[1], switchName, notes);
                    }
                    else
                    {
                        CreateExternalSwitch(args[1], switchName, notes);
                    }
                }
                else
                {
                    Console.WriteLine("First parameter must be the type of switch to create: Private, Internal, ExternalOnly, or External");
                    return;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to create the switch. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}
