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
    using System.Reflection;
 
    class Program
    {
        /// <summary>
        /// Entry point of the program.
        /// </summary>
        /// <param name="args">Command line arguments.</param>
        static void 
        Main(
            string[] args)
        {
            if (args.Length == 3)
            {
                if (string.Equals(args[0], "GetVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    
                    StorageGetSample.GetVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath);
                }
                else if (string.Equals(args[0], "ValidateVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    
                    StorageValidateSample.ValidateVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath);
                }
                else if (string.Equals(args[0], "DetachVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    
                    StorageDetachSample.DetachVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath);
                }
                else if (string.Equals(args[0], "CreateVirtualFloppyDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    
                    StorageFloppySample.CreateVirtualFloppyDisk(
                        serverName,
                        virtualHardDiskPath);
                }
                else
                {
                    ShowUsage();
                }
            }
            else if (args.Length == 4)
            {
                if (string.Equals(args[0], "CompactVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    string mode = args[3];
                    
                    if (string.Equals(mode, "2", StringComparison.OrdinalIgnoreCase))
                    {
                        StorageAttachSample.AttachVirtualHardDisk(
                            serverName,
                            virtualHardDiskPath,
                            "false",
                            "true");
                    }
                                        
                    StorageCompactSample.CompactVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath,
                        mode);

                    if (string.Equals(mode, "2", StringComparison.OrdinalIgnoreCase))
                    {
                        StorageDetachSample.DetachVirtualHardDisk(
                            serverName,
                            virtualHardDiskPath);
                    }
                }
                else if (string.Equals(args[0], "MergeVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string sourcePath = args[2];
                    string destinationPath = args[3];
                    
                    StorageMergeSample.MergeVirtualHardDisk(
                        serverName,
                        sourcePath,
                        destinationPath);
                }
                else if (string.Equals(args[0], "ResizeVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    
                    UInt64 fileSize = UInt64.Parse(args[3], CultureInfo.CurrentCulture);
                    
                    StorageResizeSample.ResizeVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath,
                        fileSize);
                }
                else if (string.Equals(args[0], "SetVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    Int32 sectorSize;
                    string parentPath = args[3];

                    if (Int32.TryParse(args[3], NumberStyles.None, CultureInfo.CurrentCulture, out sectorSize))
                    {
                        StorageSetSample.SetVirtualHardDisk(
                            serverName,
                            virtualHardDiskPath,
                            null,
                            sectorSize);
                    }
                    else
                    {
                        StorageSetSample.SetVirtualHardDisk(
                            serverName,
                            virtualHardDiskPath,
                            parentPath,
                            0);
                    }
                }
                else
                {
                    ShowUsage();
                }
            }
            else if (args.Length == 5)
            {
                if (string.Equals(args[0], "AttachVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    string assignDriveLetter = args[3];
                    string readOnly = args[4];
                    
                    StorageAttachSample.AttachVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath,
                        assignDriveLetter,
                        readOnly);
                }
                else if (string.Equals(args[0], "CreateDifferencingVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    string parentPath = args[3];
                    VirtualHardDiskType type = VirtualHardDiskType.Differencing;
                    VirtualHardDiskFormat format;

                    if (string.Equals(args[4], "vhdx", StringComparison.OrdinalIgnoreCase))
                    {
                        format = VirtualHardDiskFormat.Vhdx;
                    }
                    else
                    {
                        format = VirtualHardDiskFormat.Vhd;
                    }

                    StorageCreateSample.CreateVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath,
                        parentPath,
                        type,
                        format,
                        0,
                        0,
                        0,
                        0);
                }
                else if (string.Equals(args[0], "ConvertVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string sourcePath = args[2];
                    string destinationPath = args[3];

                    VirtualHardDiskFormat format;
                    if (string.Equals(args[4], "vhdx", StringComparison.OrdinalIgnoreCase))
                    {
                        format = VirtualHardDiskFormat.Vhdx;
                    }
                    else
                    {
                        format = VirtualHardDiskFormat.Vhd;
                    }

                    StorageConvertSample.ConvertVirtualHardDisk(
                        serverName,
                        sourcePath,
                        destinationPath,
                        format);
                }
                else
                {
                    ShowUsage();
                }
            }
            else if (args.Length == 6)
            {
                if (string.Equals(args[0], "SetParentVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string ChildPath = args[2];
                    string ParentPath = args[3];
                    string LeafPath = args[4];
                    string IgnoreIDMismatch = args[5];

                    if (string.Equals(LeafPath, "null", StringComparison.OrdinalIgnoreCase))
                    {
                        // Only valid if VHD is not online.
                        LeafPath = null;
                    }
                    
                    StorageSetParentSample.SetParentVirtualHardDisk(
                        serverName,
                        ChildPath,
                        ParentPath,
                        LeafPath,
                        IgnoreIDMismatch);
                }
                else
                {
                    ShowUsage();
                }
            }            
            else if (args.Length == 8)
            {
                if (string.Equals(args[0], "CreateFixedVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    string parentPath = null;
                    VirtualHardDiskType type = VirtualHardDiskType.FixedSize;
                    VirtualHardDiskFormat format;

                    if (string.Equals(args[3], "vhdx", StringComparison.OrdinalIgnoreCase))
                    {
                        format = VirtualHardDiskFormat.Vhdx;
                    }
                    else
                    {
                        format = VirtualHardDiskFormat.Vhd;
                    }

                    Int64 fileSize = Int64.Parse(args[4], CultureInfo.CurrentCulture);
                    Int32 blockSize = Int32.Parse(args[5], CultureInfo.CurrentCulture);
                    Int32 logicalSectorSize = Int32.Parse(args[6], CultureInfo.CurrentCulture);
                    Int32 physicalSectorSize = Int32.Parse(args[7], CultureInfo.CurrentCulture);

                    StorageCreateSample.CreateVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath,
                        parentPath,
                        type,
                        format,
                        fileSize,
                        blockSize,
                        logicalSectorSize,
                        physicalSectorSize);
                }                
                else if (string.Equals(args[0], "CreateDynamicVirtualHardDisk", StringComparison.OrdinalIgnoreCase))
                {
                    string serverName = args[1];
                    string virtualHardDiskPath = args[2];
                    string parentPath = null;
                    VirtualHardDiskType type = VirtualHardDiskType.DynamicallyExpanding;
                    VirtualHardDiskFormat format;

                    if (string.Equals(args[3], "vhdx", StringComparison.OrdinalIgnoreCase))
                    {
                        format = VirtualHardDiskFormat.Vhdx;
                    }
                    else
                    {
                        format = VirtualHardDiskFormat.Vhd;
                    }

                    Int64 fileSize = Int64.Parse(args[4], CultureInfo.CurrentCulture);
                    Int32 blockSize = Int32.Parse(args[5], CultureInfo.CurrentCulture);
                    Int32 logicalSectorSize = Int32.Parse(args[6], CultureInfo.CurrentCulture);
                    Int32 physicalSectorSize = Int32.Parse(args[7], CultureInfo.CurrentCulture);

                    StorageCreateSample.CreateVirtualHardDisk(
                        serverName,
                        virtualHardDiskPath,
                        parentPath,
                        type,
                        format,
                        fileSize,
                        blockSize,
                        logicalSectorSize,
                        physicalSectorSize);
                }               
                else
                {
                    ShowUsage();
                }
            }
            else
            {
                ShowUsage();
            }
        }

        /// <summary>
        /// Displays the command line usage for the program.
        /// </summary>
        static void
        ShowUsage()
        {
            string moduleName = Assembly.GetExecutingAssembly().GetModules()[0].Name;

            Console.WriteLine("\nUsage:\t{0} <SampleName> <Arguments>\n", moduleName);

            Console.WriteLine("Supported SampleNames and Arguments:\n");
            Console.WriteLine("   GetVirtualHardDisk <server> <path>");
            Console.WriteLine("   SetVirtualHardDisk <server> <path> [<parent> | <physical sector size>]");
            Console.WriteLine("   ValidateVirtualHardDisk <server> <path>");
            Console.WriteLine("   CreateFixedVirtualHardDisk <server> <path> <file size> <block size> <logical sector size> <physical sector size>");
            Console.WriteLine("   CreateDynamicVirtualHardDisk <server> <path> <file size> <block size> <logical sector size> <physical sector size>");
            Console.WriteLine("   CreateDifferencingVirtualHardDisk <server> <path> <parent path>");
            Console.WriteLine("   CreateVirtualFloppyDisk <server> <path>");
            Console.WriteLine("   AttachVirtualHardDisk <server> <path> <assign drive letter> <read only>");
            Console.WriteLine("   DetachVirtualHardDisk <server> <path>");
            Console.WriteLine("   SetParentVirtualHardDisk <child> <parent> <leaf> <ignore id mismatch>");
            Console.WriteLine("   ConvertVirtualHardDisk <server> <source path> <destination path> <format>");
            Console.WriteLine("   MergeVirtualHardDisk <server> <source path> <destination path>");
            Console.WriteLine("   CompactVirtualHardDisk <server> <path> <mode>");
            Console.WriteLine("   ResizeVirtualHardDisk <server> <path> <file size>");
            Console.WriteLine("\n");
 
            Console.WriteLine("Examples:\n");
            Console.WriteLine("   {0} GetVirtualHardDisk . c:\\fixed.vhd", moduleName);
            Console.WriteLine("   {0} SetVirtualHardDisk . c:\\diff.vhdx c:\\dynamic.vhdx", moduleName);
            Console.WriteLine("   {0} SetVirtualHardDisk . c:\\diff.vhdx 512", moduleName);
            Console.WriteLine("   {0} ValidateVirtualHardDisk . c:\\fixed.vhd", moduleName);
            Console.WriteLine("   {0} CreateFixedVirtualHardDisk . c:\\fixed.vhd vhd 1073741824 0 0 0", moduleName);
            Console.WriteLine("   {0} CreateDynamicVirtualHardDisk . c:\\dynamic.vhd vhd 1073741824 0 0 0", moduleName);
            Console.WriteLine("   {0} CreateDifferencingVirtualHardDisk . c:\\diff.vhd c:\\dynamic.vhd vhd", moduleName);
            Console.WriteLine("   {0} CreateVirtualFloppyDisk . c:\\floppy.vfd", moduleName);
            Console.WriteLine("   {0} AttachVirtualHardDisk . c:\\fixed.vhd true true", moduleName);
            Console.WriteLine("   {0} DetachVirtualHardDisk . c:\\fixed.vhd", moduleName);
            Console.WriteLine("   {0} SetParentVirtualHardDisk . c:\\diff.vhd c:\\fixed.vhd null false", moduleName);
            Console.WriteLine("   {0} ConvertVirtualHardDisk . c:\\dynamic.vhd c:\\fixed.vhd vhd", moduleName);
            Console.WriteLine("   {0} MergeVirtualHardDisk . c:\\diff.vhd c:\\dynamic.vhd", moduleName);
            Console.WriteLine("   {0} CompactVirtualHardDisk . c:\\dynamic.vhd 0", moduleName);
            Console.WriteLine("   {0} ResizeVirtualHardDisk . c:\\dynamic.vhd 2147483648", moduleName);
            Console.WriteLine("\n");
            Console.WriteLine("\n");
        }
    }
}
