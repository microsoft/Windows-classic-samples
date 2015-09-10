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

    static class StorageDetachSample
    {
        /// <summary>
        /// Detaches a VHD/VHDX that is currently mounted.
        /// </summary>
        /// <param name="ServerName">The name of the server on which to perform the action.</param>
        /// <param name="VirtualHardDiskPath">The path to the VHD/VHDX to detach.</param>
        internal static void
        DetachVirtualHardDisk(
            string ServerName,
            string VirtualHardDiskPath)
        {
            ManagementScope scope =
                new ManagementScope("\\\\" + ServerName + "\\root\\virtualization\\v2");

            ManagementClass mountedStorageImageServiceClass =
                new ManagementClass("Msvm_MountedStorageImage");

            mountedStorageImageServiceClass.Scope = scope;

            using (ManagementObjectCollection collection = mountedStorageImageServiceClass.GetInstances())
            {
                foreach (ManagementObject image in collection) 
                {
                    using (image)
                    {
                        string Name = image.GetPropertyValue("Name").ToString();
                        if (string.Equals(Name, VirtualHardDiskPath, StringComparison.OrdinalIgnoreCase))
                        {
                            image.InvokeMethod("DetachVirtualHardDisk", null, null);
                        }
                    }
                }
            }
            
        }
    }
}
