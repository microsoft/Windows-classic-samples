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

    static class StorageCompactSample
    {
        /// <summary>
        /// Compacts a VHD or VHDX.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="VirtualHardDiskPath">The path to the VHD to compact.</param>
        /// <param name="Mode">The mode of the compact operation.</param>
        internal static void
        CompactVirtualHardDisk(
            string ServerName,
            string VirtualHardDiskPath,
            string Mode)
        {
            //
            // To obtain the full benefit of compaction, the VHD or VHDX should be mounted prior to
            // and throughout the compaction.
            //
            // Mounting a VHD is demonstrated in the AttachVirtualDisk sample.
            //
            // The VHD can only be mounted read-only during compaction.
            //

            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("CompactVirtualHardDisk"))
                {
                    inParams["Path"] = VirtualHardDiskPath;
                    inParams["Mode"] = Mode;

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "CompactVirtualHardDisk", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }  
        }
    }
}
