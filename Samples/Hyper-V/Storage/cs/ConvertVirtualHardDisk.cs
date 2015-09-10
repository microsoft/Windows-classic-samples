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

    static class StorageConvertSample
    {
        /// <summary>
        /// Converts a VHD or VHDX disk to fixed.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="SourcePath">The path to the VHD/VHDX to convert.</param>
        /// <param name="DestinationPath">The path of the new VHD/VHDX.</param>
        /// <param name="Format">The format of the new VHD.</param>
        internal static void
        ConvertVirtualHardDisk(
            string ServerName,
            string SourcePath,
            string DestinationPath,
            VirtualHardDiskFormat Format)
        {
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            VirtualHardDiskSettingData settingData =
                new VirtualHardDiskSettingData(
                    VirtualHardDiskType.FixedSize,
                    Format,
                    DestinationPath,
                    null,
                    0,
                    0,
                    0,
                    0);

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("ConvertVirtualHardDisk"))
                {
                    inParams["SourcePath"] = SourcePath;

                    inParams["VirtualDiskSettingData"] =
                        settingData.GetVirtualHardDiskSettingDataEmbeddedInstance(
                            ServerName,
                            imageManagementService.Path.Path);

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "ConvertVirtualHardDisk", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }
    }
}
