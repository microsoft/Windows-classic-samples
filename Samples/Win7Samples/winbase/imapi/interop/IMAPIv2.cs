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
    #region Interfaces
    /// <summary>Use this interface to enumerate the CD and DVD devices installed on the computer.</summary>
    [
        ComImport,
        Guid("27354130-7F64-5B0F-8F00-5D77AFBE261E"), 
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscMaster2 : IEnumerable
    {
        /// <summary>Retrieves a list of type5 devices installed on the computer.</summary>
        /// <returns>list of the type5 devices installed</returns>
        [DispId(-4)]
        [return: MarshalAs(UnmanagedType.CustomMarshaler,
                 MarshalTypeRef=typeof(EnumeratorToEnumVariantMarshaler))]
        new IEnumerator GetEnumerator();

        /// <summary>Retrieves the unique identifier of the specified disc device.</summary>
        /// <param name="index">Zero based index</param>
        /// <returns>Recorder's unique id</returns>
        [DispId(0)]
        String this[int index] { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>Retrieves the number of the CD and DVD disc devices installed on the computer.</summary>
        [DispId(1)]
        int Count { get; }

        /// <summary>Retrieves a value that determines if the environment contains one or more optical devices 
        /// and the execution context has permission to access the devices.</summary>
        [DispId(2)]
        bool IsSupportedEnvironment { [return: MarshalAs(UnmanagedType.VariantBool)] get; }
    }

    /// <summary>This interface represents a physical device. 
    /// You use this interface to retrieve information about a CD and DVD device installed on the computer and to 
    /// perform operations such as closing the tray or eject the media.</summary>
    /// <remarks>To write data to media, you need to attach a recorder to a format writer, for example, to attach the 
    /// recorder to a data writer, call the <see cref="IDiscFormat2Data.Recorder"/> method. 
    /// <para>Several properties of this interface return packet data defined by Multimedia Command (MMC). 
    /// For information on the format of the packet data, see the latest revision of the MMC specification at 
    /// ftp://ftp.t10.org/t10/drafts/mmc5/. </para></remarks>
    [
        ComImport,
        Guid("27354133-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscRecorder2
    {
        /// <summary>Ejects the media (if any) and opens the tray</summary>
        [DispId(256)]
        void EjectMedia();

        /// <summary>Close the media tray and load any media in the tray.</summary>
        /// <remarks>Some drives, such as those with slot-loading mechanisms, do not support this method. 
        /// To determine if the device supports this method, call <see cref="IDiscRecorder2.DeviceCanLoadMedia"/> 
        /// property.</remarks>
        [DispId(257)]
        void CloseTray();

        /// <summary>Acquires exclusive access to device. May be called multiple times.</summary>
        /// <remarks>You should not have to call this method to acquire the lock yourself because the write 
        /// operations, such as <see cref="IDiscFormat2Data.Write"/>, acquires the lock for you. 
        /// <para>Each recorder has a lock count. 
        /// The first call to a recorder locks the device for exclusive access. Applications can use the 
        /// AcquireExclusiveAccess method multiple times to apply multiple locks on a device. Each call increments 
        /// the lock count by one.</para><para>When unlocking a recorder, the lock count must reach zero to free the 
        /// device for other clients. Calling the <see cref="IDiscRecorder2.ReleaseExclusiveAccess"/> method 
        /// decrements the lock count by one.</para> 
        /// <para>An equal number of calls to the AcquireExclusiveAccess and 
        /// <see cref="IDiscRecorder2.ReleaseExclusiveAccess"/> methods are needed to 
        /// free a device. Should the application exit unexpectedly or crash while holding the exclusive access, the 
        /// CDROM.SYS driver will automatically release these exclusive locks.</para><para>If the device is already 
        /// locked, you can call <see cref="IDiscRecorder2.ExclusiveAccessOwner"/> to retrieve the name of the client 
        /// application that currently has exclusive access.</para></remarks>
        /// <param name="force">Set to true to gain exclusive access to the volume whether the file system volume 
        /// can or cannot be dismounted. 
        /// If false, this method gains exclusive access only when there is no file system mounted on the volume.</param>
        /// <param name="clientName">String that contains the friendly name of the client application requesting 
        /// exclusive access. Cannot be NULL or a zero-length string. The string must conform to the restrictions 
        /// for the IOCTL_CDROM_EXCLUSIVE_ACCESS control code found in the DDK.</param>
        [DispId(258)]
        void AcquireExclusiveAccess([MarshalAs(UnmanagedType.VariantBool)] bool force, [MarshalAs(UnmanagedType.BStr)] String clientName);

        /// <summary>Releases exclusive access to device. Call once per 
        /// <see cref="IDiscRecorder2.AcquireExclusiveAccess"/>.</summary>
        /// <remarks>Each recorder has a lock count. The first call to a recorder locks the device for exclusive access. 
        /// Applications can use the <see cref="IDiscRecorder2.AcquireExclusiveAccess"/> method multiple times to 
        /// apply multiple locks on a device. 
        /// Each call increments the lock count by one. When unlocking a recorder, the lock count 
        /// must reach zero to free the device for other clients. Calling the ReleaseExclusiveAccess method decrements 
        /// the lock count by one. An equal number of calls to the <see cref="IDiscRecorder2.AcquireExclusiveAccess"/> 
        /// and ReleaseExclusiveAccess methods are needed to free a device. 
        /// When the lock count reaches zero, recording device is free; the last lock has been removed.</remarks>
        [DispId(259)]
        void ReleaseExclusiveAccess();

        /// <summary>Disables Media Change Notification (MCN) for the device.</summary>
        /// <remarks>MCN is the CD-ROM device driver's method of detecting media change and state changes in the 
        /// CD-ROM device. For example, when you change the media in a CD-ROM device, a MCN message is sent to 
        /// trigger media features, such as Autoplay. To disable the features, call this method. To enable 
        /// notifications, call the <see cref="IDiscRecorder2.EnableMcn"/> method. If the application crashes or closes 
        /// unexpectedly, then MCN is automatically re-enabled by the driver. Note that DisableMcn increments a 
        /// reference count each time it is called. The EnableMcn method decrements the count. The device is enabled 
        /// when the reference count is zero.</remarks>
        [DispId(260)]
        void DisableMcn();

        /// <summary>Enables Media Change Notification (MCN) for the device.</summary>
        /// <remarks>MCN is the CD-ROM device driver's method of detecting media change and state changes in the 
        /// CD-ROM device. For example, when you change the media in a CD-ROM device, a MCN message is sent to 
        /// trigger media features, such as Autoplay. MCN is enabled by default. Call this method to enable 
        /// notifications when the notifications have been disabled using <see cref="IDiscRecorder2.DisableMcn"/>. 
        /// Note that <see cref="DisableMcn"/> increments a reference count each time it is called. 
        /// The EnableMcn method decrements the count. The device is enabled when the reference count is zero.</remarks>
        [DispId(261)]
        void EnableMcn();

        /// <summary>Associates the object with the specified disc device.</summary>
        /// <remarks>You must initialize the recorder before calling any of the methods of this interface. 
        /// To retrieve a list of devices on the computer and their unique identifiers, call the 
        /// <see cref="IDiscMaster2.NewEnum"/> method. This method will not fail on a drive that is exclusively 
        /// locked. However, if the drive is exclusively locked, several of the methods of this interface may return 
        /// E_IMAPI_RECORDER_LOCKED. To determine who has exclusive access, call the 
        /// <see cref="IDiscRecorder2.ExclusiveAccessOwner"/> method.</remarks>
        /// <param name="recorderUniqueId">String that contains the unique identifier for the device.</param>
        [DispId(262)]
        void InitializeDiscRecorder([MarshalAs(UnmanagedType.BStr)] String recorderUniqueId);

        /// <summary>Retrieves the unique identifier used to initialize the disc device.</summary>
        [DispId(0)]
        String ActiveDiscRecorder { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>Retrieves the vendor ID for the device.</summary>
        [DispId(513)]
        String VendorId { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>Retrieves the product ID of the device.</summary>
        [DispId(514)]
        String ProductId { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>Retrieves the product revision code of the device.</summary>
        [DispId(515)]
        String ProductRevision { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>Retrieves the unique volume name associated with the device.</summary>
        /// <remarks>To retrieve the drive letter assignment, call the 
        /// <see cref="IDiscRecorder2.get_VolumePathNames"/> method.</remarks>
        [DispId(516)]
        String VolumeName { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>Retrieves a list of drive letters and NTFS mount points for the device.</summary>
        [DispId(517)]
        String[] VolumePathNames { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType=VarEnum.VT_VARIANT)] get; }

        /// <summary>Determines if the device can eject and subsequently reload media.</summary>
        /// <remarks>Drives that use a tray design can reload media by closing the tray after ejecting the media. 
        /// In contrast, caddy-based or cartridge-based drives cannot reload ejected media.</remarks>
        [DispId(518)]
        bool DeviceCanLoadMedia { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>Retrieves the legacy device number for a CD or DVD device.</summary>
        [DispId(519)]
        int LegacyDeviceNumber { get; }

        /// <summary>Retrieves the list of features that the device supports.</summary>
        [DispId(520)]
        Enum[] SupportedFeaturePages { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>Retrieves the list of feature pages of the device that are marked as current.</summary>
        [DispId(521)]
        Enum[] CurrentFeaturePages { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>Retrieves the list of MMC profiles that the device supports.</summary>
        [DispId(522)]
        Enum[] SupportedProfiles { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>Retrieves all MMC profiles of the device that are marked as current.</summary>
        [DispId(523)]
        Enum[] CurrentProfiles { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>Retrieves the list of MMC mode pages that the device supports.</summary>
        [DispId(524)]
        Enum[] SupportedModePages { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>Retrieves the name of the client application that has exclusive access to the device.</summary>
        /// <remarks>This property returns the current exclusive access owner of the device. 
        /// This value comes directly from CDROM.SYS and should be queried anytime an operation fails with error 
        /// E_IMAPI_RECORDER_LOCKED.</remarks>
        [DispId(525)]
        String ExclusiveAccessOwner { [return: MarshalAs(UnmanagedType.BStr)] get; }
    }

    /// <summary>This interface represents a physical device. You use this interface to retrieve information about a 
    /// CD and DVD device installed on the computer and to perform operations such as closing the tray or ejecting 
    /// the media. This interface retrieves information not available through <see cref="IDiscRecorder2"/> interface, 
    /// and provides easier access to some of the same property values in <see cref="IDiscRecorder2"/>. 
    /// To get an instance of this interface, create an instance of the <see cref="IDiscRecorder2"/> interface and 
    /// then cast it to IDiscRecorder2Ex interface.</summary>
    /// <remarks>To write data to media, you need to attach this recorder to the <see cref="IWriteEngine2"/> data 
    /// writer, using the <see cref="IWriteEngine2.Recorder"/> method.</remarks>
    [
        ComImport,
        Guid("27354132-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown)
    ]
    public interface IDiscRecorder2Ex
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
        /// <param name="Cdb">Command packet to send to the device.</param>
        /// <param name="CdbSize">Size, in bytes, of the command packet to send. Must be between 
        /// 6 and 16 bytes.</param>
        /// <param name="SenseBuffer">Sense data returned by the recording device.</param>
        /// <param name="Timeout">Time limit, in seconds, allowed for the send command to receive 
        /// a result.</param>
        void SendCommandNoData([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 1)] 
                               Byte[] cdb,
                               uint cdbSize,
                               [MarshalAs(UnmanagedType.LPArray, SizeConst = 18)] 
                               Byte[] senseBuffer,
                               uint timeout);

        /// <summary>Sends a MMC command and its associated data buffer to the recording device.</summary>
        /// <param name="Cdb">Command packet to send to the device.</param>
        /// <param name="CdbSize">Size, in bytes, of the command packet to send. Must be between 
        /// 6 and 16 bytes.</param>
        /// <param name="SenseBuffer">Sense data returned by the recording device.</param>
        /// <param name="Timeout">Time limit, in seconds, allowed for the send command to receive 
        /// a result.</param>
        /// <param name="Buffer">Buffer containing data associated with the send command. 
        /// Must not be NULL.</param>
        /// <param name="BufferSize">Size, in bytes, of the data buffer to send. Must not be zero.</param>
        void SendCommandSendDataToDevice([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 1)] 
                                         Byte[] cdb,
                                         uint cdbSize,
                                         [MarshalAs(UnmanagedType.LPArray, SizeConst = 18)] 
                                         Byte[] senseBuffer,
                                         uint timeout,
                                         [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 5)] 
                                         Byte[] buffer,
                                         uint bufferSize);

        /// <summary>Sends a MMC command to the recording device requesting data from the device.</summary>
        /// <param name="Cdb">Command packet to send to the device.</param>
        /// <param name="CdbSize">Size, in bytes, of the command packet to send. Must be between 
        /// 6 and 16 bytes.</param>
        /// <param name="SenseBuffer">Sense data returned by the recording device.</param>
        /// <param name="Timeout">Time limit, in seconds, allowed for the send command to receive 
        /// a result.</param>
        /// <param name="Buffer">Application-allocated data buffer that will receive data associated 
        /// with the send command. Must not be NULL.</param>
        /// <param name="BufferSize">Size, in bytes, of the Buffer data buffer. Must not be zero.</param>
        /// <param name="BufferFetched">Size, in bytes, of the data returned in the Buffer data buffer.</param>
        void SendCommandGetDataFromDevice([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 1)] 
                                          Byte[] cdb,
                                          uint cdbSize,
                                          [MarshalAs(UnmanagedType.LPArray, SizeConst = 18)] 
                                          Byte[] senseBuffer,
                                          uint timeout,
                                          [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 5)] 
                                          Byte[] buffer,
                                          uint bufferSize,
                                          ref uint bufferFetched);

        /// <summary>Read a DVD Structure from the media</summary>
        /// <param name="format">Format field of the command packet. 
        /// Acceptable values range from zero to 0xFF.</param>
        /// <param name="address">Address field of the command packet.</param>
        /// <param name="layer">Layer field of the command packet.</param>
        /// <param name="agid">Authentication grant ID (AGID) field of the command packet.</param>
        /// <param name="data">Data buffer that contains the DVD structure. 
        /// For details of the contents of the data buffer, see the READ DISC STRUCTURE command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// This method removes headers from the buffer.
        /// When done, free the memory.</param>
        /// <param name="Count"></param>
        void ReadDvdStructure(uint format,
                              uint address,
                              uint layer,
                              uint agid,
                              out IntPtr dvdStructurePtr, 
                              ref uint count);

        /// <summary>Read a DVD Structure from the media</summary>
        /// <param name="format">Format field of the command packet. 
        /// Acceptable values range from zero to 0xFF.</param>
        /// <param name="data">Data buffer that contains the DVD structure to send to the media. 
        /// Do not include a header; this method generates and prepends a header to the DVD structure.</param>
        /// <param name="Count">Size, in bytes, of the data buffer.</param>
        void SendDvdStructure(uint format,
                              [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 2)] 
                              Byte[] dvdStructure,
                              uint count);

        /// <summary>Retrieves the adapter descriptor for the device (via IOCTL_STORAGE_QUERY_PROPERTY).</summary>
        /// <param name="data">Data buffer that contains the descriptor of the storage adapter. 
        /// For details of the contents of the data buffer, see the STORAGE_ADAPTER_DESCRIPTOR structure in the DDK.
        /// When done, free the memory.</param>
        /// <param name="byteSize">Size, in bytes, of the data buffer.</param>
        void GetAdapterDescriptor(out IntPtr adapterDescriptorPtr, 
                                  ref uint byteSize);

        /// <summary>Retrieves the device descriptor for the device (via IOCTL_STORAGE_QUERY_PROPERTY).</summary>
        /// <param name="data">Data buffer that contains the descriptor of the storage device. 
        /// For details of the contents of the data buffer, see the STORAGE_DEVICE_DESCRIPTOR structure in the DDK.
        /// When done, call the CoTaskMemFree function to free the memory.</param>
        /// <param name="byteSize">Size, in bytes, of the data buffer.</param>
        void GetDeviceDescriptor(out IntPtr deviceDescriptorPtr, 
                                 ref uint byteSize);

        /// <summary>Retrieves the disc information from the media (via READ_DISC_INFORMATION).</summary>
        /// <param name="discInformation">Data buffer that contains disc information from the media. 
        /// For details of the contents of the data buffer, see the READ DISC INFORMATION command in the latest 
        /// revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// When done, free the memory.</param>
        /// <param name="byteSize">Size, in bytes, of the data buffer.</param>
        void GetDiscInformation(out IntPtr discInformationPtr, 
                                ref uint byteSize);

        /// <summary>Retrieves the track information from the media (via READ_TRACK_INFORMATION).</summary>
        /// <param name="address">Address field. The addressType parameter provides additional context for 
        /// this parameter.</param>
        /// <param name="addressType">Type of address specified in the address parameter, for example, 
        /// if this is an LBA address or a track number. For possible values, see the ReadTrackAddressType 
        /// enumeration type.</param>
        /// <param name="trackInformation">Data buffer that contains the track information. 
        /// For details of the contents of the data buffer, see the READ TRACK INFORMATION command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.
        /// When done, free the memory.</param>
        /// <param name="byteSize">Size, in bytes, of the trackInformation data buffer.</param>
        void GetTrackInformation(uint address,
                                 ReadTrackAddressType addressType,
                                 out IntPtr trackInformationPtr, 
                                 ref uint byteSize);

        /// <summary>Retrieves the specified feature page from the device (via GET_CONFIGURATION).</summary>
        /// <param name="requestedFeature">Feature page to retrieve. For possible values, see the 
        /// FeaturePageType enumeration type.</param>
        /// <param name="currentFeatureOnly">Set to True to retrieve the feature page only when it is the current 
        /// feature page. Otherwise, False to retrieve the feature page regardless of it being the current feature 
        /// page.</param>
        /// <param name="featureData">Data buffer that contains the feature page. For details of the contents of the 
        /// data buffer, see the GET CONFIGURATION command in the latest revision of the MMC specification at 
        /// ftp://ftp.t10.org/t10/drafts/mmc5/. 
        /// This method removes header information and other non-feature data before filling and sending this buffer. 
        /// When done, free the memory.</param>
        /// <param name="byteSize">Size, in bytes, of the featureData data buffer.</param>
        void GetFeaturePage(FeaturePageType requestedFeature,
                            [MarshalAs(UnmanagedType.U1)] 
                            bool currentFeatureOnly,
                            out IntPtr featureDataPtr, 
                            ref uint byteSize);

        /// <summary>Retrieves the specified mode page from the device (via MODE_SENSE10)</summary>
        /// <param name="requestedModePage">Mode page to retrieve. 
        /// For possible values, see the ModePageType enumeration type.</param>
        /// <param name="requestType">Type of mode page data to retrieve, for example, the current settings or 
        /// the settings that are write enabled. For possible values, see the ModePageRequestType 
        /// enumeration type.</param>
        /// <param name="modePageData">Data buffer that contains the mode page. 
        /// For details of the contents of the data buffer, see the MODE SENSE (10) command in the latest revision 
        /// of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/. 
        /// This method removes header information and other non-page data before returning the buffer. 
        /// When done, free the memory.</param>
        /// <param name="byteSize">Size, in bytes, of the modePageData data buffer.</param>
        void GetModePage(ModePageType requestedModePage,
                         ModePageRequestType requestType,
                         out IntPtr modePageDataPtr, 
                         ref uint byteSize);

        /// <summary>Sets the mode page data for the device (via MODE_SELECT10).</summary>
        /// <param name="requestType">Type of mode page data to send. 
        /// For possible values, see the ModePageRequestType enumeration type.</param>
        /// <param name="data">Data buffer that contains the mode page data to send to the media. 
        /// Do not include a header; this method generates and prepends a header to the mode page data. 
        /// For details on specifying the fields of the mode page data, see the MODE SELECT (10) command in the 
        /// latest revision of the MMC specification at ftp://ftp.t10.org/t10/drafts/mmc5/.</param>
        /// <param name="byteSize">Size, in bytes, of the data buffer.</param>
        void SetModePage(ModePageRequestType requestType,
                         [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 2)] 
                         Byte[] modePage,
                         uint byteSize);

        /// <summary>Retrieves the list of supported feature pages or the current feature pages of the device.</summary>
        /// <param name="currentFeatureOnly">Set to True to retrieve only current feature pages. 
        /// Otherwise, False to retrieve all feature pages that the device supports.</param>
        /// <param name="featureData">Data buffer that contains one or more feature page types. 
        /// For possible values, see the FeaturePageType enumeration type.  
        /// To get the feature page data associated with the feature page type, call the 
        /// IDiscRecorder2Ex.GetFeaturePage method. 
        /// When done, free the memory.</param>
        /// <param name="byteSize">Number of supported feature pages in the featureData data buffer.</param>
        void GetSupportedFeaturePages([MarshalAs(UnmanagedType.U1)] 
                                      bool currentFeatureOnly,
                                      out IntPtr featureDataPtr, 
                                      ref uint byteSize);

        /// <summary>Retrieves the supported profiles or the current profiles of the device.</summary>
        /// <param name="currentOnly">Set to True to retrieve the current profiles. 
        /// Otherwise, False to return all supported profiles of the device.</param>
        /// <param name="profileTypes">Data buffer that contains one or more profile types. 
        /// For possible values, see the ProfileType enumeration type.  
        /// When done, free the memory.</param>
        /// <param name="validProfiles">Number of supported profiles in the profileTypes data buffer.</param>
        void GetSupportedProfiles([MarshalAs(UnmanagedType.U1)] 
                                  bool currentOnly,
                                  out IntPtr profileTypesPtr, 
                                  ref uint validProfiles);

        /// <summary>Retrieves the supported mode pages for the device.</summary>
        /// <param name="requestType">Type of mode page data to retrieve, for example, 
        /// the current settings or the settings that are write enabled. 
        /// For possible values, see the ModePageRequestType enumeration type.</param>
        /// <param name="modePageTypes">Data buffer that contains one or more mode page types. 
        /// For possible values, see the ModePageType enumeration type. 
        /// To get the mode page data associated with the mode page type, call the IDiscRecorder2Ex.GetModePage 
        /// method. 
        /// When done, call the CoTaskMemFree function to free the memory.</param>
        /// <param name="validPages">Number of mode pages in the data buffer.</param>
        void GetSupportedModePages(ModePageRequestType requestType,
                                   out IntPtr modePageTypesPtr, 
                                   ref uint validPages);

        /// <summary>Retrieves the byte alignment mask for the device.</summary>
        /// <returns>Byte alignment mask that you use to determine if the buffer is aligned to the correct byte 
        /// boundary for the device. The byte alignment value is always a number that is a power of 2.</returns>
        uint GetByteAlignmentMask();

        /// <summary>Retrieves the maximum non-page-aligned transfer size for the device.</summary>
        /// <returns>Maximum size, in bytes, of a non-page-aligned buffer.</returns>
        uint GetMaximumNonPageAlignedTransferSize();

        /// <summary>Retrieves the maximum page-aligned transfer size for the device.</summary>
        /// <returns>Maximum size, in bytes, of a page-aligned buffer.</returns>
        uint GetMaximumPageAlignedTransferSize();
    }

    /// <summary>
    /// 
    /// </summary>
    [
        ComImport,
        Guid("27354152-8F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual),
    ]
    public interface IDiskFormat2
    {
        /// <summary>
        /// Determines if the recorder object supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2048)]
        [return: MarshalAs(UnmanagedType.VariantBool)] 
        bool IsRecorderSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media in a supported recorder object 
        /// supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2049)]
        [return: MarshalAs(UnmanagedType.VariantBool)] 
        bool IsCurrentMediaSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media is reported as physically blank 
        /// by the drive
        /// </summary>
        [DispId(1792)]
        bool MediaPhysicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Attempts to determine if the media is blank using heuristics 
        /// (mainly for DVD+RW and DVD-RAM media)
        /// </summary>
        [DispId(1793)]
        bool MediaHeuristicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Supported media types
        /// </summary>
        [DispId(1794)]
        MediaPhysicalType[] SupportedMediaTypes { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }
    }

    /// <summary>
    /// 
    /// </summary>
    [
        ComImport,
        Guid("27354153-9F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscFormat2Data : IDiskFormat2
    {
        // IDiscFormat2
        /// <summary>
        /// Determines if the recorder object supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2048)]
        [return: MarshalAs(UnmanagedType.VariantBool)] 
        new bool IsRecorderSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media in a supported recorder object 
        /// supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2049)]
        [return: MarshalAs(UnmanagedType.VariantBool)] 
        new bool IsCurrentMediaSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media is reported as physically blank 
        /// by the drive
        /// </summary>
        [DispId(1792)]
        new bool MediaPhysicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Attempts to determine if the media is blank using heuristics 
        /// (mainly for DVD+RW and DVD-RAM media)
        /// </summary>
        [DispId(1793)]
        new bool MediaHeuristicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Supported media types
        /// </summary>
        [DispId(1794)]
        new MediaPhysicalType[] SupportedMediaTypes { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        // IDiscFormat2Data
        /// <summary>
        /// The disc recorder to use
        /// </summary>
        [DispId(256)]
        IDiscRecorder2 Recorder { set; [return: MarshalAs(UnmanagedType.Interface)] get; }

        /// <summary>
        /// Buffer Underrun Free recording should be disabled
        /// </summary>
        [DispId(257)]
        bool BufferUnderrunFreeDisabled { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Postgap is included in image
        /// </summary>
        [DispId(260)]
        bool PostgapAlreadyInImage { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// The state (usability) of the current media
        /// </summary>
        [DispId(262)]
        Format2DataMediaState CurrentMediaStatus { get; }

        /// <summary>
        /// The write protection state of the current media.
        /// </summary>
        [DispId(263)]
        MediaWriteProtectState WriteProtectStatus { get; }

        /// <summary>
        /// Total sectors available on the media (used + free).
        /// </summary>
        [DispId(264)]
        int TotalSectorsOnMedia { get; }

        /// <summary>
        /// Free sectors available on the media.
        /// </summary>
        [DispId(265)]
        int FreeSectorsOnMedia { get; }

        /// <summary>
        /// Next writable address on the media (also used sectors).
        /// </summary>
        [DispId(266)]
        int NextWritableAddress { get; }

        /// <summary>
        /// The first sector in the previous session on the media.
        /// </summary>
        [DispId(267)]
        int StartAddressOfPreviousSession { get; }

        /// <summary>
        /// The last sector in the previous session on the media.
        /// </summary>
        [DispId(268)]
        int LastWrittenAddressOfPreviousSession { get; }

        /// <summary>
        /// Prevent further additions to the file system
        /// </summary>
        [DispId(269)]
        bool ForceMediaToBeClosed { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Default is to maximize compatibility with DVD-ROM. 
        /// May be disabled to reduce time to finish writing the disc or 
        /// increase usable space on the media for later writing.
        /// </summary>
        [DispId(270)]
        bool DisableConsumerDvdCompatibilityMode { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Get the current physical media type.
        /// </summary>
        [DispId(271)]
        MediaPhysicalType CurrentPhysicalMediaType { get; }

        /// <summary>
        /// The friendly name of the client 
        /// (used to determine recorder reservation conflicts).
        /// </summary>
        [DispId(272)]
        String ClientName { set; [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// The last requested write speed.
        /// </summary>
        [DispId(273)]
        int RequestedWriteSpeed { get; }

        /// <summary>
        /// The last requested rotation type.
        /// </summary>
        [DispId(274)]
        bool RequestedRotationTypeIsPureCAV { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// The drive's current write speed.
        /// </summary>
        [DispId(275)]
        int CurrentWriteSpeed { get; }

        /// <summary>
        /// The drive's current rotation type.
        /// </summary>
        [DispId(276)]
        bool CurrentRotationTypeIsPureCAV { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Gets an array of the write speeds supported for the 
        /// attached disc recorder and current media
        /// </summary>
        [DispId(277)]
        uint[] SupportedWriteSpeeds { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// Gets an array of the detailed write configurations 
        /// supported for the attached disc recorder and current media
        /// </summary>
        [DispId(278)]
        IWriteSpeedDescriptor[] SupportedWriteSpeedDescriptors { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// Forces the Datawriter to overwrite the disc on overwritable media types
        /// </summary>
        [DispId(279)]
        bool ForceOverwrite { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Returns the array of available multi-session interfaces. 
        /// The array shall not be empty
        /// </summary>
        [DispId(280)]
        IMultisessionSequential[] MultisessionInterfaces { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// Writes all the data provided in the IStream to the device
        /// </summary>
        /// <param name="data"></param>
        [DispId(512)]
        void Write([In, MarshalAs(UnmanagedType.Interface)] IStream data);

        /// <summary>
        /// Cancels the current write operation
        /// </summary>
        [DispId(513)]
        void CancelWrite();

        /// <summary>
        /// Sets the write speed (in sectors per second) of the attached disc recorder
        /// </summary>
        /// <param name="RequestedSectorsPerSecond"></param>
        /// <param name="RotationTypeIsPureCAV"></param>
        [DispId(514)]
        void SetWriteSpeed(int RequestedSectorsPerSecond, [In, MarshalAs(UnmanagedType.VariantBool)] bool RotationTypeIsPureCAV);
    }

    [
        ComImport,
        Guid("27354156-8F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscFormat2Erase : IDiskFormat2
    {
        // IDiscFormat2
        /// <summary>
        /// Determines if the recorder object supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2048)]
        [return: MarshalAs(UnmanagedType.VariantBool)]
        new bool IsRecorderSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media in a supported recorder object 
        /// supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2049)]
        [return: MarshalAs(UnmanagedType.VariantBool)]
        new bool IsCurrentMediaSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media is reported as physically blank 
        /// by the drive
        /// </summary>
        [DispId(1792)]
        new bool MediaPhysicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Attempts to determine if the media is blank using heuristics 
        /// (mainly for DVD+RW and DVD-RAM media)
        /// </summary>
        [DispId(1793)]
        new bool MediaHeuristicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Supported media types
        /// </summary>
        [DispId(1794)]
        new MediaPhysicalType[] SupportedMediaTypes { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        // IDiscFormat2Erase
        [DispId(0x100)]
        IDiscRecorder2 Recorder { set; [return: MarshalAs(UnmanagedType.Interface)] get; }

        [DispId(0x101)]
        bool FullErase { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x102)]
        MediaPhysicalType CurrentPhysicalMediaType { get; }

        [DispId(0x103)]
        String ClientName { set; [return: MarshalAs(UnmanagedType.BStr)] get; }

        [DispId(0x201)]
        void EraseMedia();
    }

    [
        ComImport,
        Guid("27354144-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IWriteSpeedDescriptor
    {
        [DispId(0x101)]
        MediaPhysicalType MediaType { get; }

        [DispId(0x102)]
        bool RotationTypeIsPureCAV { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x103)]
        int WriteSpeed { get; }
    }

    [
        ComImport,
        Guid("27354150-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IMultisession
    {
        [DispId(0x100)]
        bool IsSupportedOnCurrentMediaState { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x101)]
        bool InUse { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x102)]
        IDiscRecorder2 ImportRecorder { [return: MarshalAs(UnmanagedType.Interface)] get; }
    }

    [
        ComImport,
        Guid("27354151-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IMultisessionSequential : IMultisession
    {
        [DispId(0x200)]
        bool IsFirstDataSession { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x201)]
        int StartAddressOfPreviousSession { get; }

        [DispId(0x202)]
        int LastWrittenAddressOfPreviousSession { get; }

        [DispId(0x203)]
        int NextWritableAddress { get; }

        [DispId(0x204)]
        int FreeSectorsOnMedia { get; }
    }

    [
        ComImport,
        Guid("27354154-8F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscFormat2TrackAtOnce : IDiskFormat2
    {
        // IDiscFormat2
        /// <summary>
        /// Determines if the recorder object supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2048)]
        [return: MarshalAs(UnmanagedType.VariantBool)]
        new bool IsRecorderSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media in a supported recorder object 
        /// supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2049)]
        [return: MarshalAs(UnmanagedType.VariantBool)]
        new bool IsCurrentMediaSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media is reported as physically blank 
        /// by the drive
        /// </summary>
        [DispId(1792)]
        new bool MediaPhysicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Attempts to determine if the media is blank using heuristics 
        /// (mainly for DVD+RW and DVD-RAM media)
        /// </summary>
        [DispId(1793)]
        new bool MediaHeuristicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Supported media types
        /// </summary>
        [DispId(1794)]
        new MediaPhysicalType[] SupportedMediaTypes { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        // IDiscFormat2TrackAtOnce

        [DispId(0x200)]
        void PrepareMedia();

        [DispId(0x201)]
        void AddAudioTrack(IStream data);

        [DispId(0x202)]
        void CancelAddTrack();

        [DispId(0x203)]
        void ReleaseMedia();

        [DispId(0x204)]
        void SetWriteSpeed(int requestedSectorsPerSecond, [MarshalAs(UnmanagedType.VariantBool)] bool rotationTypeIsPureCAV);

        [DispId(0x100)]
        IDiscRecorder2 Recorder { set; [return: MarshalAs(UnmanagedType.Interface)] get; }

        [DispId(0x102)]
        bool BufferUnderrunFreeDisabled { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x103)]
        int NumberOfExistingTracks { get; }

        [DispId(0x104)]
        int TotalSectorsOnMedia { get; }

        [DispId(0x105)]
        int FreeSectorsOnMedia { get; }

        [DispId(0x106)]
        int UsedSectorsOnMedia { get; }

        [DispId(0x107)]
        bool DoNotFinalizeMedia { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x10A)]
        Array ExpectedTableOfContents { [return: MarshalAs(UnmanagedType.SafeArray)] get; }

        [DispId(0x10B)]
        MediaPhysicalType CurrentPhysicalMediaType { get; }

        [DispId(0x10E)]
        String ClientName { set; [return: MarshalAs(UnmanagedType.BStr)] get; }

        [DispId(0x10F)]
        int RequestedWriteSpeed { get; }

        [DispId(0x110)]
        bool RequestedRotationTypeIsPureCAV { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x111)]
        int CurrentWriteSpeed { get; }

        [DispId(0x112)]
        bool CurrentRotationTypeIsPureCAV { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x113)]
        uint[] SupportedWriteSpeeds { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        [DispId(0x114)]
        IWriteSpeedDescriptor[] SupportedWriteSpeedDescriptors { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }
    }

    [
        ComImport,
        Guid("27354155-8F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscFormat2RawCD : IDiskFormat2
    {
        // IDiscFormat2
        /// <summary>
        /// Determines if the recorder object supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2048)]
        [return: MarshalAs(UnmanagedType.VariantBool)]
        new bool IsRecorderSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media in a supported recorder object 
        /// supports the given format
        /// </summary>
        /// <param name="Recorder"></param>
        /// <returns></returns>
        [DispId(2049)]
        [return: MarshalAs(UnmanagedType.VariantBool)]
        new bool IsCurrentMediaSupported(IDiscRecorder2 Recorder);

        /// <summary>
        /// Determines if the current media is reported as physically blank 
        /// by the drive
        /// </summary>
        [DispId(1792)]
        new bool MediaPhysicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Attempts to determine if the media is blank using heuristics 
        /// (mainly for DVD+RW and DVD-RAM media)
        /// </summary>
        [DispId(1793)]
        new bool MediaHeuristicallyBlank { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Supported media types
        /// </summary>
        [DispId(1794)]
        new MediaPhysicalType[] SupportedMediaTypes { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        // IDiscFormat2RawCD

        [DispId(0x200)]
        void PrepareMedia();

        [DispId(0x201)]
        void WriteMedia(IStream data);

        [DispId(0x202)]
        void WriteMedia2(IStream data, int streamLeadInSectors);

        [DispId(0x203)]
        void CancelWrite();

        [DispId(0x204)]
        void ReleaseMedia();

        [DispId(0x205)]
        void SetWriteSpeed(int requestedSectorsPerSecond, [MarshalAs(UnmanagedType.VariantBool)] bool rotationTypeIsPureCAV);

        [DispId(0x100)]
        IDiscRecorder2 Recorder { set; [return: MarshalAs(UnmanagedType.Interface)] get; }

        [DispId(0x102)]
        bool BufferUnderrunFreeDisabled { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x103)]
        int StartOfNextSession { get; }

        [DispId(0x104)]
        int LastPossibleStartOfLeadout { get; }

        [DispId(0x105)]
        MediaPhysicalType CurrentPhysicalMediaType { get; }

        [DispId(0x108)]
        String[] SupportedSectorTypes { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        [DispId(0x109)]
        Format2RawCDDataSectorType RequestedSectorType { set; get; }

        [DispId(0x10A)]
        String ClientName { set; [return: MarshalAs(UnmanagedType.BStr)] get; }

        [DispId(0x10B)]
        int RequestedWriteSpeed { get; }

        [DispId(0x10C)]
        bool RequestedRotationTypeIsPureCAV { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x10D)]
        int CurrentWriteSpeed { get; }

        [DispId(0x10E)]
        bool CurrentRotationTypeIsPureCAV { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x10F)]
        uint[] SupportedWriteSpeeds { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        [DispId(0x110)]
        IWriteSpeedDescriptor[] SupportedWriteSpeedDescriptors { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }
    }


    /// <summary>
    /// 
    /// </summary>
    [
        ComImport,
        Guid("27354136-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IWriteEngine2EventArgs
    {
        /// <summary>
        /// The starting logical block address for the current write operation.
        /// </summary>
        [DispId(0x100)]
        int StartLba { get; }

        /// <summary>
        /// The number of sectors being written for the current write operation.
        /// </summary>
        [DispId(0x101)]
        int SectorCount { get; }

        /// <summary>
        /// The last logical block address of data read for the current write operation.
        /// </summary>
        [DispId(0x102)]
        int LastReadLba { get; }

        /// <summary>
        /// The last logical block address of data written for the current write operation
        /// </summary>
        [DispId(0x103)]
        int LastWrittenLba { get; }

        /// <summary>
        /// The total bytes available in the system's cache buffer
        /// </summary>
        [DispId(0x106)]
        int TotalSystemBuffer { get; }

        /// <summary>
        /// The used bytes in the system's cache buffer
        /// </summary>
        [DispId(0x107)]
        int UsedSystemBuffer { get; }

        /// <summary>
        /// The free bytes in the system's cache buffer
        /// </summary>
        [DispId(0x108)]
        int FreeSystemBuffer { get; }
    }

    /// <summary>
    /// 
    /// </summary>
    [
        ComImport, 
        Guid("2735413D-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscFormat2DataEventArgs : IWriteEngine2EventArgs
    {
        // IWriteEngine2EventArgs
        /// <summary>
        /// The starting logical block address for the current write operation.
        /// </summary>
        [DispId(256)]
        new int StartLba { get; }

        /// <summary>
        /// The number of sectors being written for the current write operation.
        /// </summary>
        [DispId(257)]
        new int SectorCount { get; }

        /// <summary>
        /// The last logical block address of data read for the current write operation.
        /// </summary>
        [DispId(258)]
        new int LastReadLba { get; }

        /// <summary>
        /// The last logical block address of data written for the current write operation
        /// </summary>
        [DispId(259)]
        new int LastWrittenLba { get; }

        /// <summary>
        /// The total bytes available in the system's cache buffer
        /// </summary>
        [DispId(262)]
        new int TotalSystemBuffer { get; }

        /// <summary>
        /// The used bytes in the system's cache buffer
        /// </summary>
        [DispId(263)]
        new int UsedSystemBuffer { get; }

        /// <summary>
        /// The free bytes in the system's cache buffer
        /// </summary>
        [DispId(264)]
        new int FreeSystemBuffer { get; }

        // IDiscFormat2DataEventArgs
        /// <summary>
        /// The total elapsed time for the current write operation.
        /// </summary>
        [DispId(768)]
        int ElapsedTime { get; }

        /// <summary>
        /// The estimated time remaining for the write operation.
        /// </summary>
        [DispId(769)]
        int RemainingTime { get; }

        /// <summary>
        /// The estimated total time for the write operation.
        /// </summary>
        [DispId(770)]
        int TotalTime { get; }

        /// <summary>
        /// The current write action.
        /// </summary>
        [DispId(771)]
        Format2DataWriteAction CurrentAction { get; }
    }

    /// <summary>
    /// 
    /// </summary>
    [
        ComImport,
        Guid("2735413C-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface DDiscFormat2DataEvents
    {
        [DispId(0x200)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
        void Update([In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2Data sender,
                    [In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2DataEventArgs progress);
    }

    [
        ComImport,
        Guid("27354131-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface DDiscMaster2Events
    {
        [DispId(0x100)]
        [
            MethodImpl(MethodImplOptions.InternalCall,
                       MethodCodeType = MethodCodeType.Runtime)
        ]
        void NotifyDeviceAdded([In, MarshalAs(UnmanagedType.IDispatch)] IDiscMaster2 sender,
                               [In, MarshalAs(UnmanagedType.BStr)] String uniqueId);

        [DispId(0x101)]
        [
            MethodImpl(MethodImplOptions.InternalCall,
                       MethodCodeType = MethodCodeType.Runtime)
        ]
        void NotifyDeviceRemoved([In, MarshalAs(UnmanagedType.IDispatch)] IDiscMaster2 sender,
                                 [In, MarshalAs(UnmanagedType.BStr)] String uniqueId);
    }

    [
        ComImport,
        Guid("2735413A-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface DDiscFormat2EraseEvents
    {
        [DispId(0x200)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
        void Update([In, MarshalAs(UnmanagedType.IDispatch)] 
                    IDiscFormat2Erase sender,
                    int elapsedSeconds,
                    int estimatedTotalSeconds);
    }

    [
        ComImport,
        Guid("27354140-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscFormat2TrackAtOnceEventArgs : IWriteEngine2EventArgs
    {
        // IWriteEngine2EventArgs
        /// <summary>
        /// The starting logical block address for the current write operation.
        /// </summary>
        [DispId(256)]
        new int StartLba { get; }

        /// <summary>
        /// The number of sectors being written for the current write operation.
        /// </summary>
        [DispId(257)]
        new int SectorCount { get; }

        /// <summary>
        /// The last logical block address of data read for the current write operation.
        /// </summary>
        [DispId(258)]
        new int LastReadLba { get; }

        /// <summary>
        /// The last logical block address of data written for the current write operation
        /// </summary>
        [DispId(259)]
        new int LastWrittenLba { get; }

        /// <summary>
        /// The total bytes available in the system's cache buffer
        /// </summary>
        [DispId(262)]
        new int TotalSystemBuffer { get; }

        /// <summary>
        /// The used bytes in the system's cache buffer
        /// </summary>
        [DispId(263)]
        new int UsedSystemBuffer { get; }

        /// <summary>
        /// The free bytes in the system's cache buffer
        /// </summary>
        [DispId(264)]
        new int FreeSystemBuffer { get; }

        // IDiscFormat2TrackAtOnceEventArgs

        [DispId(0x300)]
        int CurrentTrackNumber { get; }

        [DispId(0x301)]
        Format2TAOWriteAction CurrentAction { get; }

        [DispId(0x302)]
        int ElapsedTime { get; }

        [DispId(0x303)]
        int RemainingTime { get; }
    }
    
    [
        ComImport,
        Guid("2735413F-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface DDiscFormat2TrackAtOnceEvents
    {
        [DispId(0x200)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
        void Update([In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2TrackAtOnce sender,
                    [In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2TrackAtOnceEventArgs progress);
    }


    [
        ComImport,
        Guid("27354143-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IDiscFormat2RawCDEventArgs : IWriteEngine2EventArgs
    {
        // IWriteEngine2EventArgs
        /// <summary>
        /// The starting logical block address for the current write operation.
        /// </summary>
        [DispId(256)]
        new int StartLba { get; }

        /// <summary>
        /// The number of sectors being written for the current write operation.
        /// </summary>
        [DispId(257)]
        new int SectorCount { get; }

        /// <summary>
        /// The last logical block address of data read for the current write operation.
        /// </summary>
        [DispId(258)]
        new int LastReadLba { get; }

        /// <summary>
        /// The last logical block address of data written for the current write operation
        /// </summary>
        [DispId(259)]
        new int LastWrittenLba { get; }

        /// <summary>
        /// The total bytes available in the system's cache buffer
        /// </summary>
        [DispId(262)]
        new int TotalSystemBuffer { get; }

        /// <summary>
        /// The used bytes in the system's cache buffer
        /// </summary>
        [DispId(263)]
        new int UsedSystemBuffer { get; }

        /// <summary>
        /// The free bytes in the system's cache buffer
        /// </summary>
        [DispId(264)]
        new int FreeSystemBuffer { get; }

        // IDiscFormat2RawCDEventArgs

        [DispId(0x301)]
        Format2RawCDWriteAction CurrentAction { get; }

        [DispId(0x302)]
        int ElapsedTime { get; }

        [DispId(0x303)]
        int RemainingTime { get; }
    }

    [
        ComImport,
        Guid("27354142-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface DDiscFormat2RawCDEvents
    {
        [DispId(0x200)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
        void Update([In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2RawCD sender,
                    [In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2RawCDEventArgs progress);
    }

    [
        ComImport,
        Guid("27354137-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
        // Provides notification of the progress of the WriteEngine2 writing.
    ]
    public interface DWriteEngine2Events
    {
        [DispId(0x100)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
        void Update([In, MarshalAs(UnmanagedType.IDispatch)] IWriteEngine2 sender,
                    [In, MarshalAs(UnmanagedType.IDispatch)] IWriteEngine2EventArgs progress);
    }

    [
        ComImport,
        Guid("D2FFD834-958B-426d-8470-2A13879C6A91"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IBurnVerification
    {
        [DispId(0x400)]
        BurnVerificationLevel BurnVerificationLevel { [return: MarshalAs(UnmanagedType.Interface)] get; set;}
    }

    [
        ComImport,
        Guid("27354135-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface IWriteEngine2
    {
        [DispId(0x200)]
        void WriteSection([In, MarshalAs(UnmanagedType.Interface)] IStream data, int startingBlockAddress, int numberOfBlocks);

        [DispId(0x201)]
        void CancelWrite();

        [DispId(0x100)]
        IDiscRecorder2Ex Recorder { [return: MarshalAs(UnmanagedType.Interface)] get; set; }

        [DispId(0x101)]
        bool UseStreamingWrite12 { [return: MarshalAs(UnmanagedType.VariantBool)] get; set; }

        [DispId(0x102)]
        int StartingSectorsPerSecond { get; set; }

        [DispId(0x103)]
        int EndingSectorsPerSecond { get; set; }

        [DispId(0x104)]
        int BytesPerSector { get; set;}

        [DispId(0x105)]
        bool WriteInProgress {  [return: MarshalAs(UnmanagedType.VariantBool)] get;}
    }

    #endregion Interfaces

    #region Events

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void IMAPIEventHandlerDelegate<TSender, TArgs>(TSender sender, TArgs args)
        where TSender : class;

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void IMAPIEventHandlerDelegate<TSender, TArg1, TArg2>(TSender sender, TArg1 arg1, TArg2 arg2)
        where TSender : class;

    #region DiscMaster2

    [
        ComVisible(false),
        ComEventInterface(typeof(DDiscMaster2Events),
                          typeof(DiscMaster2Events_EventProvider))
    ]
    public interface IDiscMaster2_Events
    {
        event IMAPIEventHandlerDelegate<IDiscMaster2, String> NotifyDeviceAdded;
        event IMAPIEventHandlerDelegate<IDiscMaster2, String> NotifyDeviceRemoved;
    }

    [
        TypeLibType(TypeLibTypeFlags.FHidden),
        ClassInterface(ClassInterfaceType.None)
    ]
    public sealed class DiscMaster2Events_SinkHelper : DDiscMaster2Events
    {
        public void NotifyDeviceAdded([In, MarshalAs(UnmanagedType.IDispatch)] IDiscMaster2 sender,
                                      [In, MarshalAs(UnmanagedType.BStr)] String uniqueId)
        {
            // Invoke the corresponding delegate(s)
            if (NotifyDeviceAddedDelegate != null)
            {
                NotifyDeviceAddedDelegate(sender, uniqueId);
            }
        }

        public void NotifyDeviceRemoved([In, MarshalAs(UnmanagedType.IDispatch)] IDiscMaster2 sender,
                                        [In, MarshalAs(UnmanagedType.BStr)] String uniqueId)
        {
            // Invoke the corresponding delegate(s)
            if (NotifyDeviceRemovedDelegate != null)
            {
                NotifyDeviceRemovedDelegate(sender, uniqueId);
            }
        }

        public int Cookie;

        public IMAPIEventHandlerDelegate<IDiscMaster2, String> NotifyDeviceAddedDelegate;
        public IMAPIEventHandlerDelegate<IDiscMaster2, String> NotifyDeviceRemovedDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class DiscMaster2Events_EventProvider : IDiscMaster2_Events, IDisposable
    {
        // Ctor
        public DiscMaster2Events_EventProvider(Object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    Guid eventsGuid = typeof(DDiscMaster2Events).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        // Event #1: NotifyDeviceAdded
        public event IMAPIEventHandlerDelegate<IDiscMaster2, String> NotifyDeviceAdded
        {
            add
            {
                lock (this)
                {
                    // Call Advise on a new sink helper, initializing its cookie
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new DiscMaster2Events_SinkHelper();
                    }

                    // Add the passed-in value to the sink helper's delegate list 
                    m_SinkHelper.NotifyDeviceAddedDelegate += value;

                    if (m_NeedsAdvise)
                    {
                        m_ConnectionPoint.Advise(m_SinkHelper, out m_SinkHelper.Cookie);
                        m_NeedsAdvise = false;
                    }
                }
            }

            remove
            {
                lock (this)
                {
                    if (m_SinkHelper == null ||
                        m_SinkHelper.NotifyDeviceAddedDelegate == null)
                    {
                        // TODO: ERROR, cannot remove w/o adding
                    }
                    else
                    {
                        m_SinkHelper.NotifyDeviceAddedDelegate -= value;
                        if (m_SinkHelper.NotifyDeviceAddedDelegate == null &&
                            m_SinkHelper.NotifyDeviceRemovedDelegate == null)
                        {
                            m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                            m_NeedsAdvise = true;
                        }
                    }
                }
            }
        }

        // Event #2: NotifyDeviceRemoved
        public event IMAPIEventHandlerDelegate<IDiscMaster2, String> NotifyDeviceRemoved
        {
            add
            {
                lock (this)
                {
                    // Call Advise on a new sink helper, initializing its cookie
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new DiscMaster2Events_SinkHelper();
                    }

                    // Add the passed-in value to the sink helper's delegate list
                    m_SinkHelper.NotifyDeviceRemovedDelegate += value;

                    if (m_NeedsAdvise)
                    {
                        m_ConnectionPoint.Advise(m_SinkHelper, out m_SinkHelper.Cookie);
                        m_NeedsAdvise = false;
                    }
                }
            }

            remove
            {
                lock (this)
                {
                    if (m_SinkHelper == null ||
                        m_SinkHelper.NotifyDeviceRemovedDelegate == null)
                    {
                        // TODO: ERROR, cannot remove w/o adding
                    }
                    else
                    {
                        m_SinkHelper.NotifyDeviceAddedDelegate -= value;
                        if (m_SinkHelper.NotifyDeviceAddedDelegate == null &&
                            m_SinkHelper.NotifyDeviceRemovedDelegate == null)
                        {
                            m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                            m_NeedsAdvise = true;
                        }
                    }
                }
            }
        }

        // Implementation of IDisposable.Dispose
        public void Dispose()
        {
            Cleanup();

            // It is no longer necessary for the GC to call the finalizer
            GC.SuppressFinalize(this);
        }

        // Finalizer
        ~DiscMaster2Events_EventProvider()
        {
            Cleanup();
        }

        // Called to unhook all events
        private void Cleanup()
        {
            lock (this)
            {
                if (!m_NeedsAdvise)
                {
                    m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                    m_NeedsAdvise = true;
                }
                m_SinkHelper = null;
                m_ConnectionPoint = null;
            }
        }

        private bool m_NeedsAdvise = true;
        private IConnectionPoint m_ConnectionPoint = null;
        private DiscMaster2Events_SinkHelper m_SinkHelper = null;
    }

    #endregion DiscMaster2

    #region DiscFormat2Data

    [
        ComEventInterface(typeof(DDiscFormat2DataEvents),
                          typeof(DiscFormat2Data_EventsProvider)),
        TypeLibType(TypeLibTypeFlags.FHidden)
    ]
    public interface IDiscFormat2Data_Events
    {
        event IMAPIEventHandlerDelegate<IDiscFormat2Data, IDiscFormat2DataEventArgs> Update;
    }

    [
        TypeLibType(TypeLibTypeFlags.FHidden),
        ClassInterface(ClassInterfaceType.None)
    ]
    public sealed class DiscFormat2Data_SinkHelper : DDiscFormat2DataEvents
    {
        public void Update([In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2Data sender,
                           [In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2DataEventArgs args)
        {
            UpdateDelegate(sender, args);
        }

        public int Cookie;

        public IMAPIEventHandlerDelegate<IDiscFormat2Data, IDiscFormat2DataEventArgs> UpdateDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class DiscFormat2Data_EventsProvider : IDiscFormat2Data_Events, IDisposable
    {
        public DiscFormat2Data_EventsProvider(Object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    Guid eventsGuid = typeof(DDiscFormat2DataEvents).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        public event IMAPIEventHandlerDelegate<IDiscFormat2Data, IDiscFormat2DataEventArgs> Update
        {
            add
            {
                lock (this)
                {
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new DiscFormat2Data_SinkHelper();
                    }

                    // Add the passed-in value to the sink helper's delegate list 
                    m_SinkHelper.UpdateDelegate += value;

                    // Call Advise on a new sink helper, initializing its cookie
                    if (m_NeedsAdvise)
                    {
                        m_ConnectionPoint.Advise(m_SinkHelper, out m_SinkHelper.Cookie);
                        m_NeedsAdvise = false;
                    }
                }
            }

            remove
            {
                lock (this)
                {
                    if (m_SinkHelper == null ||
                        m_SinkHelper.UpdateDelegate == null)    //.GetInvocationList().Length == 0)
                    {
                        // TODO: ERROR, cannot remove w/o adding
                    }
                    else
                    {
                        m_SinkHelper.UpdateDelegate -= value;
                        if (m_SinkHelper.UpdateDelegate == null)    //m_SinkHelper.UpdateDelegate.GetInvocationList().Length == 0)
                        {
                            m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                            m_NeedsAdvise = true;
                        }
                    }
                }
            }
        }

        // Implementation of IDisposable.Dispose
        public void Dispose()
        {
            Cleanup();

            // It is no longer necessary for the GC to call the finalizer
            GC.SuppressFinalize(this);
        }

        // Finalizer
        ~DiscFormat2Data_EventsProvider()
        {
            Cleanup();
        }

        // Called to unhook all events
        private void Cleanup()
        {
            lock (this)
            {
                if (!m_NeedsAdvise)
                {
                    m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                    m_NeedsAdvise = true;
                }
                m_SinkHelper = null;
                m_ConnectionPoint = null;
            }
        }

        private bool m_NeedsAdvise = true;
        private IConnectionPoint m_ConnectionPoint = null;
        private DiscFormat2Data_SinkHelper m_SinkHelper = null;
    }

    #endregion DiscFormat2Data

    #region DiscFormat2Erase

    [
        ComEventInterface(typeof(DDiscFormat2EraseEvents),
                          typeof(DiscFormat2Erase_EventsProvider)),
        TypeLibType(TypeLibTypeFlags.FHidden)
    ]
    public interface IDiscFormat2Erase_Events
    {
        event IMAPIEventHandlerDelegate<IDiscFormat2Erase, int, int> Update;
    }

    [
        TypeLibType(TypeLibTypeFlags.FHidden),
        ClassInterface(ClassInterfaceType.None)
    ]
    public sealed class DiscFormat2Erase_SinkHelper : DDiscFormat2EraseEvents
    {
        public void Update([In, MarshalAs(UnmanagedType.IDispatch)] 
                           IDiscFormat2Erase sender,
                           int elapsedSeconds,
                           int estimatedTotalSeconds)
        {
            UpdateDelegate(sender, elapsedSeconds, estimatedTotalSeconds);
        }

        public int Cookie;

        public IMAPIEventHandlerDelegate<IDiscFormat2Erase, int, int> UpdateDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class DiscFormat2Erase_EventsProvider : IDiscFormat2Erase_Events, IDisposable
    {
        public DiscFormat2Erase_EventsProvider(Object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    Guid eventsGuid = typeof(DDiscFormat2EraseEvents).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        public event IMAPIEventHandlerDelegate<IDiscFormat2Erase, int, int> Update
        {
            add
            {
                lock (this)
                {
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new DiscFormat2Erase_SinkHelper();
                    }

                    // Add the passed-in value to the sink helper's delegate list 
                    m_SinkHelper.UpdateDelegate += value;

                    // Call Advise on a new sink helper, initializing its cookie
                    if (m_NeedsAdvise)
                    {
                        m_ConnectionPoint.Advise(m_SinkHelper, out m_SinkHelper.Cookie);
                        m_NeedsAdvise = false;
                    }
                }
            }

            remove
            {
                lock (this)
                {
                    if (m_SinkHelper == null ||
                        m_SinkHelper.UpdateDelegate == null)    //.GetInvocationList().Length == 0)
                    {
                        // TODO: ERROR, cannot remove w/o adding
                    }
                    else
                    {
                        m_SinkHelper.UpdateDelegate -= value;
                        if (m_SinkHelper.UpdateDelegate == null)    //m_SinkHelper.UpdateDelegate.GetInvocationList().Length == 0)
                        {
                            m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                            m_NeedsAdvise = true;
                        }
                    }
                }
            }
        }

        // Implementation of IDisposable.Dispose
        public void Dispose()
        {
            Cleanup();

            // It is no longer necessary for the GC to call the finalizer
            GC.SuppressFinalize(this);
        }

        // Finalizer
        ~DiscFormat2Erase_EventsProvider()
        {
            Cleanup();
        }

        // Called to unhook all events
        private void Cleanup()
        {
            lock (this)
            {
                if (!m_NeedsAdvise)
                {
                    m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                    m_NeedsAdvise = true;
                }
                m_SinkHelper = null;
                m_ConnectionPoint = null;
            }
        }

        private bool m_NeedsAdvise = true;
        private IConnectionPoint m_ConnectionPoint = null;
        private DiscFormat2Erase_SinkHelper m_SinkHelper = null;
    }

    #endregion DiscFormat2Erase

    #region DiscFormat2TrackAtOnce

    [
        ComEventInterface(typeof(DDiscFormat2TrackAtOnceEvents),
                          typeof(DiscFormat2TrackAtOnce_EventsProvider)),
        TypeLibType(TypeLibTypeFlags.FHidden)
    ]
    public interface IDiscFormat2TrackAtOnce_Events
    {
        event IMAPIEventHandlerDelegate<IDiscFormat2TrackAtOnce, IDiscFormat2TrackAtOnceEventArgs> Update;
    }

    [
        TypeLibType(TypeLibTypeFlags.FHidden),
        ClassInterface(ClassInterfaceType.None)
    ]
    public sealed class DiscFormat2TrackAtOnce_SinkHelper : DDiscFormat2TrackAtOnceEvents
    {
        public void Update([In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2TrackAtOnce sender,
                           [In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2TrackAtOnceEventArgs args)
        {
            UpdateDelegate(sender, args);
        }

        public int Cookie;

        public IMAPIEventHandlerDelegate<IDiscFormat2TrackAtOnce, IDiscFormat2TrackAtOnceEventArgs> UpdateDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class DiscFormat2TrackAtOnce_EventsProvider : IDiscFormat2TrackAtOnce_Events, IDisposable
    {
        public DiscFormat2TrackAtOnce_EventsProvider(Object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    Guid eventsGuid = typeof(DDiscFormat2TrackAtOnceEvents).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        public event IMAPIEventHandlerDelegate<IDiscFormat2TrackAtOnce, IDiscFormat2TrackAtOnceEventArgs> Update
        {
            add
            {
                lock (this)
                {
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new DiscFormat2TrackAtOnce_SinkHelper();
                    }

                    // Add the passed-in value to the sink helper's delegate list 
                    m_SinkHelper.UpdateDelegate += value;

                    // Call Advise on a new sink helper, initializing its cookie
                    if (m_NeedsAdvise)
                    {
                        m_ConnectionPoint.Advise(m_SinkHelper, out m_SinkHelper.Cookie);
                        m_NeedsAdvise = false;
                    }
                }
            }

            remove
            {
                lock (this)
                {
                    if (m_SinkHelper == null ||
                        m_SinkHelper.UpdateDelegate == null)    //.GetInvocationList().Length == 0)
                    {
                        // TODO: ERROR, cannot remove w/o adding
                    }
                    else
                    {
                        m_SinkHelper.UpdateDelegate -= value;
                        if (m_SinkHelper.UpdateDelegate == null)    //m_SinkHelper.UpdateDelegate.GetInvocationList().Length == 0)
                        {
                            m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                            m_NeedsAdvise = true;
                        }
                    }
                }
            }
        }

        // Implementation of IDisposable.Dispose
        public void Dispose()
        {
            Cleanup();

            // It is no longer necessary for the GC to call the finalizer
            GC.SuppressFinalize(this);
        }

        // Finalizer
        ~DiscFormat2TrackAtOnce_EventsProvider()
        {
            Cleanup();
        }

        // Called to unhook all events
        private void Cleanup()
        {
            lock (this)
            {
                if (!m_NeedsAdvise)
                {
                    m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                    m_NeedsAdvise = true;
                }
                m_SinkHelper = null;
                m_ConnectionPoint = null;
            }
        }

        private bool m_NeedsAdvise = true;
        private IConnectionPoint m_ConnectionPoint = null;
        private DiscFormat2TrackAtOnce_SinkHelper m_SinkHelper = null;
    }

    #endregion DiscFormat2TrackAtOnce

    #region DiscFormat2RawCD

    [
        ComEventInterface(typeof(DDiscFormat2RawCDEvents),
                          typeof(DiscFormat2RawCD_EventsProvider)),
        TypeLibType(TypeLibTypeFlags.FHidden)
    ]
    public interface IDiscFormat2RawCD_Events
    {
        event IMAPIEventHandlerDelegate<IDiscFormat2RawCD, IDiscFormat2RawCDEventArgs> Update;
    }

    [
        TypeLibType(TypeLibTypeFlags.FHidden),
        ClassInterface(ClassInterfaceType.None)
    ]
    public sealed class DiscFormat2RawCD_SinkHelper : DDiscFormat2RawCDEvents
    {
        public void Update([In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2RawCD sender,
                           [In, MarshalAs(UnmanagedType.IDispatch)] IDiscFormat2RawCDEventArgs args)
        {
            UpdateDelegate(sender, args);
        }

        public int Cookie;

        public IMAPIEventHandlerDelegate<IDiscFormat2RawCD, IDiscFormat2RawCDEventArgs> UpdateDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class DiscFormat2RawCD_EventsProvider : IDiscFormat2RawCD_Events, IDisposable
    {
        public DiscFormat2RawCD_EventsProvider(Object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    Guid eventsGuid = typeof(DDiscFormat2RawCDEvents).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        public event IMAPIEventHandlerDelegate<IDiscFormat2RawCD, IDiscFormat2RawCDEventArgs> Update
        {
            add
            {
                lock (this)
                {
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new DiscFormat2RawCD_SinkHelper();
                    }

                    // Add the passed-in value to the sink helper's delegate list 
                    m_SinkHelper.UpdateDelegate += value;

                    // Call Advise on a new sink helper, initializing its cookie
                    if (m_NeedsAdvise)
                    {
                        m_ConnectionPoint.Advise(m_SinkHelper, out m_SinkHelper.Cookie);
                        m_NeedsAdvise = false;
                    }
                }
            }

            remove
            {
                lock (this)
                {
                    if (m_SinkHelper == null ||
                        m_SinkHelper.UpdateDelegate == null)    //.GetInvocationList().Length == 0)
                    {
                        // TODO: ERROR, cannot remove w/o adding
                    }
                    else
                    {
                        m_SinkHelper.UpdateDelegate -= value;
                        if (m_SinkHelper.UpdateDelegate == null)    //m_SinkHelper.UpdateDelegate.GetInvocationList().Length == 0)
                        {
                            m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                            m_NeedsAdvise = true;
                        }
                    }
                }
            }
        }

        // Implementation of IDisposable.Dispose
        public void Dispose()
        {
            Cleanup();

            // It is no longer necessary for the GC to call the finalizer
            GC.SuppressFinalize(this);
        }

        // Finalizer
        ~DiscFormat2RawCD_EventsProvider()
        {
            Cleanup();
        }

        // Called to unhook all events
        private void Cleanup()
        {
            lock (this)
            {
                if (!m_NeedsAdvise)
                {
                    m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                    m_NeedsAdvise = true;
                }
                m_SinkHelper = null;
                m_ConnectionPoint = null;
            }
        }

        private bool m_NeedsAdvise = true;
        private IConnectionPoint m_ConnectionPoint = null;
        private DiscFormat2RawCD_SinkHelper m_SinkHelper = null;
    }

    #endregion DiscFormat2RawCD

    #region WriteEngine2
    [
        ComEventInterface(typeof(DWriteEngine2Events),
                          typeof(WriteEngine2_EventsProvider)),
        TypeLibType(TypeLibTypeFlags.FHidden)
    ]
    public interface IWriteEngine2_Events
    {
        event IMAPIEventHandlerDelegate<IWriteEngine2, IWriteEngine2EventArgs> Update;
    }

    [
        TypeLibType(TypeLibTypeFlags.FHidden),
        ClassInterface(ClassInterfaceType.None)
    ]
    public sealed class WriteEngine2_SinkHelper : DWriteEngine2Events
    {
        public void Update([In, MarshalAs(UnmanagedType.IDispatch)] IWriteEngine2 sender,
                           [In, MarshalAs(UnmanagedType.IDispatch)] IWriteEngine2EventArgs args)
        {
            UpdateDelegate(sender, args);
        }

        public int Cookie;

        public IMAPIEventHandlerDelegate<IWriteEngine2, IWriteEngine2EventArgs> UpdateDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class WriteEngine2_EventsProvider : IWriteEngine2_Events, IDisposable
    {
        public WriteEngine2_EventsProvider(Object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    Guid eventsGuid = typeof(DWriteEngine2Events).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        public event IMAPIEventHandlerDelegate<IWriteEngine2, IWriteEngine2EventArgs> Update
        {
            add
            {
                lock (this)
                {
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new WriteEngine2_SinkHelper();
                    }

                    // Add the passed-in value to the sink helper's delegate list 
                    m_SinkHelper.UpdateDelegate += value;

                    // Call Advise on a new sink helper, initializing its cookie
                    if (m_NeedsAdvise)
                    {
                        m_ConnectionPoint.Advise(m_SinkHelper, out m_SinkHelper.Cookie);
                        m_NeedsAdvise = false;
                    }
                }
            }

            remove
            {
                lock (this)
                {
                    if (m_SinkHelper == null ||
                        m_SinkHelper.UpdateDelegate == null)    //.GetInvocationList().Length == 0)
                    {
                        // TODO: ERROR, cannot remove w/o adding
                    }
                    else
                    {
                        m_SinkHelper.UpdateDelegate -= value;
                        if (m_SinkHelper.UpdateDelegate == null)    //m_SinkHelper.UpdateDelegate.GetInvocationList().Length == 0)
                        {
                            m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                            m_NeedsAdvise = true;
                        }
                    }
                }
            }
        }

        // Implementation of IDisposable.Dispose
        public void Dispose()
        {
            Cleanup();

            // It is no longer necessary for the GC to call the finalizer
            GC.SuppressFinalize(this);
        }

        // Finalizer
        ~WriteEngine2_EventsProvider()
        {
            Cleanup();
        }

        // Called to unhook all events
        private void Cleanup()
        {
            lock (this)
            {
                if (!m_NeedsAdvise)
                {
                    m_ConnectionPoint.Unadvise(m_SinkHelper.Cookie);
                    m_NeedsAdvise = true;
                }
                m_SinkHelper = null;
                m_ConnectionPoint = null;
            }
        }

        private bool m_NeedsAdvise = true;
        private IConnectionPoint m_ConnectionPoint = null;
        private WriteEngine2_SinkHelper m_SinkHelper = null;
    }
    #endregion WriteEngine2

    #endregion Events

    #region CoClass

    /// <summary>
    /// 
    /// </summary>
    [
        ComImport,
        Guid("27354130-7F64-5B0F-8F00-5D77AFBE261E"),
        CoClass(typeof(MsftDiscMaster2Class))
    ]
    public interface MsftDiscMaster2 : IDiscMaster2, IDiscMaster2_Events
    {
    }

    [
        ComImport,
        Guid("27354132-7F64-5B0F-8F00-5D77AFBE261E"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
        CoClass(typeof(MsftDiscRecorder2XClass)), 
    ]
    public interface MsftDiscRecorder2Ex : IDiscRecorder2Ex
    {
    }

    [
        ComImport,
        Guid("27354133-7F64-5B0F-8F00-5D77AFBE261E"),
        CoClass(typeof(MsftDiscRecorder2XClass))
    ]
    public interface MsftDiscRecorder2 : IDiscRecorder2
    {
    }

    [
        ComImport,
        Guid("27354151-7F64-5B0F-8F00-5D77AFBE261E"),
        CoClass(typeof(MsftMultisessionSequentialClass))
    ]
    public interface MsftMultisessionSequential : IMultisessionSequential
    {
    }

    [
        ComImport,
        Guid("27354153-9F64-5B0F-8F00-5D77AFBE261E"),
        CoClass(typeof(MsftDiscFormat2DataClass))
    ]
    public interface MsftDiscFormat2Data : IDiscFormat2Data, IDiscFormat2Data_Events, IBurnVerification
    {
    }

    [
        ComImport,
        Guid("27354154-8F64-5B0F-8F00-5D77AFBE261E"),
        CoClass(typeof(MsftDiscFormat2TrackAtOnceClass))
    ]
    public interface MsftDiscFormat2TrackAtOnce : IDiscFormat2TrackAtOnce, IDiscFormat2TrackAtOnce_Events, IBurnVerification
    {
    }

    [
        ComImport,
        Guid("27354156-8F64-5B0F-8F00-5D77AFBE261E"),
        CoClass(typeof(MsftDiscFormat2EraseClass))
    ]
    public interface MsftDiscFormat2Erase : IDiscFormat2Erase, IDiscFormat2Erase_Events
    {
    }

    [
        ComImport,
        Guid("27354155-8F64-5B0F-8F00-5D77AFBE261E"),
        CoClass(typeof(MsftDiscFormat2RawCDClass))
    ]
    public interface MsftDiscFormat2RawCD : IDiscFormat2RawCD, IDiscFormat2RawCD_Events
    {
    }

    [
        ComImport,
        Guid("27354135-7F64-5B0F-8F00-5D77AFBE261E"),
        CoClass(typeof(MsftWriteEngine2Class))
    ]
    public interface MsftWriteEngine2 : IWriteEngine2, IWriteEngine2_Events
    {
    }
    #endregion CoClass

    #region Class

    /// <summary>
    /// 
    /// </summary>
    [
        ComImport,
        Guid("27354122-7F64-5B0F-8F00-5D77AFBE261E"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate)
    ]
    public class MsftMultisessionSequentialClass
    {
    }

    [
        ComImport,
        Guid("2735412E-7F64-5B0F-8F00-5D77AFBE261E"), 
        ComSourceInterfaces("DDiscMaster2Events\0"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate)
    ]
    public class MsftDiscMaster2Class
    {
    }

    [
        ComImport,
        Guid("2735412D-7F64-5B0F-8F00-5D77AFBE261E"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate)
    ]
    public class MsftDiscRecorder2Class
    {
    }

    [
        ComImport,
        Guid("2735412A-7F64-5B0F-8F00-5D77AFBE261E"),
        ComSourceInterfaces("DDiscFormat2DataEvents\0"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class MsftDiscFormat2DataClass
    {
    }

    [
        ComImport,
        Guid("2735412B-7F64-5B0F-8F00-5D77AFBE261E"),
        ComSourceInterfaces("DDiscFormat2EraseEvents\0"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class MsftDiscFormat2EraseClass
    {
    }

    [
        ComImport,
        Guid("27354129-7F64-5B0F-8F00-5D77AFBE261E"),
        ComSourceInterfaces("DDiscFormat2TrackAtOnceEvents\0"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class MsftDiscFormat2TrackAtOnceClass
    {
    }

    [
        ComImport,
        Guid("27354128-7F64-5B0F-8F00-5D77AFBE261E"),
        ComSourceInterfaces("DDiscFormat2RawCDEvents\0"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class MsftDiscFormat2RawCDClass
    {
    }

    [
        ComImport,
        Guid("2735412C-7F64-5B0F-8F00-5D77AFBE261E"),
        ComSourceInterfaces("DWriteEngine2Events\0"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class MsftWriteEngine2Class
    {
    }

    #endregion Class

    #region Enumerators

    [Serializable]
    public enum FeaturePageType :int
    {
        ProfileList                  = 0,
        Core                         = 0x01,
        Morphing                     = 0x02,
        RemovableMedium              = 0x03,
        Write_Protect                = 0x04,
        RandomlyReadable             = 0x10,
        CD_Multiread                 = 0x1D,
        CD_Read                      = 0x1E,
        DVD_Read                     = 0x1F,
        RandomlyWritable             = 0x20,
        IncrementalStreamingWritable = 0x21,
        SectorErasable               = 0x22,
        Formattable                  = 0x23,
        HardwareDefectManagement     = 0x24,
        WriteOnce                    = 0x25,
        RestrictedOverwrite          = 0x26,
        CDRW_CAV_Write               = 0x27,
        MRW                          = 0x28,
        EnhancedDefectReporting      = 0x29,
        DVDPlusRW                    = 0x2A,
        DVDPlusR                     = 0x2B,
        RigidRestrictedOverwrite     = 0x2C,
        CD_TrackAtOnce               = 0x2D,
        CD_Mastering                 = 0x2E,
        DVDDash_Write                = 0x2F,
        LayerJumpRecording           = 0x33,
        CDRW_MediaWriteSupport       = 0x37,
        BD_PseudoOverwrite           = 0x38,
        DVDPlusRWDualLayer           = 0x3a,
        DVDPlusRDualLayer            = 0x3b,
        BD_Read                      = 0x40,
        BD_Write                     = 0x41,
        HDDVD_Read                   = 0x50,
        HDDVD_Write                  = 0x51,
        PowerManagement              = 0x100,
        Smart                        = 0x101,
        EmbeddedChanger              = 0x102,
        CD_AnalogPlay                = 0x103,
        MicrocodeUpdate              = 0x104,
        Timeout                      = 0x105,
        DVD_CSS                      = 0x106,
        RealTimeStreaming            = 0x107,
        LogicalUnitSerialNumber      = 0x108,
        MediaSerialNumber            = 0x109,
        DiscControlBlocks            = 0x10A,
        DVD_CPRM                     = 0x10B,
        FirmwareInformation          = 0x10C,
        AACS                         = 0x10D,
        VCPS                         = 0x110,
    }

    [Serializable]
    public enum ModePageType
    {
        ReadWriteErrorRecovery  = 1,
        MRW                     = 3,
        WriteParameters         = 5,
        Caching                 = 8,
        PowerCondition          = 26,
        InformationalExceptions = 28,
        TimeoutAnd_Protect      = 29,
        LegacyCapabilities      = 42,
    }

    [Serializable]
    public enum ModePageRequestType
    {
        CurrentValues   = 0,
        ChangableValues = 1,
        DefaultValues   = 2,
        SavedValues     = 3,
    }

    [Serializable]
    public enum ReadTrackAddressType
    {
        LBA     = 0,
        Track   = 1,
        Session = 2,
    }

    [Serializable]
    public enum MediaPhysicalType
    {
        Unknown            = 0,
        CDROM              = 1,
        CDR                = 2,
        CDRW               = 3,
        DVDROM             = 4,
        DVDRAM             = 5,
        DVDPlusR           = 6,
        DVDPlusRW          = 7,
        DVDPlusRDualLayer  = 8,
        DVDDashR           = 9,
        DVDDashRW          = 10,
        DVDDashRDualLayer  = 11,
        Disk               = 12,
        DVDPlusRWDualLayer = 13,
        HDDVDROM           = 14,
        HDDVDR             = 15,
        HDDVDRAM           = 16,
        BDROM              = 17,
        BDR                = 18,
        BDRE               = 19,
    }

    [Flags]
    [Serializable]
    public enum MediaWriteProtectState
    {
        UntilPowerdown         = 0x00001,
        ByCartridge            = 0x00002,
        ByMediaSpecificReason  = 0x00004,
        BySoftwareWriteProtect = 0x00008,
        ByDiscControlBlock     = 0x00010,
        ReadOnlyMedia          = 0x04000,
    }

    [Serializable]
    public enum ProfileType
    {
        Invalid                = 0x0000,
        NonRemovableDisk       = 0x0001,
        RemovableDisk          = 0x0002,
        MO_Erasable            = 0x0003,
        MO_WriteOnce           = 0x0004,
        AS_MO                  = 0x0005,
        // Reserved           0x0006 - 0x0007
        CDROM                  = 0x0008,
        CDRecordable           = 0x0009,
        CDRewritable           = 0x000A,
        // Reserved           0x000B - 0x000F
        DVDROM                 = 0x0010,
        DVDDashRecordable      = 0x0011,
        DVDRAM                 = 0x0012,
        DVDDashRewritable      = 0x0013,
        DVDDashRWSequential    = 0x0014,
        DVDDashRDualSequential = 0x0015,
        DVDDashRDualLayerJump  = 0x0016,
        // Reserved           0x0017 - 0x0019
        DVDPlusRW              = 0x001A,
        DVDPlusR               = 0x001B,
        // Reserved           0x001B - 001F
        DDCDROM                = 0x0020,
        DDCDRecordable         = 0x0021,
        DDCDRewritable         = 0x0022,
        // Reserved           0x0023 - 0x0029
        DVDPlusRWDualLayer     = 0x002A,
        DVDPlusRDualLayer      = 0x002B,
        // Reserved           0x002C - 0x003F
        BDROM                  = 0x0040,
        BDRSequential          = 0x0041,
        BDRRandomRecording     = 0x0042,
        BDRewritable           = 0x0043,
        // Reserved           0x0044 - 0x004F
        HDDVDROM               = 0x0050,
        HDDVDRecordable        = 0x0051,
        HDDVDRAM               = 0x0052,
        HDDVDRW                = 0x0053,
        // Reserved           0x0054 - 0xFFFE
        NonStandard            = 0xFFFF
    }

    [Flags]
    [Serializable]
    public enum Format2DataMediaState
    {
        [TypeLibVar(TypeLibVarFlags.FHidden)]
        Unknown           = 0x0000,
        OverwriteOnly     = 0x0001,
        RandomlyWritable  = 0x0001,  // deprecated, need to replace usage
        Blank             = 0x0002,
        Appendable        = 0x0004,
        FinalSession      = 0x0008,
        InformationalMask = 0x000F,
        Damaged           = 0x0400,
        EraseRequired     = 0x0800,
        NonEmptySession   = 0x1000,
        WriteProtected    = 0x2000,
        Finalized         = 0x4000,
        UnsupportedMedia  = 0x8000,
        UnsupportedMask   = 0xFC00,
    }

    [Serializable]
    public enum Format2DataWriteAction
    {
        ValidatingMedia      = 0,
        FormattingMedia      = 1,
        InitializingHardware = 2,
        CalibratingPower     = 3,
        WritingData          = 4,
        Finalization         = 5,
        Completed            = 6,
    }
    
    [Serializable]
    public enum Format2TAOWriteAction
    {
        [TypeLibVar(TypeLibVarFlags.FHidden)]
        Unknown   = 0,
        Preparing = 1,
        Writing   = 2,
        Finishing = 3
    }

    [Serializable]
    public enum Format2RawCDDataSectorType
    {
        SubcodePQOnly            = 0x0001,
        SubcodeIsCooked          = 0x0002,
        SubcodeIsRaw             = 0x0003,
    }

    [Serializable]
    public enum Format2RawCDWriteAction
    {
        Unknown   = 0x0000,
        Preparing = 0x0001,
        Writing   = 0x0002,
        Finishing = 0x0003,
    }

    [Serializable]
    public enum BurnVerificationLevel
    {
        None   = 0x0000,
        Quick  = 0x0001,
        Full   = 0x0002,
    }

    #endregion

}
