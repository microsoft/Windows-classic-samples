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
    
    static class StorageUtilities
    {
        /// <summary>
        /// Gets the image management service.
        /// </summary>
        /// <param name="scope">The ManagementScope to use to connect to WMI.</param>
        /// <returns>The image management object.</returns>
        internal static ManagementObject
        GetImageManagementService(
            ManagementScope scope)
        {
            using(ManagementClass imageManagementServiceClass = new ManagementClass("Msvm_ImageManagementService"))
            {
                imageManagementServiceClass.Scope = scope;

                ManagementObject imageManagementService = WmiUtilities.GetFirstObjectFromCollection(
                    imageManagementServiceClass.GetInstances());

                return imageManagementService;
            }
        }
    }
}
