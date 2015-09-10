// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.ResourcePools
{
    using System;
    using System.Collections;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;

    /// <summary>
    /// Resource pool related utilities.
    /// 
    /// For convenience, we define a "display name" for each supported resource,
    /// to simplify command line arguments.
    /// </summary>
    static class ResourceUtilities
    {
        static internal string[][] ResourceTypeInformation = new string[][]
        {
                        // Display Name         Resource Type  Resource Subtype

            new string[] { @"RDV",                @"1",          @"Microsoft:Hyper-V:Rdv Component" },
            new string[] { @"Processor",          @"3",          @"Microsoft:Hyper-V:Processor" },
            new string[] { @"Memory",             @"4",          @"Microsoft:Hyper-V:Memory" },
            new string[] { @"ScsiHBA",            @"6",          @"Microsoft:Hyper-V:Synthetic SCSI Controller" },
            new string[] { @"FCPort",             @"7",          @"Microsoft:Hyper-V:Synthetic FibreChannel Port" },
            new string[] { @"EmulatedEthernet",   @"10",         @"Microsoft:Hyper-V:Emulated Ethernet Port" },
            new string[] { @"SyntheticEthernet",  @"10",         @"Microsoft:Hyper-V:Synthetic Ethernet Port" },
            new string[] { @"Mouse",              @"13",         @"Microsoft:Hyper-V:Synthetic Mouse" },
            new string[] { @"SyntheticDVD",       @"16",         @"Microsoft:Hyper-V:Synthetic DVD Drive" },
            new string[] { @"PhysicalDisk",       @"17",         @"Microsoft:Hyper-V:Physical Disk Drive" },
            new string[] { @"SyntheticDisk",      @"17",         @"Microsoft:Hyper-V:Synthetic Disk Drive" },
            new string[] { @"CD/DVD",             @"31",         @"Microsoft:Hyper-V:Virtual CD/DVD Disk" },
            new string[] { @"3DGraphics",         @"24",         @"Microsoft:Hyper-V:Synthetic 3D Display Controller" },
            new string[] { @"Graphics",           @"24",         @"Microsoft:Hyper-V:Synthetic Display Controller" },
            new string[] { @"VHD",                @"31",         @"Microsoft:Hyper-V:Virtual Hard Disk" },
            new string[] { @"Floppy",             @"31",         @"Microsoft:Hyper-V:Virtual Floppy Disk" },
            new string[] { @"EthernetConnection", @"33",         @"Microsoft:Hyper-V:Ethernet Connection" },
            new string[] { @"FCConnection",       @"64764",      @"Microsoft:Hyper-V:FibreChannel Connection" }
        };

        /// <summary>
        /// Display information for the specified resource.
        /// </summary>
        /// <param name="resource">Array of strings containing the resource information.</param>
        static internal void
        DisplayResourceInformation(
            string[] resource
            )
        {
            Console.WriteLine(
                "\tResource:\n\t\tDisplay name: {0}\n\t\tType: {1}\n\t\tSubtype: {2}",
                resource[0],
                resource[1],
                resource[2]);
        }

        /// <summary>
        /// Display information for the specified resource.
        /// </summary>
        /// <param name="displayName">A string containing the display name of the resource.</param>
        static internal void
        DisplayResourceInformation(
            string displayName
            )
        {
            foreach (string[] resource in ResourceTypeInformation)
            {
                if (string.Equals(displayName, resource[0], StringComparison.CurrentCultureIgnoreCase))
                {
                    DisplayResourceInformation(resource);

                    return;
                }
            }

            Console.WriteLine("The specified resource is not supported.");
        }

        /// <summary>
        /// Enumerates the resource types supported by Hyper-V.
        /// </summary>
        static internal void
        EnumerateSupportedResources()
        {
            foreach (string[] resource in ResourceTypeInformation)
            {
                DisplayResourceInformation(resource);
            }
        }

        /// <summary>
        /// Returns the resource type for the specified resource.
        /// </summary>
        /// <param name="displayName">A string containing the display name of the resource.</param>
        /// <returns>A string containing the resource type.</returns>
        static internal string
        GetResourceType(
            string displayName
            )
        {
            foreach (string[] resource in ResourceTypeInformation)
            {
                if (string.Equals(displayName, resource[0], StringComparison.CurrentCultureIgnoreCase))
                {
                    return resource[1];
                }
            }

            throw new ManagementException("Invalid resource (" + displayName + ") specified.");
        }

        /// <summary>
        /// Returns the resource subtype for the specified resource.
        /// </summary>
        /// <param name="displayName">A string containing the display name of the resource.</param>
        /// <returns>A string containing the resource subtype.</returns>
        static internal string
        GetResourceSubType(
            string displayName
            )
        {
            foreach (string[] resource in ResourceTypeInformation)
            {
                if (string.Equals(displayName, resource[0], StringComparison.CurrentCultureIgnoreCase))
                {
                    return resource[2];
                }
            }

            throw new ManagementException("Invalid resource (" + displayName + ") specified.");
        }
    }
}
