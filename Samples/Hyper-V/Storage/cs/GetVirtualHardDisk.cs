// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Storage
{
    using System;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    static class StorageGetSample
    {
        /// <summary>
        /// Retrieves information for a VHD/VHDX.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="VirtualHardDiskPath">The path to the VHD/VHDX.</param>
        internal static void
        GetVirtualHardDisk(
            string ServerName,
            string VirtualHardDiskPath)
        {
            VirtualHardDiskSettingData virtualHardDiskSettingData;
            VirtualHardDiskState virtualHardDiskState;
            
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                //
                // Get VirtualHardDiskSettingData.
                //

                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("GetVirtualHardDiskSettingData"))
                {
                    inParams["Path"] = VirtualHardDiskPath;

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "GetVirtualHardDiskSettingData", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);

                        virtualHardDiskSettingData 
                            = VirtualHardDiskSettingData.Parse(
                                outParams["SettingData"].ToString());
                    }
                }

                //
                // Get VirtualHardDiskState.
                //

                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("GetVirtualHardDiskState"))
                {
                    inParams["Path"] = VirtualHardDiskPath;

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "GetVirtualHardDiskState", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);

                        virtualHardDiskState 
                            = VirtualHardDiskState.Parse(
                                outParams["State"].ToString());
                    }
                }

                //
                // Display results.
                //

                Console.Write("Path:\t\t\t{0}", virtualHardDiskSettingData.Path);

                if (virtualHardDiskSettingData.DiskFormat == VirtualHardDiskFormat.Vhd)
                {
                    Console.Write(" (vhd)");
                }
                else if (virtualHardDiskSettingData.DiskFormat == VirtualHardDiskFormat.Vhdx)
                {
                    Console.Write(" (vhdx)");
                }

                if (virtualHardDiskSettingData.DiskType == VirtualHardDiskType.FixedSize)
                {
                    Console.WriteLine(" (Fixed Disk)");
                    Console.WriteLine("FragmentationPercentage:{0}", virtualHardDiskState.FragmentationPercentage);
                }
                else if (virtualHardDiskSettingData.DiskType == VirtualHardDiskType.DynamicallyExpanding)
                {
                    Console.WriteLine(" (Dynamically Expanding Disk)");
                    Console.WriteLine("MaxInternalSize:\t{0}", virtualHardDiskSettingData.MaxInternalSize);
                    Console.WriteLine("BlockSize:\t\t{0}", virtualHardDiskSettingData.BlockSize);
                    Console.WriteLine("Alignment:\t\t{0}", virtualHardDiskState.Alignment);
                    Console.WriteLine("FragmentationPercentage:{0}", virtualHardDiskState.FragmentationPercentage);
                }
                else if (virtualHardDiskSettingData.DiskType == VirtualHardDiskType.Differencing)
                {
                    Console.WriteLine(" (Differencing Disk)");
                    Console.WriteLine("Parent:\t\t\t{0}", virtualHardDiskSettingData.ParentPath);
                    Console.WriteLine("MaxInternalSize:\t{0}", virtualHardDiskSettingData.MaxInternalSize);
                    Console.WriteLine("BlockSize:\t\t{0}", virtualHardDiskSettingData.BlockSize);
                    Console.WriteLine("Alignment:\t\t{0}", virtualHardDiskState.Alignment);
                }

                Console.WriteLine("FileSize:\t\t{0}", virtualHardDiskState.FileSize);
                Console.WriteLine("LogicalSectorSize:\t{0}", virtualHardDiskSettingData.LogicalSectorSize);
                Console.WriteLine("PhysicalSectorSize:\t{0}", virtualHardDiskSettingData.PhysicalSectorSize);

                if (virtualHardDiskState.MinInternalSize != null)
                {
                    Console.WriteLine("MinInternalSize:\t{0}", virtualHardDiskState.MinInternalSize);
                }
            }
        }
    }
}
