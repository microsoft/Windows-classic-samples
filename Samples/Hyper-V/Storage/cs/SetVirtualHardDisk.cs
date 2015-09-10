//
//THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
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

    static class StorageSetSample
    {
        /// <summary>
        /// Sets various settings on a VHD/VHDX.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="VirtualHardDiskPath">The path to the VHD/VHDX.</param>
        /// <param name="ParentPath">The new path to the parent VHD/VHDX.</param>
        /// <param name="PhysicalSectorSize">The new physical sector size of the VHDX.  Setting the
        /// physical sector size is only supported on VHDX.</param>
        internal static void
        SetVirtualHardDisk(
            string ServerName,
            string VirtualHardDiskPath,
            string ParentPath,
            Int32 PhysicalSectorSize)
        {
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            VirtualHardDiskSettingData settingData =
                new VirtualHardDiskSettingData(
                    VirtualHardDiskType.Unknown,
                    VirtualHardDiskFormat.Unknown,
                    VirtualHardDiskPath,
                    ParentPath,
                    0,
                    0,
                    0,
                    PhysicalSectorSize);

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("SetVirtualHardDiskSettingData"))
                {                    
                    inParams["VirtualDiskSettingData"] =
                        settingData.GetVirtualHardDiskSettingDataEmbeddedInstance(
                            ServerName,
                            imageManagementService.Path.Path);

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "SetVirtualHardDiskSettingData", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }
    }
}
