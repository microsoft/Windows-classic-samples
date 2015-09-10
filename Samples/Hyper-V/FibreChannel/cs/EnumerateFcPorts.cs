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

    static class EnumerateFcPorts
    {
        private static readonly string StatusNotHyperVCapable = "Not Hyper-V Capable";
        private static readonly string StatusAvailable = "Available";

        /// <summary>
        /// Encapsulates the information for an external FC port.
        /// </summary>
        private class FcPortInfo
        {
            internal WorldWideName PortWwn;
            internal string HostResource;
            internal string Status;
        };

        /// <summary>
        /// Encapsulates the information for a Virtual SAN.
        /// </summary>
        private struct SanInfo
        {
            internal string SanName;
            internal string[] HostResources;
        }

        /// <summary>
        /// Builds a list of SanInfo instances for each Virtual SAN configured in the system.
        /// It uses the Resource Allocation Setting Data for the virtual SAN to determine the
        /// host resources assigned to the Virtual SAN.
        /// </summary>
        /// <returns></returns>
        private static List<SanInfo>
        BuildSanInfo()
        {
            ManagementObjectCollection fcPools =
                WmiUtilities.GetResourcePools(FibreChannelUtilities.FcConnectionResourceType,
                                              FibreChannelUtilities.FcConnectionResourceSubType,
                                              FibreChannelUtilities.GetFcScope());

            List<SanInfo> sanInfoList = new List<SanInfo>();

            foreach (ManagementObject san in fcPools)
            {
                //
                // Virtual SANs are resource pools that are children of the primordial pool.
                //
                if (!(bool)san["Primordial"])
                {
                    SanInfo sanInfo = new SanInfo();
                    sanInfo.SanName = (string) san["PoolId"];

                    //
                    // The resources assigned to the Virtual SAN are present in the HostResource
                    // property of the associated Resource Allocation Setting Data.
                    //
                    using (ManagementObjectCollection poolRasds =
                           san.GetRelated("Msvm_FcPortAllocationSettingData"))
                    using (ManagementObject allocationSettingData =
                           WmiUtilities.GetFirstObjectFromCollection(poolRasds))
                    {
                        sanInfo.HostResources = (string[])allocationSettingData["HostResource"];
                    }
                    sanInfoList.Add(sanInfo);
                }
            }

            if (sanInfoList.Count == 0)
            {
                Console.WriteLine("No Virtual SANs detected in the system.");
            }
            return sanInfoList;
        }

        /// <summary>
        /// Check whether the port is Hyper-V Capable and return the appropriate status.
        /// The boolean IsHyperVCapable property of the Msvm_ExternalFcPort class is used for this.
        /// </summary>
        /// <param name="port">Instance of Msvm_ExternalFcPort</param>
        /// <returns>"Available" or "Not Hyper-V Capable"</returns>
        private static string
        GetPortStatus(
            ManagementObject port)
        {
            string status = StatusAvailable;

            if (!(bool)port["IsHyperVCapable"])
            {
                status = StatusNotHyperVCapable;
            }

            return status;
        }

        /// <summary>
        /// Builds a list of FcPortInfo instances for each external FC port.
        /// We do not associate the SAN info yet.
        /// </summary>
        /// <returns>List of FcPortInfo instances for each instance of Msvm_ExternalFcPort.</returns>
        private static List<FcPortInfo>
        BuildPortInfo()
        {
            List<FcPortInfo> portInfoList = new List<FcPortInfo>();

            ManagementScope scope = FibreChannelUtilities.GetFcScope();
            using (ManagementClass portClass = new ManagementClass("Msvm_ExternalFcPort"))
            {
                portClass.Scope = scope;
                using (ManagementObjectCollection fcPorts = portClass.GetInstances())
                {
                    foreach (ManagementObject port in fcPorts)
                    {
                        WorldWideName wwn = new WorldWideName();
                        wwn.NodeName = (string)port["WWNN"];
                        wwn.PortName = (string)port["WWPN"];

                        FcPortInfo portInfo = new FcPortInfo();
                        portInfo.PortWwn = wwn;

                        //
                        // Convert the WWN to the path of the corresponding Virtual FC Switch to be used
                        // as HostResources for the ResourcePool.
                        //
                        portInfo.HostResource = FibreChannelUtilities.GetHostResourceFromWwn(wwn);
                        portInfo.Status = GetPortStatus(port);

                        portInfoList.Add(portInfo);
                    }
                }
            }

            if (portInfoList.Count == 0)
            {
                Console.WriteLine("No FibreChannel Ports found in the system.");
            }

            return portInfoList;
        }

        /// <summary>
        /// Generates as output a list of WWNN, WWPN and Status for each external FC port.
        /// If the port is not HyperVCapable, then status is Not Hyper-V Capable.
        /// If the port is assigned to a Virtual SAN, then the status will be the SAN Name.
        /// Else the status will be Available.
        /// </summary>
        private static void
        OutputFcPortInfo()
        {
            List<SanInfo> sanInfoList = BuildSanInfo();
            List<FcPortInfo> portInfoList = BuildPortInfo();

            //
            // Update the port Status by checking if it is assigned to a Virtual SAN.
            // The HostResource of the external FC port must be present in the
            // HostResources array for the Virtual SAN.
            // A port can only be assigned to 1 Virtual SAN.
            //
            foreach (FcPortInfo portInfo in portInfoList)
            {
                foreach (SanInfo sanInfo in sanInfoList)
                {
                    if (sanInfo.HostResources == null)
                    {
                        continue;
                    }

                    foreach (string sanResource in sanInfo.HostResources)
                    {
                        if (string.Equals(sanResource, portInfo.HostResource, StringComparison.OrdinalIgnoreCase))
                        {
                            portInfo.Status = sanInfo.SanName;
                        }
                    }
                }
            }

            if (portInfoList.Count != 0)
            {
                Console.WriteLine();
                Console.WriteLine("WorldWideNodeName\tWorldWidePortName\tStatus");
                Console.WriteLine("-----------------\t-----------------\t------");
            }

            foreach (FcPortInfo portInfo in portInfoList)
            {
                Console.WriteLine("{0} \t{1} \t{2}",
                                  portInfo.PortWwn.NodeName,
                                  portInfo.PortWwn.PortName,
                                  portInfo.Status);
            }
        }

        /// <summary>
        /// Entry point for the CreateSan sample. 
        /// </summary>
        /// <param name="args">The command line arguments.</param>
        internal static void
        ExecuteSample(
            string[] args)
        {
            if (args.Length > 0)
            {
                Console.WriteLine("Usage: EnumerateFcPorts");
                return;
            }

            try
            {
                OutputFcPortInfo();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to enumerate FC ports. Error message details:\n");
                Console.WriteLine(ex.Message);
            }
        }
    }
}