using System;
using System.Threading;
using System.Collections;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.ComTypes;
using System.Runtime.InteropServices.CustomMarshalers;

using System.Text.RegularExpressions;

namespace Storage.Interop.IMAPIv2
{
    public interface IDiscRecorder2X
    {
        /// <summary>Sends a MMC command to the recording device. Use this function when no data buffer 
        /// is sent to nor received from the device.</summary>
        /// <remarks>For details of the contents of the command packet and sense data, see the latest revision of the 
        /// MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// <para>Client-defined commands (CDBs) used with this method must be between 6 and 16 bytes in length. 
        /// In addition, the size of each command must match the size defined by the operation code as defined in the 
        /// following table.</para>
        /// <para><list type="table">
        ///    <listheader>
        ///        <term>CDB group</term>
        ///        <term>CDB operation code range</term>
        ///        <term>Required CDB size</term>
        ///    </listheader>
        ///    <item>
        ///        <description>0</description>
        ///        <description>0x00 — 0x1F</description>
        ///        <description>6 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>1</description>
        ///        <description>0x20 — 0x3F</description>
        ///        <description>10 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>2</description>
        ///        <description>0x40 — 0x5F</description>
        ///        <description>10 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>3</description>
        ///        <description>0x60 — 0x7F</description>
        ///        <description>TBD - will enforce standard-specified size requirements for this opcode range in the future.</description>
        ///    </item>
        ///    <item>
        ///        <description>4</description>
        ///        <description>0x80 — 0x9F</description>
        ///        <description>16 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>5</description>
        ///        <description>0xA0 — 0xBF</description>
        ///        <description>12 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>6</description>
        ///        <description>0xC0 — 0xDF</description>
        ///        <description>Vendor Unique - Any size allowed</description>
        ///    </item>
        ///    <item>
        ///        <description>7</description>
        ///        <description>0xE0 — 0xFF</description>
        ///        <description>Vendor Unique - Any size allowed</description>
        ///    </item>
        /// </list></para>
        /// <para>Some very early devices used vendor-unique opcodes and therefore some opcodes cannot be validated 
        /// in this manner. The following opcodes are still valid and only verify that the size is between 6 and 16 
        /// bytes:</para><para>0x02, 0x05, 0x06, 0x09, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x13, 0x14, 0x19, 0x20, 0x21, 0x22, 
        /// 0x23, 0x24, 0x26, 0x27, 0x29, 0x2C, 0x2D</para></remarks>
        /// <param name="cdb">
        /// Command packet to send to the device.
        /// Must be between 6 and 16 bytes.
        /// </param>
        /// <param name="Timeout">
        /// Time limit, in seconds, allowed for the send command to receive a result.
        /// </param>
        /// <returns>Sense data returned by the recording device.</returns>
        Byte[] SendCommandNoData(Byte[] cdb, uint timeout);

        /// <summary>
        /// Sends a MMC command and its associated data buffer to the recording device.
        /// </summary>
        /// <param name="cdb">
        /// Command packet to send to the device. Must be between 6 and 16 bytes.
        /// </param>
        /// <param name="buffer">
        /// Buffer containing data associated with the send command. Must not be NULL.
        /// </param>
        /// <param name="timeout">
        /// Time limit, in seconds, allowed for the send command to receive a result.
        /// </param>
        /// <returns>
        /// Sense data returned by the recording device.
        /// </returns>
        Byte[] SendCommandSendDataToDevice(Byte[] cdb, Byte[] buffer, uint timeout);

        /// <summary>
        /// Sends a MMC command to the recording device requesting data from the device.
        /// </summary>
        /// <param name="cdb">
        /// Command packet to send to the device. Must be between 6 and 16 bytes.
        /// </param>
        /// <param name="timeout">
        /// Time limit, in seconds, allowed for the send command to receive a result.
        /// </param>
        /// <param name="buffer">
        /// Application-allocated data buffer that will receive data associated 
        /// with the send command. Must not be NULL.
        /// </param>
        /// <returns>
        /// Sense data returned by the recording device.
        /// </returns>
        Byte[] SendCommandGetDataFromDevice(Byte[] cdb, Byte[] buffer, uint timeout);

        /// <summary>
        /// Read a DVD Structure from the media
        /// </summary>
        /// <param name="format">
        /// Format field of the command packet. Acceptable values range from zero to 0xFF.
        /// </param>
        /// <param name="address">
        /// Address field of the command packet.
        /// </param>
        /// <param name="layer">
        /// Layer field of the command packet.
        /// </param>
        /// <param name="agid">
        /// Authentication grant ID (AGID) field of the command packet.
        /// </param>
        /// <returns>
        /// Data buffer that contains the DVD structure. 
        /// For details of the contents of the data buffer, see the READ DISC STRUCTURE command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// This method removes headers from the buffer.
        /// </returns>
        Byte[] ReadDvdStructure(uint format, uint address, uint layer, uint agid);

        /// <summary>
        /// Sends a DVD structure to the media.
        /// </summary>
        /// <param name="format">
        /// Format field of the command packet. Acceptable values range from zero to 0xFF.
        /// </param>
        /// <param name="data">
        /// Data buffer that contains the DVD structure to send to the media. 
        /// Do not include a header; this method generates and prepends a header to the DVD structure.
        /// </param>
        void SendDvdStructure(Byte[] dvdStructure, uint format);

        /// <summary>
        /// Retrieves the adapter descriptor for the device (via IOCTL_STORAGE_QUERY_PROPERTY).
        /// </summary>
        /// <returns>
        /// Data buffer that contains the descriptor of the storage adapter. 
        /// For details of the contents of the data buffer, see the STORAGE_ADAPTER_DESCRIPTOR structure in the DDK.
        /// </returns>
        Byte[] GetAdapterDescriptor();

        /// <summary>
        /// Retrieves the device descriptor for the device (via IOCTL_STORAGE_QUERY_PROPERTY).
        /// </summary>
        /// <returns>
        /// Data buffer that contains the descriptor of the storage device. 
        /// For details of the contents of the data buffer, see the STORAGE_DEVICE_DESCRIPTOR structure in the DDK.
        /// </returns>
        Byte[] GetDeviceDescriptor();

        /// <summary>
        /// Retrieves the disc information from the media (via READ_DISC_INFORMATION).</summary>
        /// <returns>
        /// Data buffer that contains disc information from the media. 
        /// For details of the contents of the data buffer, see the READ DISC INFORMATION command in the latest 
        /// revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// </returns>
        Byte[] GetDiscInformation();

        /// <summary>
        /// Retrieves the track information from the media (via READ_TRACK_INFORMATION).
        /// </summary>
        /// <param name="address">
        /// Address field. The addressType parameter provides additional context for 
        /// this parameter.
        /// </param>
        /// <param name="addressType">
        /// Type of address specified in the address parameter, for example, 
        /// if this is an LBA address or a track number. For possible values, see the ReadTrackAddressType 
        /// enumeration type.
        /// </param>
        /// <returns>
        /// Data buffer that contains the track information. 
        /// For details of the contents of the data buffer, see the READ TRACK INFORMATION command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// </returns>
        Byte[] GetTrackInformation(uint address, ReadTrackAddressType addressType);

        /// <summary>
        /// Retrieves the specified feature page from the device (via GET_CONFIGURATION).
        /// </summary>
        /// <param name="requestedFeature">
        /// Feature page to retrieve. For possible values, see the 
        /// FeaturePageType enumeration type.
        /// </param>
        /// <param name="currentFeatureOnly">
        /// Set to True to retrieve the feature page only when it is the current 
        /// feature page. Otherwise, False to retrieve the feature page regardless of it being the current feature 
        /// page.
        /// </param>
        /// <returns>
        /// Data buffer that contains the feature page. For details of the contents of the 
        /// data buffer, see the GET CONFIGURATION command in the latest revision of the MMC specification at 
        /// ftp://ftp.t10.org/t10/drafts/mmc5/. 
        /// This method removes header information and other non-feature data before filling and sending this buffer. 
        /// </returns>
        Byte[] GetFeaturePage(FeaturePageType requestedFeature, bool currentFeatureOnly);

        /// <summary>
        /// Retrieves the specified mode page from the device (via MODE_SENSE10)
        /// </summary>
        /// <param name="requestedModePage">
        /// Mode page to retrieve. 
        /// For possible values, see the ModePageType enumeration type.
        /// </param>
        /// <param name="requestType">
        /// Type of mode page data to retrieve, for example, the current settings or 
        /// the settings that are write enabled. For possible values, see the ModePageRequestType 
        /// enumeration type.
        /// </param>
        /// <returns>
        /// Data buffer that contains the mode page. 
        /// For details of the contents of the data buffer, see the MODE SENSE (10) command in the latest revision 
        /// of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/. 
        /// This method removes header information and other non-page data before returning the buffer. 
        /// </returns>
        Byte[] GetModePage(ModePageType requestedModePage, ModePageRequestType requestType);

        /// <summary>
        /// Sets the mode page data for the device (via MODE_SELECT10).
        /// </summary>
        /// <param name="requestType">
        /// Type of mode page data to send. 
        /// For possible values, see the ModePageRequestType enumeration type.
        /// </param>
        /// <param name="modePage">
        /// Data buffer that contains the mode page data to send to the media. 
        /// Do not include a header; this method generates and prepends a header to the mode page data. 
        /// For details on specifying the fields of the mode page data, see the MODE SELECT (10) command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// </param>
        void SetModePage(ModePageRequestType requestType, Byte[] modePage);

        /// <summary>
        /// Retrieves the list of supported feature pages or the current feature pages of the device.
        /// </summary>
        /// <param name="currentFeatureOnly">
        /// Set to True to retrieve only current feature pages. 
        /// Otherwise, False to retrieve all feature pages that the device supports.
        /// </param>
        /// <returns>
        /// Data buffer that contains one or more feature page types. 
        /// For possible values, see the FeaturePageType enumeration type.  
        /// To get the feature page data associated with the feature page type, call the 
        /// IDiscRecorder2Ex.GetFeaturePage method. 
        /// </returns>
        Byte[] GetSupportedFeaturePages(bool currentFeatureOnly);

        /// <summary>
        /// Retrieves the supported profiles or the current profiles of the device.
        /// </summary>
        /// <param name="currentOnly">
        /// Set to True to retrieve the current profiles. 
        /// Otherwise, False to return all supported profiles of the device.
        /// </param>
        /// <returns>
        /// Data buffer that contains one or more profile types. 
        /// For possible values, see the ProfileType enumeration type.
        /// </returns>
        Byte[] GetSupportedProfiles(bool currentOnly);

        /// <summary>
        /// Retrieves the supported mode pages for the device.
        /// </summary>
        /// <param name="requestType">
        /// Type of mode page data to retrieve, for example, 
        /// the current settings or the settings that are write enabled. 
        /// For possible values, see the ModePageRequestType enumeration type.
        /// </param>
        /// <returns>
        /// Data buffer that contains one or more mode page types. 
        /// For possible values, see the ModePageType enumeration type. 
        /// To get the mode page data associated with the mode page type, call the IDiscRecorder2Ex.GetModePage 
        /// method.
        /// </returns>
        Byte[] GetSupportedModePages(ModePageRequestType requestType);

        /// <summary>
        /// Retrieves the byte alignment mask for the device.
        /// </summary>
        /// <returns>
        /// Byte alignment mask that you use to determine if the buffer is aligned to the correct byte 
        /// boundary for the device. The byte alignment value is always a number that is a power of 2.
        /// </returns>
        uint GetByteAlignmentMask();

        /// <summary>
        /// Retrieves the maximum non-page-aligned transfer size for the device.
        /// </summary>
        /// <returns>
        /// Maximum size, in bytes, of a non-page-aligned buffer.
        /// </returns>
        uint GetMaximumNonPageAlignedTransferSize();

        /// <summary>
        /// Retrieves the maximum page-aligned transfer size for the device.
        /// </summary>
        /// <returns>
        /// Maximum size, in bytes, of a page-aligned buffer.
        /// </returns>
        uint GetMaximumPageAlignedTransferSize();
    }

    public class MsftDiscRecorder2XClass : MsftDiscRecorder2Class, IDiscRecorder2X
    {
        /// <summary>
        /// Sends a MMC command to the recording device. Use this function when no data buffer 
        /// is sent to nor received from the device.
        /// </summary>
        /// <remarks>For details of the contents of the command packet and sense data, see the latest revision of the 
        /// MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// <para>Client-defined commands (CDBs) used with this method must be between 6 and 16 bytes in length. 
        /// In addition, the size of each command must match the size defined by the operation code as defined in the 
        /// following table.</para>
        /// <para><list type="table">
        ///    <listheader>
        ///        <term>CDB group</term>
        ///        <term>CDB operation code range</term>
        ///        <term>Required CDB size</term>
        ///    </listheader>
        ///    <item>
        ///        <description>0</description>
        ///        <description>0x00 — 0x1F</description>
        ///        <description>6 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>1</description>
        ///        <description>0x20 — 0x3F</description>
        ///        <description>10 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>2</description>
        ///        <description>0x40 — 0x5F</description>
        ///        <description>10 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>3</description>
        ///        <description>0x60 — 0x7F</description>
        ///        <description>TBD - will enforce standard-specified size requirements for this opcode range in the future.</description>
        ///    </item>
        ///    <item>
        ///        <description>4</description>
        ///        <description>0x80 — 0x9F</description>
        ///        <description>16 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>5</description>
        ///        <description>0xA0 — 0xBF</description>
        ///        <description>12 bytes</description>
        ///    </item>
        ///    <item>
        ///        <description>6</description>
        ///        <description>0xC0 — 0xDF</description>
        ///        <description>Vendor Unique - Any size allowed</description>
        ///    </item>
        ///    <item>
        ///        <description>7</description>
        ///        <description>0xE0 — 0xFF</description>
        ///        <description>Vendor Unique - Any size allowed</description>
        ///    </item>
        /// </list></para>
        /// <para>Some very early devices used vendor-unique opcodes and therefore some opcodes cannot be validated 
        /// in this manner. The following opcodes are still valid and only verify that the size is between 6 and 16 
        /// bytes:</para><para>0x02, 0x05, 0x06, 0x09, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x13, 0x14, 0x19, 0x20, 0x21, 0x22, 
        /// 0x23, 0x24, 0x26, 0x27, 0x29, 0x2C, 0x2D</para></remarks>
        /// <param name="cdb">
        /// Command packet to send to the device.
        /// Must be between 6 and 16 bytes.
        /// </param>
        /// <param name="Timeout">
        /// Time limit, in seconds, allowed for the send command to receive a result.
        /// </param>
        /// <returns>Sense data returned by the recording device.</returns>
        Byte[] IDiscRecorder2X.SendCommandNoData(Byte[] cdb, uint timeout)
        {
            if (cdb.Length < 6 || cdb.Length > 16)
            {
                // TODO: log error
                throw new ArgumentException("The command packet to send to the device must be between 6 and 16 bytes."); 
            }

            Byte[] senseBuffer = new Byte[18];
            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.SendCommandNoData(cdb, (uint)cdb.Length, senseBuffer, timeout);

            return senseBuffer;
        }

        /// <summary>
        /// Sends a MMC command and its associated data buffer to the recording device.
        /// </summary>
        /// <param name="cdb">
        /// Command packet to send to the device. Must be between 6 and 16 bytes.
        /// </param>
        /// <param name="buffer">
        /// Buffer containing data associated with the send command. Must not be NULL.
        /// </param>
        /// <param name="timeout">
        /// Time limit, in seconds, allowed for the send command to receive a result.
        /// </param>
        /// <returns>
        /// Sense data returned by the recording device.
        /// </returns>
        Byte[] IDiscRecorder2X.SendCommandSendDataToDevice(Byte[] cdb, Byte[] buffer, uint timeout)
        {
            if (cdb.Length < 6 || cdb.Length > 16)
            {
                // TODO: log error
                throw new ArgumentException("The command packet to send to the device must be between 6 and 16 bytes.");
            }

            if (buffer == null)
            {
                // TODO: log error
                throw new ArgumentNullException("buffer", "The data associated with the send command must not be NULL.");
            }

            Byte[] senseBuffer = new Byte[18];
            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.SendCommandSendDataToDevice(cdb, (uint)cdb.Length, 
                                                   senseBuffer, 
                                                   timeout, 
                                                   buffer, (uint)buffer.Length);

            return senseBuffer;
        }

        /// <summary>
        /// Sends a MMC command to the recording device requesting data from the device.
        /// </summary>
        /// <param name="cdb">
        /// Command packet to send to the device. Must be between 6 and 16 bytes.
        /// </param>
        /// <param name="timeout">
        /// Time limit, in seconds, allowed for the send command to receive a result.
        /// </param>
        /// <param name="buffer">
        /// Application-allocated data buffer that will receive data associated 
        /// with the send command. Must not be null.
        /// </param>
        /// <returns>
        /// Sense data returned by the recording device.
        /// </returns>
        Byte[] IDiscRecorder2X.SendCommandGetDataFromDevice(Byte[] cdb, Byte[] buffer, uint timeout)
        {
            if (cdb.Length < 6 || cdb.Length > 16)
            {
                // TODO: log error
                throw new ArgumentException("The command packet to send to the device must be between 6 and 16 bytes.");
            }

            if (buffer == null || buffer.Length == 0)
            {
                // TODO: log error
                throw new ArgumentNullException("buffer", "The buffer that will receive data associated with the send command must not be null or w/o space.");
            }

            uint bufferFetched = 0;
            Byte[] senseBuffer = new Byte[18];
            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.SendCommandGetDataFromDevice(cdb, (uint)cdb.Length,
                                                   senseBuffer,
                                                   timeout,
                                                   buffer, (uint)buffer.Length, 
                                                   ref bufferFetched);
            if (buffer.Length != bufferFetched)
            {
                Array.Resize<Byte>(ref buffer, (int)bufferFetched);
            }

            return senseBuffer;
        }

        /// <summary>
        /// Read a DVD Structure from the media
        /// </summary>
        /// <param name="format">
        /// Format field of the command packet. Acceptable values range from zero to 0xFF.
        /// </param>
        /// <param name="address">
        /// Address field of the command packet.
        /// </param>
        /// <param name="layer">
        /// Layer field of the command packet.
        /// </param>
        /// <param name="agid">
        /// Authentication grant ID (AGID) field of the command packet.
        /// </param>
        /// <returns>
        /// Data buffer that contains the DVD structure. 
        /// For details of the contents of the data buffer, see the READ DISC STRUCTURE command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// This method removes headers from the buffer.
        /// </returns>
        Byte[] IDiscRecorder2X.ReadDvdStructure(uint format, uint address, uint layer, uint agid)
        {
            if (format > 0xFF)
            {
                // TODO: log error
                throw new ArgumentOutOfRangeException("format", "Acceptable values range from zero to 0xFF.");
            }

            uint count = 0;
            IntPtr dvdStructurePtr;
            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.ReadDvdStructure(format, address, layer, agid, out dvdStructurePtr, ref count);

            Byte[] dvdStructure = new Byte[count];

            Marshal.Copy(dvdStructurePtr, dvdStructure, 0, (int)count);
            Marshal.FreeCoTaskMem(dvdStructurePtr);

            return dvdStructure;
        }

        /// <summary>
        /// Sends a DVD structure to the media.
        /// </summary>
        /// <param name="format">
        /// Format field of the command packet. Acceptable values range from zero to 0xFF.
        /// </param>
        /// <param name="data">
        /// Data buffer that contains the DVD structure to send to the media. 
        /// Do not include a header; this method generates and prepends a header to the DVD structure.
        /// </param>
        void IDiscRecorder2X.SendDvdStructure(Byte[] dvdStructure, uint format)
        {
            if (format > 0xFF)
            {
                // TODO: log error
                throw new ArgumentOutOfRangeException("format", "Acceptable values range from zero to 0xFF.");
            }

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.SendDvdStructure(format, dvdStructure, (uint)dvdStructure.Length);
        }

        /// <summary>
        /// Retrieves the adapter descriptor for the device (via IOCTL_STORAGE_QUERY_PROPERTY).
        /// </summary>
        /// <returns>
        /// Data buffer that contains the descriptor of the storage adapter. 
        /// For details of the contents of the data buffer, see the STORAGE_ADAPTER_DESCRIPTOR structure in the DDK.
        /// </returns>
        Byte[] IDiscRecorder2X.GetAdapterDescriptor()
        {
            uint byteSize = 0;
            IntPtr adapterDescriptorPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetAdapterDescriptor(out adapterDescriptorPtr, ref byteSize);

            Byte[] adapterDescriptor = new Byte[byteSize];

            Marshal.Copy(adapterDescriptorPtr, adapterDescriptor, 0, (int)byteSize);
            Marshal.FreeCoTaskMem(adapterDescriptorPtr);

            return adapterDescriptor;
        }

        /// <summary>
        /// Retrieves the device descriptor for the device (via IOCTL_STORAGE_QUERY_PROPERTY).
        /// </summary>
        /// <returns>
        /// Data buffer that contains the descriptor of the storage device. 
        /// For details of the contents of the data buffer, see the STORAGE_DEVICE_DESCRIPTOR structure in the DDK.
        /// </returns>
        Byte[] IDiscRecorder2X.GetDeviceDescriptor()
        {
            uint byteSize = 0;
            IntPtr deviceDescriptorPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetDeviceDescriptor(out deviceDescriptorPtr, ref byteSize);

            Byte[] deviceDescriptor = new Byte[byteSize];

            Marshal.Copy(deviceDescriptorPtr, deviceDescriptor, 0, (int)byteSize);
            Marshal.FreeCoTaskMem(deviceDescriptorPtr);

            return deviceDescriptor;
        }

        /// <summary>
        /// Retrieves the disc information from the media (via READ_DISC_INFORMATION).</summary>
        /// <returns>
        /// Data buffer that contains disc information from the media. 
        /// For details of the contents of the data buffer, see the READ DISC INFORMATION command in the latest 
        /// revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// </returns>
        Byte[] IDiscRecorder2X.GetDiscInformation()
        {
            uint byteSize = 0;
            IntPtr discDescriptorPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetDiscInformation(out discDescriptorPtr, ref byteSize);

            Byte[] discDescriptor = new Byte[byteSize];

            Marshal.Copy(discDescriptorPtr, discDescriptor, 0, (int)byteSize);
            Marshal.FreeCoTaskMem(discDescriptorPtr);

            return discDescriptor;
        }

        /// <summary>
        /// Retrieves the track information from the media (via READ_TRACK_INFORMATION).
        /// </summary>
        /// <param name="address">
        /// Address field. The addressType parameter provides additional context for 
        /// this parameter.
        /// </param>
        /// <param name="addressType">
        /// Type of address specified in the address parameter, for example, 
        /// if this is an LBA address or a track number. For possible values, see the ReadTrackAddressType 
        /// enumeration type.
        /// </param>
        /// <returns>
        /// Data buffer that contains the track information. 
        /// For details of the contents of the data buffer, see the READ TRACK INFORMATION command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// </returns>
        Byte[] IDiscRecorder2X.GetTrackInformation(uint address, ReadTrackAddressType addressType)
        {
            uint byteSize = 0;
            IntPtr trackInformationPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetTrackInformation(address, addressType, out trackInformationPtr, ref byteSize);

            Byte[] trackInformation = new Byte[byteSize];

            Marshal.Copy(trackInformationPtr, trackInformation, 0, (int)byteSize);
            Marshal.FreeCoTaskMem(trackInformationPtr);

            return trackInformation;
        }

        /// <summary>
        /// Retrieves the specified feature page from the device (via GET_CONFIGURATION).
        /// </summary>
        /// <param name="requestedFeature">
        /// Feature page to retrieve. For possible values, see the 
        /// FeaturePageType enumeration type.
        /// </param>
        /// <param name="currentFeatureOnly">
        /// Set to True to retrieve the feature page only when it is the current 
        /// feature page. Otherwise, False to retrieve the feature page regardless of it being the current feature 
        /// page.
        /// </param>
        /// <returns>
        /// Data buffer that contains the feature page. For details of the contents of the 
        /// data buffer, see the GET CONFIGURATION command in the latest revision of the MMC specification at 
        /// ftp://ftp.t10.org/t10/drafts/mmc5/. 
        /// This method removes header information and other non-feature data before filling and sending this buffer. 
        /// </returns>
        Byte[] IDiscRecorder2X.GetFeaturePage(FeaturePageType requestedFeature, bool currentFeatureOnly)
        {
            uint byteSize = 0;
            IntPtr featureDataPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetFeaturePage(requestedFeature, currentFeatureOnly, out featureDataPtr, ref byteSize);

            Byte[] featureData = new Byte[byteSize];

            Marshal.Copy(featureDataPtr, featureData, 0, (int)byteSize);
            Marshal.FreeCoTaskMem(featureDataPtr);

            return featureData;
        }

        /// <summary>
        /// Retrieves the specified mode page from the device (via MODE_SENSE10)
        /// </summary>
        /// <param name="requestedModePage">
        /// Mode page to retrieve. 
        /// For possible values, see the ModePageType enumeration type.
        /// </param>
        /// <param name="requestType">
        /// Type of mode page data to retrieve, for example, the current settings or 
        /// the settings that are write enabled. For possible values, see the ModePageRequestType 
        /// enumeration type.
        /// </param>
        /// <returns>
        /// Data buffer that contains the mode page. 
        /// For details of the contents of the data buffer, see the MODE SENSE (10) command in the latest revision 
        /// of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/. 
        /// This method removes header information and other non-page data before returning the buffer. 
        /// </returns>
        Byte[] IDiscRecorder2X.GetModePage(ModePageType requestedModePage, ModePageRequestType requestType)
        {
            uint byteSize = 0;
            IntPtr modePageDataPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetModePage(requestedModePage, requestType, out modePageDataPtr, ref byteSize);

            Byte[] modePageData = new Byte[byteSize];

            Marshal.Copy(modePageDataPtr, modePageData, 0, (int)byteSize);
            Marshal.FreeCoTaskMem(modePageDataPtr);

            return modePageData;
        }

        /// <summary>
        /// Sets the mode page data for the device (via MODE_SELECT10).
        /// </summary>
        /// <param name="requestType">
        /// Type of mode page data to send. 
        /// For possible values, see the ModePageRequestType enumeration type.
        /// </param>
        /// <param name="modePage">
        /// Data buffer that contains the mode page data to send to the media. 
        /// Do not include a header; this method generates and prepends a header to the mode page data. 
        /// For details on specifying the fields of the mode page data, see the MODE SELECT (10) command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// </param>
        void IDiscRecorder2X.SetModePage(ModePageRequestType requestType, Byte[] modePage)
        {
            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.SetModePage(requestType, modePage, (uint)modePage.Length);
        }

        /// <summary>
        /// Retrieves the list of supported feature pages or the current feature pages of the device.
        /// </summary>
        /// <param name="currentFeatureOnly">
        /// Set to True to retrieve only current feature pages. 
        /// Otherwise, False to retrieve all feature pages that the device supports.
        /// </param>
        /// <returns>
        /// Data buffer that contains one or more feature page types. 
        /// For possible values, see the FeaturePageType enumeration type.  
        /// To get the feature page data associated with the feature page type, call the 
        /// IDiscRecorder2X.GetFeaturePage method. 
        /// </returns>
        Byte[] IDiscRecorder2X.GetSupportedFeaturePages(bool currentFeatureOnly)
        {
            uint byteSize = 0;
            IntPtr featureDataPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetSupportedFeaturePages(currentFeatureOnly, out featureDataPtr, ref byteSize);

            Byte[] featureData = new Byte[byteSize];

            Marshal.Copy(featureDataPtr, featureData, 0, (int)byteSize);
            Marshal.FreeCoTaskMem(featureDataPtr);

            return featureData;
        }

        /// <summary>
        /// Retrieves the supported profiles or the current profiles of the device.
        /// </summary>
        /// <param name="currentOnly">
        /// Set to True to retrieve the current profiles. 
        /// Otherwise, False to return all supported profiles of the device.
        /// </param>
        /// <returns>
        /// Data buffer that contains one or more profile types. 
        /// For possible values, see the ProfileType enumeration type.
        /// </returns>
        Byte[] IDiscRecorder2X.GetSupportedProfiles(bool currentOnly)
        {
            uint validProfiles = 0;
            IntPtr profileTypesPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetSupportedProfiles(currentOnly, out profileTypesPtr, ref validProfiles);

            Byte[] profileTypes = new Byte[validProfiles];

            Marshal.Copy(profileTypesPtr, profileTypes, 0, (int)validProfiles);
            Marshal.FreeCoTaskMem(profileTypesPtr);

            return profileTypes;
        }

        /// <summary>
        /// Retrieves the supported mode pages for the device.
        /// </summary>
        /// <param name="requestType">
        /// Type of mode page data to retrieve, for example, 
        /// the current settings or the settings that are write enabled. 
        /// For possible values, see the ModePageRequestType enumeration type.
        /// </param>
        /// <returns>
        /// Data buffer that contains one or more mode page types. 
        /// For possible values, see the ModePageType enumeration type. 
        /// To get the mode page data associated with the mode page type, call the IDiscRecorder2Ex.GetModePage 
        /// method.
        /// </returns>
        Byte[] IDiscRecorder2X.GetSupportedModePages(ModePageRequestType requestType)
        {
            uint byteSize = 0;
            IntPtr modePageTypesPtr;

            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            recorderEx.GetSupportedModePages(requestType, out modePageTypesPtr, ref byteSize);

            Byte[] modePageTypes = new Byte[byteSize];

            Marshal.Copy(modePageTypesPtr, modePageTypes, 0, (int)byteSize);
            Marshal.FreeCoTaskMem(modePageTypesPtr);

            return modePageTypes;
        }

        /// <summary>
        /// Retrieves the byte alignment mask for the device.
        /// </summary>
        /// <returns>
        /// Byte alignment mask that you use to determine if the buffer is aligned to the correct byte 
        /// boundary for the device. The byte alignment value is always a number that is a power of 2.
        /// </returns>
        uint IDiscRecorder2X.GetByteAlignmentMask()
        {
            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            return recorderEx.GetByteAlignmentMask();
        }

        /// <summary>
        /// Retrieves the maximum non-page-aligned transfer size for the device.
        /// </summary>
        /// <returns>
        /// Maximum size, in bytes, of a non-page-aligned buffer.
        /// </returns>
        uint IDiscRecorder2X.GetMaximumNonPageAlignedTransferSize()
        {
            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            return recorderEx.GetMaximumNonPageAlignedTransferSize();
        }

        /// <summary>
        /// Retrieves the maximum page-aligned transfer size for the device.
        /// </summary>
        /// <returns>
        /// Maximum size, in bytes, of a page-aligned buffer.
        /// </returns>
        uint IDiscRecorder2X.GetMaximumPageAlignedTransferSize()
        {
            IDiscRecorder2Ex recorderEx = (IDiscRecorder2Ex)this;

            return recorderEx.GetMaximumPageAlignedTransferSize();
        }
    }

}
