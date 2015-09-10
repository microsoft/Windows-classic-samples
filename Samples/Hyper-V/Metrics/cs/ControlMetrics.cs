// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Metrics
{
    using System;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    enum MetricOperation
    {
        Enable = 2,
        Disable = 3,
        Reset = 4
    };

    static class ControlMetricsSample
    {
        /// <summary>
        /// Enables all metrics for a given virtual machine.
        /// </summary>
        /// <param name="name">The name of the virtual machine to enable metrics for.</param>
        internal static void 
        EnableMetricsForVirtualMachine(
            string name)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // Retrieve the Msvm_ComputerSystem that we want to control metrics for.
            //
            using (ManagementObject vm = WmiUtilities.GetVirtualMachine(name, scope))
            {
                //
                // Call the Msvm_MetricService::ControlMetrics method. Note that passing a null
                // metric definition indicates to control all metric definitions.
                //
                string vmPath = vm.Path.Path;

                ControlMetrics(vmPath, null, MetricOperation.Enable, scope);

                Console.WriteLine(string.Format(CultureInfo.CurrentCulture, 
                    "All metrics were successfully enabled for virtual machine \"{0}\"", name));
            }
        }

        /// <summary>
        /// Disables a single metric definition for a given network adapter.
        /// </summary>
        /// <param name="macAddress">The MAC address of the network adapter to disable metrics for.</param>
        /// <param name="ipAddress">The IP address of the port ACL to disable metrics for.</param>
        internal static void 
        DisableMetricsForNetworkAdapter(
            string macAddress,
            string ipAddress)
        {
            ManagementScope scope = new ManagementScope(@"root\virtualization\v2");

            //
            // 1. Retrieve the Msvm_SyntheticEthernetPortSettingData associated with the specified
            //    MAC address.
            // 2. Retrieve the associated Msvm_EthernetPortAllocationSettingData, which 
            //    corresponds to the connection.
            // 3. Retrieve the associated Msvm_EthernetSwitchPortAclSettingData with the specified
            //    IP address. This is the object that metrics are associated with.
            //
            string portAclSettingDataPath;

            using (ManagementObject portSettingData = 
                MetricUtilities.GetSyntheticEthernetPortSettingData(macAddress, scope))
            using (ManagementObject connectionSettingData = 
                MetricUtilities.GetEthernetPortAllocationSettingData(portSettingData, scope))
            using (ManagementObject portAclSettingData =
                MetricUtilities.GetEthernetSwitchPortAclSettingData(connectionSettingData, ipAddress, scope))
            {
                portAclSettingDataPath = portAclSettingData.Path.Path;
            }
                
            //
            // Retrieve the Msvm_BaseMetricDefinition for the Filtered Incoming Network Traffic.
            //
            string metricDefinitionPath;

            using (ManagementObject metricDefinition =
                MetricUtilities.GetMetricDefinition("Filtered Incoming Network Traffic", scope))
            {
                metricDefinitionPath = metricDefinition.Path.Path;
            }

            //
            // Call the Msvm_MetricService::ControlMetrics method.
            //
            ControlMetrics(portAclSettingDataPath, metricDefinitionPath, 
                MetricOperation.Disable, scope);

            Console.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "The metric was successfully disabled for network adapter \"{0}\" (\"{1}\")", 
                macAddress, ipAddress));
        }

        /// <summary>
        /// Acts as a wrapper around the MetricService::ControlMetrics WMI method. It is used to 
        /// enable, disable, or reset metrics.
        /// </summary>
        /// <param name="managedElementPath">The path to the managed element for which to control
        /// metrics. This can be the path to a Cim_ResourceAllocationSettingData derived class, a
        /// Msvm_ComputerSystem, or a Cim_ResourcePool derived class. Null indicates all virtual 
        /// machines.</param>
        /// <param name="metricDefinitionPath">The path to the metric definition to control metrics
        /// for. Null indicates all definitions.</param>
        /// <param name="operation">The MetricOperation (Enable, Disable, or Reset).</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        private static void
        ControlMetrics(
            string managedElementPath,
            string metricDefinitionPath,
            MetricOperation operation,
            ManagementScope scope)
        {
            using (ManagementObject metricService = MetricUtilities.GetMetricService(scope))
            {
                using (ManagementBaseObject inParams = 
                    metricService.GetMethodParameters("ControlMetrics"))
                {
                    inParams["Subject"] = managedElementPath;
                    inParams["Definition"] = metricDefinitionPath;
                    inParams["MetricCollectionEnabled"] = (uint)operation;

                    using (ManagementBaseObject outParams = 
                        metricService.InvokeMethod("ControlMetrics", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }
    }
}
