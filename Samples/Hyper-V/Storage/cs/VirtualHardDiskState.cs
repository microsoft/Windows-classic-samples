// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

namespace Microsoft.Samples.HyperV.Storage
{
    using System;
    using System.Xml;
    using System.Globalization;
    using System.Management;
    using Microsoft.Samples.HyperV.Common;
    
    /// <summary>
    /// Contains information about a virtual hard disk file.
    /// </summary>
    /// <remarks>
    /// This class could have also been reasonably designed as a struct. We decided against this
    /// approach, because client code had already been written assuming class semantics.
    /// </remarks>
    public class VirtualHardDiskState
    {
        private Int64 m_FileSize;
        private bool m_InUse;
        private Int64? m_MinInternalSize;
        private Int32 m_PhysicalSectorSize;
        private Int32 m_Alignment;
        private Int32? m_FragmentationPercentage;

        /// <summary>
        /// Initializes a new <see cref="VirtualHardDiskState"/> class.
        /// </summary>
        /// <param name="fileSize">The virtual disk size.</param>
        /// <param name="inUse">The virtual disk in use flag.</param>
        /// <param name="minInternalSize">The minimum internal size of the virtual disk.</param>
        /// <param name="physicalSectorSize">The physical sector size of the underlying physical
        /// hard disk.</param>
        /// <param name="alignment">The alignment within the virtual hard disk.</param>
        public
        VirtualHardDiskState(
            Int64 fileSize,
            bool inUse,
            Int64? minInternalSize,
            Int32 physicalSectorSize,
            Int32 alignment,
            Int32? fragmentationPercentage)
        {
            m_FileSize = fileSize;
            m_InUse = inUse;
            m_MinInternalSize = minInternalSize;
            m_PhysicalSectorSize = physicalSectorSize;
            m_Alignment = alignment;
            m_FragmentationPercentage = fragmentationPercentage;
        }

        /// <summary>
        /// Gets the size of this disk. 
        /// </summary>
        public long
        FileSize
        {
            get { return m_FileSize; }
        }

        /// <summary>
        /// Gets whether the virtual hard disk is in use.
        /// </summary>
        public bool
        InUse
        {
            get { return m_InUse;}
        }

        /// <summary>
        /// Gets the minimum internal size of this disk. 
        /// </summary>
        public long?
        MinInternalSize
        {
            get { return m_MinInternalSize; }
        }

        /// <summary>
        /// Gets the disk's physical sector size.
        /// </summary>
        public long
        PhysicalSectorSize
        {
            get { return m_PhysicalSectorSize; }
        }

        /// <summary>
        /// Gets the disk's aligment.
        /// </summary>
        public long
        Alignment
        {
            get { return m_Alignment; }
        }

        /// <summary>
        /// Gets the disk's fragmentation percentage.
        /// </summary>
        public long?
        FragmentationPercentage
        {
            get { return m_FragmentationPercentage; }
        }

        /// <summary>
        /// Parses the hard disk State embedded instance returned from the server and
        /// creates a new VirtualHardDiskState with that information.
        /// </summary>
        /// <param name="embeddedInstance">The disk State embedded instance.</param>
        /// <returns>A <see cref="VirtualHardDiskState"/> object with the data contained in the
        /// embedded instance.</returns>
        /// <exception cref="ArgumentNullException">If either param is null.</exception>
        /// <exception cref="FormatException">If there was a problem parsing the embedded instance
        /// </exception>
        public static VirtualHardDiskState
        Parse(
            string embeddedInstance)
        {
            if (embeddedInstance == null)
            {
                throw new ArgumentNullException("embeddedInstance");
            }

            try
            {
                Int64 fileSize = 0;
                bool inUse = false;
                Int64? minInternalSize = 0;
                Int32 physicalSectorSize = 0;
                Int32 alignment = 0;
                Int32? fragmentationPercentage = 0;

                XmlDocument doc = new XmlDocument();
                doc.LoadXml(embeddedInstance);

                XmlNodeList nodelist = doc.SelectNodes(@"/INSTANCE/@CLASSNAME");
                if (nodelist.Count != 1)
                {
                    throw new FormatException();
                }

                if (nodelist[0].Value != "Msvm_VirtualHardDiskState")
                {
                    throw new FormatException();
                }

                // FileSize
                nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'FileSize']/VALUE/child::text()");
                if (nodelist.Count != 1)
                {
                    throw new FormatException();
                }

                fileSize = Int64.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);

                // InUse should be false
                nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'InUse']/VALUE/child::text()");
                if (nodelist.Count != 1)
                {
                    throw new FormatException();
                }

                inUse = bool.Parse(nodelist[0].Value);

                // MinInternalSize
                nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'MinInternalSize']/VALUE/child::text()");
                if (nodelist.Count == 0)
                {
                    minInternalSize = null;
                }
                else if (nodelist.Count == 1)
                {
                    minInternalSize = Int64.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);
                }
                else
                {
                    throw new FormatException();
                }

                // PhysicalSectorSize
                nodelist = 
                    doc.SelectNodes(@"//PROPERTY[@NAME = 'PhysicalSectorSize']/VALUE/child::text()");
                if (nodelist.Count != 1)
                {
                    throw new FormatException();
                }

                physicalSectorSize = Int32.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);

                // Alignment
                nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'Alignment']/VALUE/child::text()");
                if (nodelist.Count != 1)
                {
                    throw new FormatException();
                }

                alignment = Int32.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);

                // FragmentationPercentage
                nodelist = doc.SelectNodes(@"//PROPERTY[@NAME = 'FragmentationPercentage']/VALUE/child::text()");
                if (nodelist.Count == 0)
                {
                    fragmentationPercentage = null;
                }
                else if (nodelist.Count == 1)
                {
                    fragmentationPercentage = Int32.Parse(nodelist[0].Value, CultureInfo.InvariantCulture);
                }
                else
                {
                    throw new FormatException();
                }

                return new VirtualHardDiskState(fileSize, inUse, minInternalSize,
                                physicalSectorSize, alignment, fragmentationPercentage);

            }
            catch (XmlException ex)
            {
                // There was an error parsing the embeddedInstance as XML.
                // Throw this as a FormatException- the format of the returned
                // embeddedInstance is not as expected. 

                throw new FormatException(null, ex);
            }
            catch (OverflowException ex)
            {
                // Treat any overflows as a format exception so that we return a consistent error
                // from this method.
                throw new FormatException(null, ex);
            }
        }
    }
}
