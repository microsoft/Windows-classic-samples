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

    static class StorageMergeSample
    {
        /// <summary>
        /// Merges a VHD/VHDX into a parent VHD/VHDX.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="ChildPath">The path to the VHD/VHDX to merge.</param>
        /// <param name="ParentPath">The path to the parent into which to merge.</param>
        internal static void
        MergeVirtualHardDisk(
            string ServerName,
            string ChildPath,
            string ParentPath)
        {
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("MergeVirtualHardDisk"))
                {
                    inParams["SourcePath"] = ChildPath;
                    inParams["DestinationPath"] = ParentPath;

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "MergeVirtualHardDisk", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }
    }
}
