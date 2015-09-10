// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft. All rights reserved.

namespace Microsoft.Samples.HyperV.Storage
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Xml;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;
    
    /// <summary>
    /// The different types of virtual hard disks.
    /// </summary>
    public enum VirtualHardDiskType
    {
        /// <summary>
        /// Unknown format.
        /// </summary>
        Unknown = 0,
        /// <summary>
        /// Fixed size virtual hard disk type.
        /// </summary>
        FixedSize = 2,
        /// <summary>
        /// Dynamically expanding virtual hard disk type.
        /// </summary>
        DynamicallyExpanding = 3,
        /// <summary>
        /// Differencing virtual hard disk type.
        /// </summary>
        Differencing = 4
    }

    /// <summary>
    /// The different formats of virtual hard disks.
    /// </summary>
    public enum VirtualHardDiskFormat
    {
        /// <summary>
        /// Unknown format.
        /// </summary>
        Unknown = 0,
        /// <summary>
        /// VHD format.
        /// </summary>
        Vhd = 2,
        /// <summary>
        /// VHDX format.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Vhdx")]
        Vhdx = 3
    }
    
    /// <summary>
    /// Contains information about a virtual hard disk file.
    /// </summary>
    public class VirtualHardDiskSettingData
    {
        private VirtualHardDiskType m_DiskType;
        private VirtualHardDiskFormat m_DiskFormat;
        private string m_Path;
        private string m_ParentPath;
        private Int64 m_MaxInternalSize;
        private Int32 m_BlockSize;
        private Int32 m_LogicalSectorSize;
        private Int32 m_PhysicalSectorSize;

        /// <summary>
        /// Initializes a new <see cref="VirtualHardDiskSettingData"/> class.
        /// </summary>
        /// <param name="diskType">The type of the disk.</param>
        /// <param name="diskFormat">The format of the disk.</param>
        /// <param name="path">The disk's path.</param>
        /// <param name="parentPath">The path of the disk's parent. This parameter should be null if
        /// the disk does not have a parent.</param>
        /// <param name="maxInternalSize">The maximum disk size as viewable by the virtual machine.
        /// </param>
        /// <param name="blockSize">The block size of the virtual hard disk.</param>
        /// <param name="logicalSectorSize">The logical sector size of the virtual hard disk.
        /// <param name="physicalSectorSize">The physical sector size of the virtual hard disk.
        /// </param>
        public
        VirtualHardDiskSettingData(
            VirtualHardDiskType diskType,
            VirtualHardDiskFormat diskFormat,
            string path,
            string parentPath,
            Int64 maxInternalSize,
            Int32 blockSize,
            Int32 logicalSectorSize,
            Int32 physicalSectorSize)
        {
            m_DiskType = diskType;
            m_DiskFormat = diskFormat;
            m_Path = path;
            m_ParentPath = parentPath;
            m_MaxInternalSize = maxInternalSize;
            m_BlockSize = blockSize;
            m_LogicalSectorSize = logicalSectorSize;
            m_PhysicalSectorSize = physicalSectorSize;
        }

        /// <summary>
        /// Gets the type of this disk. 
        /// </summary>
        public VirtualHardDiskType
        DiskType
        {
            get { return m_DiskType; }
        }

        /// <summary>
        /// Gets the format of this disk. 
        /// </summary>
        public VirtualHardDiskFormat
        DiskFormat
        {
            get { return m_DiskFormat; }
        }

        /// <summary>
        /// Gets the path of the disk.
        /// </summary>
        public string
        Path
        {
            get { return m_Path; }
        }

        /// <summary>
        /// Gets the parent of the disk. If the disk does not have a parent this property is null.
        /// </summary>
        public string
        ParentPath
        {
            get { return m_ParentPath; }
        }

        /// <summary>
        /// Gets the disk's maximum size as viewable by the virtual machine.
        /// </summary>
        public long
        MaxInternalSize
        {
            get { return m_MaxInternalSize; }
        }

        /// <summary>
        /// Gets the block size of the virtual hard disk
        /// </summary>
        public long
        BlockSize
        {
            get { return m_BlockSize; }
        }

        /// <summary>
        /// Gets the logical sector size of the virtual hard disk
        /// </summary>
        public long
        LogicalSectorSize
        {
            get { return m_LogicalSectorSize; }
        }

        /// <summary>
        /// Gets the physical sector size of the virtual hard disk
        /// </summary>
        public long
        PhysicalSectorSize
        {
            get { return m_PhysicalSectorSize; }
        }

        /// <summary>
        /// Gets the embedded instance string usable by WMI
        /// </summary>
        /// <returns>Embedded instance string usable by WMI.</returns>
        public string
        GetVirtualHardDiskSettingDataEmbeddedInstance(
            string serverName,
            string namespacePath)
        {
            ManagementPath path = new ManagementPath()
            {
                Server = serverName,
                NamespacePath = namespacePath,
                ClassName = "Msvm_VirtualHardDiskSettingData"
            };

            using (ManagementClass settingsClass = new ManagementClass(path))
            {
             using (ManagementObject settingsInstance = settingsClass.CreateInstance())
                {
                    settingsInstance["Type"] = m_DiskType;
                    settingsInstance["Format"] = m_DiskFormat;
                    settingsInstance["Path"] = m_Path;
                    settingsInstance["ParentPath"] = m_ParentPath;
                    settingsInstance["MaxInternalSize"] = m_MaxInternalSize;
                    settingsInstance["BlockSize"] = m_BlockSize;
                    settingsInstance["LogicalSectorSize"] = m_LogicalSectorSize;
                    settingsInstance["PhysicalSectorSize"] = m_PhysicalSectorSize;

                    string settingsInstanceString = settingsInstance.GetText(TextFormat.WmiDtd20);

                    return settingsInstanceString;
                }
            }
        }

        /// <summary>
        /// Parses the hard disk SettingData embedded instance returned from the server and
        /// creates a new VirtualHardDiskSettingData with that information.
        /// </summary>
        /// <param name="embeddedInstance">The disk SettingData embedded instance.</param>
        /// <returns>A <see cref="VirtualHardDiskSettingData"/> object with the data contained in
        /// the embedded instance.</returns>
        /// <exception cref="ArgumentNullException">If either param is null.</exception>
        /// <exception cref="FormatException">If there was a problem parsing the embedded instance.
        /// </exception>
        public static VirtualHardDiskSettingData
        Parse(
             string embeddedInstance)
        {
            VirtualHardDiskType type = VirtualHardDiskType.Unknown;
            VirtualHardDiskFormat format = VirtualHardDiskFormat.Unknown;
            string path = string.Empty;
            string parentPath = string.Empty;
            Int64 maxInternalSize = 0;
            Int32 blockSize = 0;
            Int32 logicalSectorSize = 0;
            Int32 physicalSectorSize = 0;

            XmlDocument doc = new XmlDocument();
            doc.LoadXml(embeddedInstance);

            XmlNodeList nodelist = doc.SelectNodes(@"/INSTANCE/@CLASSNAME");
            if (nodelist.Count != 1)
            {
                throw new FormatException();
            }

            if (nodelist[0].Value != "Msvm_VirtualHardDiskSettingData")
            {
                throw new FormatException();
            }

            // Disk type
            nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'Type']/VALUE/child::text()");
            if (nodelist.Count != 1)
            {
                throw new FormatException();
            }

            int itype =
                int.Parse(nodelist[0].Value, NumberStyles.None, CultureInfo.InvariantCulture);
            type = (VirtualHardDiskType)itype;

            if (type != VirtualHardDiskType.Differencing &&
                type != VirtualHardDiskType.DynamicallyExpanding &&
                type != VirtualHardDiskType.FixedSize)
            {
                // The type integer returned is of an unrecognized type.
                throw new FormatException();
            }

            // Disk format
            nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'Format']/VALUE/child::text()");
            if (nodelist.Count != 1)
            {
                throw new FormatException();
            }

            int iformat =
                int.Parse(nodelist[0].Value, NumberStyles.None, CultureInfo.InvariantCulture);
            format = (VirtualHardDiskFormat)iformat;

            if (format != VirtualHardDiskFormat.Vhd &&
                format != VirtualHardDiskFormat.Vhdx)
            {
                //The format integer returned is of an unrecognized type.
                throw new FormatException();
            }

            // Path
            nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'Path']/VALUE/child::text()");

            if (nodelist.Count != 1)
            {
                // There can not be multiple parents.
                throw new FormatException();
            }

            path = nodelist[0].Value;

            // ParentPath
            nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'ParentPath']/VALUE/child::text()");

            // A nodeList.Count == 0 is okay and indicates that there is no parent.

            if (nodelist.Count == 1)
            {
                parentPath = nodelist[0].Value;
            }
            else if (nodelist.Count != 0)
            {
                // There can not be multiple parents.
                throw new FormatException();
            }

            if (type == VirtualHardDiskType.Differencing && string.IsNullOrEmpty(parentPath))
            {
                // Parent path must be set if this is a differencing disk.
                throw new FormatException();
            }

            // MaxInternalSize
            nodelist =
                doc.SelectNodes(@"//PROPERTY[@NAME = 'MaxInternalSize']/VALUE/child::text()");
            if (nodelist.Count != 1)
            {
                throw new FormatException();
            }

            maxInternalSize = Int64.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);

            // BlockSize
            nodelist =
                doc.SelectNodes(@"//PROPERTY[@NAME = 'BlockSize']/VALUE/child::text()");
            if (nodelist.Count != 1)
            {
                throw new FormatException();
            }

            blockSize = Int32.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);

            // LogicalSectorSize
            nodelist =
                doc.SelectNodes(@"//PROPERTY[@NAME = 'LogicalSectorSize']/VALUE/child::text()");
            if (nodelist.Count != 1)
            {
                throw new FormatException();
            }

            logicalSectorSize = Int32.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);

            // PhysicalSectorSize
            nodelist =
                doc.SelectNodes(@"//PROPERTY[@NAME = 'PhysicalSectorSize']/VALUE/child::text()");
            if (nodelist.Count != 1)
            {
                throw new FormatException();
            }
            
            physicalSectorSize = Int32.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);

            return new VirtualHardDiskSettingData(type, format, path, parentPath,
                            maxInternalSize, blockSize, logicalSectorSize, physicalSectorSize);
        }
    }
}
