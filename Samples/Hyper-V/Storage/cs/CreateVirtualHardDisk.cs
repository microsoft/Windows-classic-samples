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

    static class StorageCreateSample
    {
        /// <summary>
        /// Creates a VHD or VHDX.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="VirtualHardDiskPath">The path to the VHD/VHDX to create.</param>
        /// <param name="ParentPath">The path to the parent VHD/VHDX.</param>
        /// <param name="Type">The type for the new VHD/VHDX.</param>
        /// <param name="Format">The format of the new VHD/VHDX.</param>
        /// <param name="FileSize">The size of the new VHD/VHDX.</param>
        /// <param name="BlockSize">The block size of the new VHD/VHDX.</param>
        /// <param name="LogicalSectorSize">The logical sector size of the new VHD/VHDX.</param>
        /// <param name="PhysicalSectorSize">The physical sector size of the new VHD/VHDX.</param>
        internal static void
        CreateVirtualHardDisk(
            string ServerName,
            string VirtualHardDiskPath,
            string ParentPath,
            VirtualHardDiskType Type,
            VirtualHardDiskFormat Format,
            Int64 FileSize,
            Int32 BlockSize,
            Int32 LogicalSectorSize,
            Int32 PhysicalSectorSize)
        {
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            VirtualHardDiskSettingData settingData =
                new VirtualHardDiskSettingData(
                    Type,
                    Format,
                    VirtualHardDiskPath,
                    ParentPath,
                    FileSize,
                    BlockSize,
                    LogicalSectorSize,
                    PhysicalSectorSize);

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("CreateVirtualHardDisk"))
                {
                    inParams["VirtualDiskSettingData"] =
                        settingData.GetVirtualHardDiskSettingDataEmbeddedInstance(
                            ServerName,
                            imageManagementService.Path.Path);

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "CreateVirtualHardDisk", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }
    }
}
