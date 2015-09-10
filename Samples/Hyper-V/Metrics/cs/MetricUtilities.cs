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
    
    static class MetricUtilities
    {
        /// <summary>
        /// Gets the metric service.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The metric service management object.</returns>
        internal static ManagementObject
        GetMetricService(
            ManagementScope scope)
        {
            using (ManagementClass metricServiceClass = new ManagementClass("Msvm_MetricService"))
            {
                metricServiceClass.Scope = scope;

                ManagementObject metricService = 
                    WmiUtilities.GetFirstObjectFromCollection(metricServiceClass.GetInstances());

                return metricService;
            }
        }

        /// <summary>
        /// Gets the CIM_BaseMetricDefinition derived instance that matches the specified name.
        /// </summary>
        /// <param name="name">The name of the metric definition.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The CIM_BaseMetricDefinition derived instance.</returns>
        internal static ManagementObject
        GetMetricDefinition(
            string name,
            ManagementScope scope)
        {
            string metricDefQueryWql = string.Format(CultureInfo.InvariantCulture,
                "SELECT * FROM CIM_BaseMetricDefinition WHERE ElementName=\"{0}\"", name);

            SelectQuery metricDefQuery = new SelectQuery(metricDefQueryWql);

            using (ManagementObjectSearcher metricDefSearcher = new ManagementObjectSearcher(
                scope, metricDefQuery))
            using (ManagementObjectCollection metricDefCollection = metricDefSearcher.Get())
            {
                //
                // There will always only be one metric definition for a given name.
                //
                if (metricDefCollection.Count != 1)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "A single CIM_BaseMetricDefinition derived instance could not be found " +
                        "for name \"{0}\"", name));
                }

                ManagementObject metricDef = 
                    WmiUtilities.GetFirstObjectFromCollection(metricDefCollection);

                return metricDef;
            }
        }

        /// <summary>
        /// Gets the Msvm_SyntheticEthernetPortSettingData instance that matches the requested MAC
        /// address.
        /// </summary>
        /// <param name="macAddress">The MAC address of the network adapter.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The Msvm_SyntheticEthernetPortSettingData instance.</returns>
        internal static ManagementObject
        GetSyntheticEthernetPortSettingData(
            string macAddress,
            ManagementScope scope)
        {
            // Remove any MAC address separators.
            macAddress = macAddress.Replace("-", "").Replace(":", "");

            string portSettingDataQueryWql = string.Format(CultureInfo.InvariantCulture,
                "SELECT * FROM Msvm_SyntheticEthernetPortSettingData WHERE Address=\"{0}\"", 
                macAddress);

            SelectQuery portSettingDataQuery = new SelectQuery(portSettingDataQueryWql);

            using (ManagementObjectSearcher portSettingDataSearcher = new ManagementObjectSearcher(
                scope, portSettingDataQuery))
            using (ManagementObjectCollection portSettingDataCollection = 
                portSettingDataSearcher.Get())
            {
                if (portSettingDataCollection.Count == 0)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "No Msvm_SyntheticEthernetPortSettingData could be found for MAC address \"{0}\"", 
                        macAddress));
                }

                //
                // If multiple port setting data instances exist for the requested MAC address, return 
                // the first one.
                //
                ManagementObject portSettingData = WmiUtilities.GetFirstObjectFromCollection(
                    portSettingDataCollection);

                return portSettingData;
            }
        }

        /// <summary>
        /// Gets the Msvm_EthernetPortAllocationSettingData instance for the specified parent port.
        /// This object corresponds to the connection setting data of the port.
        /// </summary>
        /// <param name="parentPort">The parent port of the connection to retrieve.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The Msvm_EthernetPortAllocationSettingData instance.</returns>
        internal static ManagementObject
        GetEthernetPortAllocationSettingData(
            ManagementObject parentPort,
            ManagementScope scope)
        {
            string connectionSettingDataQueryWql = string.Format(CultureInfo.InvariantCulture,
                "SELECT * FROM Msvm_EthernetPortAllocationSettingData WHERE Parent=\"{0}\"",
                WmiUtilities.EscapeObjectPath(parentPort.Path.Path));

            SelectQuery connectionSettingDataQuery = new SelectQuery(connectionSettingDataQueryWql);
            
            using (ManagementObjectSearcher connectionSettingDataSearcher = new ManagementObjectSearcher(
                scope, connectionSettingDataQuery))
            using (ManagementObjectCollection connectionSettingDataCollection = 
                connectionSettingDataSearcher.Get())
            {
                //
                // There will always only be one connection per port.
                //
                if (connectionSettingDataCollection.Count != 1)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "A single Msvm_EthernetPortAllocationSettingData could not be found for " +
                        "parent port \"{0}\"", parentPort.Path.Path));
                }

                ManagementObject connectionSettingData = 
                    WmiUtilities.GetFirstObjectFromCollection(connectionSettingDataCollection);

                return connectionSettingData;
            }
        }

        /// <summary>
        /// Gets the Msvm_EthernetSwitchPortAclSettingData instance that is associated with a given
        /// connection and that matches the specified IP address.
        /// </summary>
        /// <param name="ethernetConnection">The connection to retrieve the port ACL for.</param>
        /// <param name="ipAddress">The IP address of the port ACL to retrieve.</param>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The virtual machine's configuration object.</returns>
        public static ManagementObject
        GetEthernetSwitchPortAclSettingData(
            ManagementObject ethernetConnection,
            string ipAddress,
            ManagementScope scope)
        {
            using (ManagementObjectCollection portAclCollection = 
                ethernetConnection.GetRelated("Msvm_EthernetSwitchPortAclSettingData",
                    "Msvm_EthernetPortSettingDataComponent",
                    null, null, null, null, false, null))
            {
                if (portAclCollection.Count == 0)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "No associated Msvm_EthernetSwitchPortAclSettingData could be found"));
                }

                string address = ipAddress;
                byte addressPrefixLength = 32;
                
                //
                // If the IP address is in the form "A.B.C.D/N", extract the address and prefix 
                // length.
                //
                string[] addressParts = ipAddress.Split(new char[] {'/', '\\'});

                if (addressParts.Length == 2)
                {
                    address = addressParts[0];
                    addressPrefixLength = byte.Parse(addressParts[1], CultureInfo.InvariantCulture);
                }

                ManagementObject foundPortAcl = null;

                foreach (ManagementObject portAcl in portAclCollection)
                {
                    if ((portAcl["RemoteAddress"].ToString() == address) &&
                        ((byte)portAcl["RemoteAddressPrefixLength"] == addressPrefixLength))
                    {
                        foundPortAcl = portAcl;
                        break;
                    }
                }

                if (foundPortAcl == null)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "No associated Msvm_EthernetSwitchPortAclSettingData could be found " +
                        "for IP address \"{0}\"", ipAddress));
                }
                
                return foundPortAcl;
            }
        }
    }
}
