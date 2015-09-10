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

    static class StorageFloppySample
    {
        /// <summary>
        /// Creates a Virtual Floppy Disk.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="VirtualHardDiskPath">The path to the VFD to create.</param>
        internal static void
        CreateVirtualFloppyDisk(
            string ServerName,
            string VirtualFloppyDiskPath)
        {
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            using (ManagementObject imageManagementService =
                StorageUtilities.GetImageManagementService(scope))
            {
                using (ManagementBaseObject inParams =
                    imageManagementService.GetMethodParameters("CreateVirtualFloppyDisk"))
                {
                    inParams["Path"] = VirtualFloppyDiskPath;

                    using (ManagementBaseObject outParams = imageManagementService.InvokeMethod(
                        "CreateVirtualFloppyDisk", inParams, null))
                    {
                        WmiUtilities.ValidateOutput(outParams, scope);
                    }
                }
            }            
        }
    }
}
