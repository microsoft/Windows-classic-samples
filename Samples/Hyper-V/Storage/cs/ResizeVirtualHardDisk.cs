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

    static class StorageResizeSample
    {
        /// <summary>
        /// Resizes a VHD/VHDX.  Shrink operation is only supported on VHDX.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="VirtualHardDiskPath">The path to the VHD/VHDX to resize.</param>
        /// <param name="FileSize">The new maximum size of the VHD/VHDX.</param>
        internal static void
        ResizeVirtualHardDisk(
            string ServerName,
            string VirtualHardDiskPath,
            UInt64 FileSize)
        {
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("ResizeVirtualHardDisk"))
                {
                    inParams["Path"] = VirtualHardDiskPath;
                    inParams["MaxInternalSize"] = FileSize;

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "ResizeVirtualHardDisk", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }
    }
}
