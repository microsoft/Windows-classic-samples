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

    static class StorageAttachSample
    {
        /// <summary>
        /// Attaches a VHD/VHDX.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="VirtualHardDiskPath">The path to the VHD/VHDX/ISO to attach.</param>
        /// <param name="AssignDriveLetters">Indicates if drive letters should be assigned to the
        /// mounted disk's volumes.</param>
        /// <param name="ReadOnly">Indicates if the mounted volumes are to be read-only.  Must be
        /// true when mounting ISO files.</param>
        internal static void
        AttachVirtualHardDisk(
            string ServerName,
            string VirtualHardDiskPath,
            string AssignDriveLetters,
            string ReadOnly)
        {

            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("AttachVirtualHardDisk"))
                {
                    inParams["Path"] = VirtualHardDiskPath;
                    inParams["AssignDriveLetter"] = AssignDriveLetters;
                    inParams["ReadOnly"] = ReadOnly;

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "AttachVirtualHardDisk", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }
    }
}
