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

    static class StorageSetParentSample
    {
        /// <summary>
        /// Updates the parent for a VHD/VHDX.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="ChildPath">The path to the child VHD/VHDX.</param>
        /// <param name="ParentPath">The path to the new parent VHD/VHDX.</param>
        /// <param name="LeafPath">The path to the leaf VHD/VHDX.</param>
        /// <param name="IgnoreIDMismatch">Indicates whether the parent should be forcibly set when
        /// the virtual disk IDs do not match.</param>
        internal static void
        SetParentVirtualHardDisk(
            string ServerName,
            string ChildPath,
            string ParentPath,
            string LeafPath,
            string IgnoreIDMismatch)
        {
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("SetParentVirtualHardDisk"))
                {
                    inParams["ChildPath"] = ChildPath;
                    inParams["ParentPath"] = ParentPath;
                    inParams["LeafPath"] = LeafPath;
                    inParams["IgnoreIDMismatch"] = IgnoreIDMismatch;
                    
                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "SetParentVirtualHardDisk", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }
        }
    }
}
