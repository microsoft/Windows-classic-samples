/*-----------------------------------------------------------------------*
 * This file is part of the Microsoft IMAPIv2 Code Samples.              *
 *                                                                       *
 * Copyright (C) Microsoft Corporation.  All rights reserved.            *
 *                                                                       *
 * This source code is intended only as a supplement to Microsoft IMAPI2 *
 * help and/or on-line documentation.  See these other materials for     *
 * detailed information regarding Microsoft code samples.                *
 *                                                                       *
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY  *
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR *
 * PURPOSE.                                                              *
 *-----------------------------------------------------------------------*/

namespace IMAPI2.Interop
{
    using System;
    using System.IO;
    using System.Threading;
    using System.Collections;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    using System.Runtime.CompilerServices;

    // Interfaces
    [TypeLibType(TypeLibTypeFlags.FDual         | 
                 TypeLibTypeFlags.FDispatchable | 
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("27354130-7F64-5B0F-8F00-5D77AFBE261E"), ComImport]
    public interface IDiscMaster2
    {
        /// <summary>
        /// Enumerates the list of CD/DVD devices on the system
        /// </summary>
        /// <returns></returns>
        [TypeLibFunc(65)]
        [DispId(-4)]
        IEnumerator GetEnumerator();

        /// <summary>
        /// Gets a single recorder's ID
        /// </summary>
        /// <param name="index">Zero based index</param>
        /// <returns>Recorder's unique id</returns>
        [DispId(0)]
        String this[int index] { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// The current number of recorders in the system.
        /// </summary>
        [DispId(1)]
        int Count { get; }

        /// <summary>
        /// Whether IMAPI is running in an environment with optical 
        /// devices and permission to access them.
        /// </summary>
        [DispId(2)]
        bool IsSupportedEnvironment { [return: MarshalAs(UnmanagedType.VariantBool)] get; }
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("27354133-7F64-5B0F-8F00-5D77AFBE261E"), ComImport]
    public interface IDiscRecorder2
    {
        /// <summary>
        /// Ejects the media (if any) and opens the tray
        /// </summary>
        [DispId(256)]
        void EjectMedia();

        /// <summary>
        /// Close the media tray and load any media in the tray.
        /// </summary>
        [DispId(257)]
        void CloseTray();

        /// <summary>
        /// Acquires exclusive access to device. 
        /// May be called multiple times.
        /// </summary>
        /// <param name="force"></param>
        /// <param name="clientName">App Id</param>
        [DispId(258)]
        void AcquireExclusiveAccess([MarshalAs(UnmanagedType.VariantBool)] bool force, [MarshalAs(UnmanagedType.BStr)] String clientName);

        /// <summary>
        /// Releases exclusive access to device. 
        /// Call once per AcquireExclusiveAccess().
        /// </summary>
        [DispId(259)]
        void ReleaseExclusiveAccess();

        /// <summary>
        /// Disables Media Change Notification (MCN).
        /// </summary>
        [DispId(260)]
        void DisableMcn();

        /// <summary>
        /// Re-enables Media Change Notification after a call to DisableMcn()
        /// </summary>
        [DispId(261)]
        void EnableMcn();

        /// <summary>
        /// Initialize the recorder, opening a handle to the specified recorder.
        /// </summary>
        /// <param name="recorderUniqueId"></param>
        [DispId(262)]
        void InitializeDiscRecorder([MarshalAs(UnmanagedType.BStr)] String recorderUniqueId);

        /// <summary>
        /// The unique ID used to initialize the recorder.
        /// </summary>
        [DispId(0)]
        String ActiveDiscRecorder { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// The vendor ID in the device's INQUIRY data.
        /// </summary>
        [DispId(513)]
        String VendorId { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// The Product ID in the device's INQUIRY data.
        /// </summary>
        [DispId(514)]
        String ProductId { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// The Product Revision in the device's INQUIRY data.
        /// </summary>
        [DispId(515)]
        String ProductRevision { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Get the unique volume name (this is not a drive letter).
        /// </summary>
        [DispId(516)]
        String VolumeName { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Drive letters and NTFS mount points to access the recorder.
        /// </summary>
        [DispId(517)]
        String[] VolumePathNames { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// One of the volume names associated with the recorder.
        /// </summary>
        [DispId(518)]
        bool DeviceCanLoadMedia { [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Gets the legacy 'device number' associated with the recorder. 
        /// This number is not guaranteed to be static.
        /// </summary>
        [DispId(519)]
        int LegacyDeviceNumber { get; }

        /// <summary>
        /// Gets a list of all feature pages supported by the device
        /// </summary>
        [DispId(520)]
        int[] SupportedFeaturePages { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// Gets a list of all feature pages with 'current' bit set to true
        /// </summary>
        [DispId(521)]
        int[] CurrentFeaturePages { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// Gets a list of all profiles supported by the device
        /// </summary>
        [DispId(522)]
        int[] SupportedProfiles { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// Gets a list of all profiles with 'currentP' bit set to true
        /// </summary>
        [DispId(523)]
        int[] CurrentProfiles { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// Gets a list of all MODE PAGES supported by the device
        /// </summary>
        [DispId(524)]
        int[] SupportedModePages { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        /// <summary>
        /// Queries the device to determine who, if anyone, has acquired exclusive access
        /// </summary>
        [DispId(525)]
        String ExclusiveAccessOwner { [return: MarshalAs(UnmanagedType.BStr)] get; }
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("27354132-7F64-5B0F-8F00-5D77AFBE261E")]
    public interface IDiscRecorder2Ex
    {
        /// <summary>
        /// Send a command to the device that does not transfer any data
        /// </summary>
        /// <param name="Cdb"></param>
        /// <param name="CdbSize"></param>
        /// <param name="SenseBuffer"></param>
        /// <param name="Timeout"></param>
        void SendCommandNoData([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 1)] 
                               Byte[] cdb,
                               uint cdbSize,
                               [MarshalAs(UnmanagedType.LPArray, SizeConst = 18)] 
                               Byte[] senseBuffer,
                               uint timeout);

        /// <summary>
        /// Send a command to the device that requires data sent to the device
        /// </summary>
        /// <param name="Cdb"></param>
        /// <param name="CdbSize"></param>
        /// <param name="SenseBuffer"></param>
        /// <param name="Timeout"></param>
        /// <param name="Buffer"></param>
        /// <param name="BufferSize"></param>
        void SendCommandSendDataToDevice([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 1)] 
                                         Byte[] cdb,
                                         uint cdbSize,
                                         [MarshalAs(UnmanagedType.LPArray, SizeConst = 18)] 
                                         Byte[] senseBuffer,
                                         uint timeout,
                                         [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 5)] 
                                         Byte[] buffer,
                                         uint bufferSize);
        
        /// <summary>
        /// Send a command to the device that requests data from the device
        /// </summary>
        /// <param name="Cdb"></param>
        /// <param name="CdbSize"></param>
        /// <param name="SenseBuffer"></param>
        /// <param name="Timeout"></param>
        /// <param name="Buffer"></param>
        /// <param name="BufferSize"></param>
        /// <param name="BufferFetched"></param>
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
        
        /// <summary>
        /// Read a DVD Structure from the media
        /// </summary>
        /// <param name="format"></param>
        /// <param name="address"></param>
        /// <param name="layer"></param>
        /// <param name="agid"></param>
        /// <param name="data"></param>
        /// <param name="Count"></param>
        void ReadDvdStructure(uint format,
                              uint address,
                              uint layer,
                              uint agid,
                              out IntPtr dvdStructurePtr,
                              ref uint count);
        
        /// <summary>
        /// Read a DVD Structure from the media
        /// </summary>
        /// <param name="format"></param>
        /// <param name="data"></param>
        /// <param name="Count"></param>
        void SendDvdStructure(uint format,
                              [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 2)] 
                              Byte[] dvdStructure,
                              uint count);
        
        /// <summary>
        /// Get the full adapter descriptor (via IOCTL_STORAGE_QUERY_PROPERTY).
        /// </summary>
        /// <param name="data"></param>
        /// <param name="byteSize"></param>
        void GetAdapterDescriptor(out IntPtr adapterDescriptorPtr,
                                  ref uint byteSize);
        
        /// <summary>
        /// Get the full device descriptor (via IOCTL_STORAGE_QUERY_PROPERTY).
        /// </summary>
        /// <param name="data"></param>
        /// <param name="byteSize"></param>
        void GetDeviceDescriptor(out IntPtr deviceDescriptorPtr,
                                 ref uint byteSize);
        
        /// <summary>
        /// Gets data from a READ_DISC_INFORMATION command
        /// </summary>
        /// <param name="discInformation"></param>
        /// <param name="byteSize"></param>
        void GetDiscInformation(out IntPtr discInformationPtr,
                                ref uint byteSize);
        
        /// <summary>
        /// Gets data from a READ_TRACK_INFORMATION command
        /// </summary>
        /// <param name="address"></param>
        /// <param name="addressType"></param>
        /// <param name="trackInformation"></param>
        /// <param name="byteSize"></param>
        void GetTrackInformation(uint address,
                                 IMAPI_READ_TRACK_ADDRESS_TYPE addressType,
                                 out IntPtr trackInformationPtr,
                                 ref uint byteSize);
        
        /// <summary>
        /// Gets a feature's data from a GET_CONFIGURATION command
        /// </summary>
        /// <param name="requestedFeature"></param>
        /// <param name="currentFeatureOnly"></param>
        /// <param name="featureData"></param>
        /// <param name="byteSize"></param>
        void GetFeaturePage(IMAPI_FEATURE_PAGE_TYPE requestedFeature,
                            [MarshalAs(UnmanagedType.U1)] 
                            bool currentFeatureOnly,
                            out IntPtr featureDataPtr,
                            ref uint byteSize);
        
        /// <summary>
        /// Gets data from a MODE_SENSE10 command
        /// </summary>
        /// <param name="requestedModePage"></param>
        /// <param name="requestType"></param>
        /// <param name="modePageData"></param>
        /// <param name="byteSize"></param>
        void GetModePage(IMAPI_MODE_PAGE_TYPE requestedModePage,
                         IMAPI_MODE_PAGE_REQUEST_TYPE requestType,
                         out IntPtr modePageDataPtr,
                         ref uint byteSize);
        
        /// <summary>
        /// Sets mode page data using MODE_SELECT10 command
        /// </summary>
        /// <param name="requestType"></param>
        /// <param name="data"></param>
        /// <param name="byteSize"></param>
        void SetModePage(IMAPI_MODE_PAGE_REQUEST_TYPE requestType,
                         [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U1, SizeParamIndex = 2)] 
                         Byte[] modePage,
                         uint byteSize);
        
        /// <summary>
        /// Gets a list of all feature pages supported by the device
        /// </summary>
        /// <param name="currentFeatureOnly"></param>
        /// <param name="featureData"></param>
        /// <param name="byteSize"></param>
        void GetSupportedFeaturePages([MarshalAs(UnmanagedType.U1)] 
                                      bool currentFeatureOnly,
                                      out IntPtr featureDataPtr,
                                      ref uint byteSize);
        
        /// <summary>
        /// Gets a list of all PROFILES supported by the device
        /// </summary>
        /// <param name="currentOnly"></param>
        /// <param name="profileTypes"></param>
        /// <param name="validProfiles"></param>
        void GetSupportedProfiles([MarshalAs(UnmanagedType.U1)] 
                                  bool currentOnly,
                                  out IntPtr profileTypesPtr,
                                  ref uint validProfiles);
        
        /// <summary>
        /// Gets a list of all MODE PAGES supported by the device
        /// </summary>
        /// <param name="requestType"></param>
        /// <param name="modePageTypes"></param>
        /// <param name="validPages"></param>
        void GetSupportedModePages(IMAPI_MODE_PAGE_REQUEST_TYPE requestType,
                                   out IntPtr modePageTypesPtr,
                                   ref uint validPages);
        
        /// <summary>
        /// The byte alignment requirement mask for this device.
        /// </summary>
        /// <returns></returns>
        uint GetByteAlignmentMask();

        /// <summary>
        /// The maximum non-page-aligned transfer size for this device.
        /// </summary>
        /// <returns></returns>
        uint GetMaximumNonPageAlignedTransferSize();

        /// <summary>
        /// The maximum non-page-aligned transfer size for this device.
        /// </summary>
        /// <returns></returns>
        uint GetMaximumPageAlignedTransferSize();
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("27354153-9F64-5B0F-8F00-5D77AFBE261E")]
    public interface IDiscFormat2Data
    {
        // IDiscFormat2
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
        int[] SupportedMediaTypes { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

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
        IMAPI_FORMAT2_DATA_MEDIA_STATE CurrentMediaStatus { get; }

        /// <summary>
        /// The write protection state of the current media.
        /// </summary>
        [DispId(263)]
        IMAPI_MEDIA_WRITE_PROTECT_STATE WriteProtectStatus { get; }

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
        IMAPI_MEDIA_PHYSICAL_TYPE CurrentPhysicalMediaType { get; }

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
        Array SupportedWriteSpeedDescriptors { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

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
        Array MultisessionInterfaces { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

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

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FE1-975B-59BE-A960-9A2A262853A5")]
    public interface IFileSystemImage
    {
        /// <summary>
        /// Root directory item
        /// </summary>
        [DispId(0)]
        IFsiDirectoryItem Root { get; }

        /// <summary>
        /// Disc start block for the image
        /// </summary>
        [DispId(1)]
        int SessionStartBlock { get; set; }

        /// <summary>
        /// Maximum number of blocks available for the image
        /// </summary>
        [DispId(2)]
        int FreeMediaBlocks { get; set; }

        /// <summary>
        /// Set maximum number of blocks available based on the recorder supported discs. 
        /// 0 for unknown maximum may be set.
        /// </summary>
        /// <param name="discRecorder"></param>
        [DispId(36)]
        void SetMaxMediaBlocksFromDevice(IDiscRecorder2 discRecorder);

        /// <summary>
        /// Number of blocks in use
        /// </summary>
        [DispId(3)]
        int UsedBlocks { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(4)]
        String VolumeName { set; [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Imported Volume name
        /// </summary>
        [DispId(5)]
        String ImportedVolumeName { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Boot image and boot options
        /// </summary>
        [DispId(6)]
        IBootOptions BootImageOptions { get; set; }

        /// <summary>
        /// Number of files in the image
        /// </summary>
        [DispId(7)]
        int FileCount { get; }

        /// <summary>
        /// Number of directories in the image
        /// </summary>
        [DispId(8)]
        int DirectoryCount { get; }

        /// <summary>
        /// Temp directory for stash files
        /// </summary>
        [DispId(9)]
        String WorkingDirectory { set; [return: MarshalAs(UnmanagedType.BStr)] get;}

        /// <summary>
        /// Change point identifier
        /// </summary>
        [DispId(10)]
        int ChangePoint { get; }

        /// <summary>
        /// Strict file system compliance option
        /// </summary>
        [DispId(11)]
        bool StrictFileSystemCompliance { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// If true, indicates restricted character set is being used for file and directory names
        /// </summary>
        [DispId(12)]
        bool UseRestrictedCharacterSet { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// File systems to create
        /// </summary>
        [DispId(13)]
        FsiFileSystems FileSystemsToCreate { get; set; }

        /// <summary>
        /// File systems supported
        /// </summary>
        [DispId(14)]
        FsiFileSystems FileSystemsSupported { get; }

        /// <summary>
        /// UDF revision
        /// </summary>
        [DispId(37)]
        int UDFRevision { set; get; }

        /// <summary>
        /// UDF revision(s) supported
        /// </summary>
        [DispId(31)]
        Array UDFRevisionsSupported { get; }

        /// <summary>
        /// Select filesystem types and image size based on the current media
        /// </summary>
        /// <param name="discRecorder"></param>
        [DispId(32)]
        void ChooseImageDefaults(IDiscRecorder2 discRecorder);

        /// <summary>
        /// Select filesystem types and image size based on the media type
        /// </summary>
        /// <param name="value"></param>
        [DispId(33)]
        void ChooseImageDefaultsForMediaType(IMAPI_MEDIA_PHYSICAL_TYPE value);

        /// <summary>
        /// ISO compatibility level to create
        /// </summary>
        [DispId(34)]
        int ISO9660InterchangeLevel { set; get; }

        /// <summary>
        /// ISO compatibility level(s) supported
        /// </summary>
        [DispId(38)]
        Array ISO9660InterchangeLevelsSupported { get; }

        /// <summary>
        /// Create result image stream
        /// </summary>
        /// <returns></returns>
        [DispId(15)]
        IFileSystemImageResult CreateResultImage();

        /// <summary>
        /// Check for existance an item in the file system
        /// </summary>
        /// <param name="FullPath"></param>
        /// <returns></returns>
        [DispId(16)]
        FsiItemType Exists(string FullPath);

        /// <summary>
        /// Return a string useful for identifying the current disc
        /// </summary>
        /// <returns></returns>
        [DispId(18)]
        string CalculateDiscIdentifier();

        /// <summary>
        /// Identify file systems on a given disc
        /// </summary>
        /// <param name="discRecorder"></param>
        /// <returns></returns>
        [DispId(19)]
        FsiFileSystems IdentifyFileSystemsOnDisc(IDiscRecorder2 discRecorder);

        /// <summary>
        /// Identify which of the specified file systems would be imported by default
        /// </summary>
        /// <param name="fileSystems"></param>
        /// <returns></returns>
        [DispId(20)]
        FsiFileSystems GetDefaultFileSystemForImport(FsiFileSystems fileSystems);

        /// <summary>
        /// Import the default file system on the current disc
        /// </summary>
        /// <returns></returns>
        [DispId(21)]
        FsiFileSystems ImportFileSystem();

        /// <summary>
        /// Import a specific file system on the current disc
        /// </summary>
        /// <param name="fileSystemToUse"></param>
        [DispId(22)]
        void ImportSpecificFileSystem(FsiFileSystems fileSystemToUse);

        /// <summary>
        /// Roll back to the specified change point
        /// </summary>
        /// <param name="ChangePoint"></param>
        [DispId(23)]
        void RollbackToChangePoint(int ChangePoint);

        /// <summary>
        /// Lock in changes
        /// </summary>
        [DispId(24)]
        void LockInChangePoint();

        /// <summary>
        /// Create a directory item with the specified name
        /// </summary>
        /// <param name="Name"></param>
        /// <returns></returns>
        [DispId(25)]
        IFsiDirectoryItem CreateDirectoryItem(string Name);

        /// <summary>
        /// Create a file item with the specified name
        /// </summary>
        /// <param name="Name"></param>
        /// <returns></returns>
        [DispId(26)]
        IFsiFileItem CreateFileItem(string Name);

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(27)]
        String VolumeNameUDF { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(28)]
        String VolumeNameJoliet { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(29)]
        String VolumeNameISO9660 { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Indicates whether or not IMAPI should stage the filesystem before the burn.
        /// Set to false to force IMAPI to not stage the filesystem prior to the burn.
        /// </summary>
        [DispId(30)]
        bool StageFiles { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// available multi-session interfaces.
        /// </summary>
        [DispId(40)]
        Array MultisessionInterfaces { get; set; }
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FD8-975B-59BE-A960-9A2A262853A5")]
    public interface IFileSystemImageResult
    {
        /// <summary>
        /// Image stream
        /// </summary>
        [DispId(1)]
        IStream ImageStream { get; }

        /// <summary>
        /// Progress item block mapping collection
        /// </summary>
        [DispId(2)]
        IProgressItems ProgressItems { get; }

        /// <summary>
        /// Number of blocks in the result image
        /// </summary>
        [DispId(3)]
        int TotalBlocks { get; }

        /// <summary>
        /// Number of bytes in a block
        /// </summary>
        [DispId(4)]
        int BlockSize { get; }

        /// <summary>
        /// Disc Identifier (for identifing imported session of multi-session disc)
        /// </summary>
        [DispId(5)]
        String DiscId { [return: MarshalAs(UnmanagedType.BStr)] get; }
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FD7-975B-59BE-A960-9A2A262853A5")]
    public interface IProgressItems
    {
        /// <summary>
        /// Get an enumerator for the collection
        /// </summary>
        /// <returns></returns>
        [DispId(-4)]
        [TypeLibFunc(65)]
        IEnumerator GetEnumerator();

        /// <summary>
        /// Find the block mapping from the specified index
        /// </summary>
        /// <param name="Index"></param>
        /// <returns></returns>
        [DispId(0)]
        IProgressItem this[int Index] { get; }

        /// <summary>
        /// Number of items in the collection
        /// </summary>
        [DispId(1)]
        int Count { get; }

        /// <summary>
        /// Find the block mapping from the specified block
        /// </summary>
        /// <param name="block"></param>
        /// <returns></returns>
        [DispId(2)]
        IProgressItem ProgressItemFromBlock(uint block);

        /// <summary>
        /// Find the block mapping from the specified item description
        /// </summary>
        /// <param name="Description"></param>
        /// <returns></returns>
        [DispId(3)]
        IProgressItem ProgressItemFromDescription(string Description);

        /// <summary>
        /// Get a non-variant enumerator
        /// </summary>
        [DispId(4)]
        IEnumProgressItems EnumProgressItems { get; }
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FD4-975B-59BE-A960-9A2A262853A5")]
    public interface IBootOptions
    {
        /// <summary>
        /// Get boot image data stream
        /// </summary>
        [DispId(1)]
        IStream BootImage { get; }

        /// <summary>
        /// Get boot manufacturer
        /// </summary>
        [DispId(2)]
        String Manufacturer { set; [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Get boot platform identifier
        /// </summary>
        [DispId(3)]
        PlatformId PlatformId { get; set; }

        /// <summary>
        /// Get boot emulation type
        /// </summary>
        [DispId(4)]
        EmulationType Emulation { get; set; }

        /// <summary>
        /// Get boot image size
        /// </summary>
        [DispId(5)]
        uint ImageSize { get; }

        /// <summary>
        /// Set the boot image data stream, emulation type, and image size
        /// </summary>
        /// <param name="newVal"></param>
        [DispId(20)]
        void AssignBootImage(IStream newVal);
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FD9-975B-59BE-A960-9A2A262853A5")]
    public interface IFsiItem
    {
        /// <summary>
        /// Item name
        /// </summary>
        [DispId(11)]
        String Name { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Path
        /// </summary>
        [DispId(12)]
        String FullPath { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Date and time of creation
        /// </summary>
        [DispId(13)]
        DateTime CreationTime { get; set; }

        /// <summary>
        /// Date and time of last access
        /// </summary>
        [DispId(14)]
        DateTime LastAccessedTime { get; set; }

        /// <summary>
        /// Date and time of last modification
        /// </summary>
        [DispId(15)]
        DateTime LastModifiedTime { get; set; }

        /// <summary>
        /// Flag indicating if item is hidden
        /// </summary>
        [DispId(16)]
        bool IsHidden { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(17)]
        string FileSystemName(FsiFileSystems fileSystem);

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(18)]
        string FileSystemPath(FsiFileSystems fileSystem);
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FDB-975B-59BE-A960-9A2A262853A5")]
    public interface IFsiFileItem
    {
        // IFsiItem
        /// <summary>
        /// Item name
        /// </summary>
        [DispId(11)]
        String Name { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Path
        /// </summary>
        [DispId(12)]
        String FullPath { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Date and time of creation
        /// </summary>
        [DispId(13)]
        DateTime CreationTime { get; set; }

        /// <summary>
        /// Date and time of last access
        /// </summary>
        [DispId(14)]
        DateTime LastAccessedTime { get; set; }

        /// <summary>
        /// Date and time of last modification
        /// </summary>
        [DispId(15)]
        DateTime LastModifiedTime { get; set; }

        /// <summary>
        /// Flag indicating if item is hidden
        /// </summary>
        [DispId(16)]
        bool IsHidden { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(17)]
        string FileSystemName(FsiFileSystems fileSystem);

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(18)]
        string FileSystemPath(FsiFileSystems fileSystem);

        // IFsiFileItem
        /// <summary>
        /// Data byte count
        /// </summary>
        [DispId(41)]
        long DataSize { get; }

        /// <summary>
        /// Lower 32 bits of the data byte count
        /// </summary>
        [DispId(42)]
        int DataSize32BitLow { get; }

        /// <summary>
        /// Upper 32 bits of the data byte count
        /// </summary>
        [DispId(43)]
        int DataSize32BitHigh { get; }

        /// <summary>
        /// Data stream
        /// </summary>
        [DispId(44)]
        IStream Data { get; set; }
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FDC-975B-59BE-A960-9A2A262853A5")]
    public interface IFsiDirectoryItem
    {
        // IFsiItem
        /// <summary>
        /// Item name
        /// </summary>
        [DispId(11)]
        String Name { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Path
        /// </summary>
        [DispId(12)]
        String FullPath { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// Date and time of creation
        /// </summary>
        [DispId(13)]
        DateTime CreationTime { get; set; }

        /// <summary>
        /// Date and time of last access
        /// </summary>
        [DispId(14)]
        DateTime LastAccessedTime { get; set; }

        /// <summary>
        /// Date and time of last modification
        /// </summary>
        [DispId(15)]
        DateTime LastModifiedTime { get; set; }

        /// <summary>
        /// Flag indicating if item is hidden
        /// </summary>
        [DispId(16)]
        bool IsHidden { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(17)]
        string FileSystemName(FsiFileSystems fileSystem);

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(18)]
        string FileSystemPath(FsiFileSystems fileSystem);

        // IFsiDirectoryItem
        /// <summary>
        /// Get an enumerator for the collection
        /// </summary>
        /// <returns></returns>
        [DispId(-4)]
        [TypeLibFunc(65)]
        IEnumerator GetEnumerator();

        /// <summary>
        /// Get the item with the given relative path
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        [DispId(0)]
        IFsiItem this[string path] { get; }

        /// <summary>
        /// Number of items in the collection
        /// </summary>
        [DispId(1)]
        int Count { get; }

        /// <summary>
        /// Get a non-variant enumerator
        /// </summary>
        [DispId(2)]
        IEnumFsiItems EnumFsiItems { get; }

        /// <summary>
        /// Add a directory with the specified relative path
        /// </summary>
        /// <param name="path"></param>
        [DispId(30)]
        void AddDirectory(string path);

        /// <summary>
        /// Add a file with the specified relative path and data
        /// </summary>
        /// <param name="path"></param>
        /// <param name="fileData"></param>
        [DispId(31)]
        void AddFile(string path, IStream fileData);

        /// <summary>
        /// Add files and directories from the specified source directory
        /// </summary>
        /// <param name="sourceDirectory"></param>
        /// <param name="includeBaseDirectory"></param>
        [DispId(32)]
        void AddTree(string sourceDirectory, bool includeBaseDirectory);

        /// <summary>
        /// Add an item
        /// </summary>
        /// <param name="Item"></param>
        [DispId(33)]
        void Add(IFsiItem Item);

        /// <summary>
        /// Remove an item with the specified relative path
        /// </summary>
        /// <param name="path"></param>
        [DispId(34)]
        void Remove(string path);

        /// <summary>
        /// Remove a subtree with the specified relative path
        /// </summary>
        /// <param name="path"></param>
        [DispId(35)]
        void RemoveTree(string path);
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("2C941FDA-975B-59BE-A960-9A2A262853A5")]
    public interface IEnumFsiItems
    {
        /// <summary>
        /// Get next items in the enumeration
        /// </summary>
        /// <param name="celt"></param>
        /// <param name="rgelt"></param>
        /// <param name="pceltFetched"></param>
        void Next(uint celt, out IFsiItem rgelt, out uint pceltFetched);

        /// <summary>
        /// Remoting support for Next (allow NULL pointer for item count when 
        /// requesting single item)
        /// </summary>
        /// <param name="celt"></param>
        /// <param name="rgelt"></param>
        /// <param name="pceltFetched"></param>
        void RemoteNext(uint celt, out IFsiItem rgelt, out uint pceltFetched);

        /// <summary>
        /// Skip items in the enumeration
        /// </summary>
        /// <param name="celt"></param>
        void Skip(uint celt);

        /// <summary>
        /// Reset the enumerator
        /// </summary>
        void Reset();

        /// <summary>
        /// Make a copy of the enumerator
        /// </summary>
        /// <param name="ppEnum"></param>
        void Clone(out IEnumFsiItems ppEnum);
    }

    [TypeLibType(TypeLibTypeFlags.FDual         |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FD5-975B-59BE-A960-9A2A262853A5")]
    public interface IProgressItem
    {
        /// <summary>
        /// Progress item description
        /// </summary>
        [DispId(1)]
        String Description { [return: MarshalAs(UnmanagedType.BStr)] get; }

        /// <summary>
        /// First block in the range of blocks used by the progress item
        /// </summary>
        [DispId(2)]
        uint FirstBlock { get; }

        /// <summary>
        /// Last block in the range of blocks used by the progress item
        /// </summary>
        [DispId(3)]
        uint LastBlock { get; }

        /// <summary>
        /// Number of blocks used by the progress item
        /// </summary>
        [DispId(4)]
        uint BlockCount { get; }
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("2C941FD6-975B-59BE-A960-9A2A262853A5")]
    public interface IEnumProgressItems
    {
        /// <summary>
        /// Get next items in the enumeration
        /// </summary>
        /// <param name="celt"></param>
        /// <param name="rgelt"></param>
        /// <param name="pceltFetched"></param>
        void Next(uint celt, out IProgressItem rgelt, out uint pceltFetched);

        /// <summary>
        /// Remoting support for Next (allow NULL pointer for item count when 
        /// requesting single item)
        /// </summary>
        /// <param name="celt"></param>
        /// <param name="rgelt"></param>
        /// <param name="pceltFetched"></param>
        void RemoteNext(uint celt, out IProgressItem rgelt, out uint pceltFetched);

        /// <summary>
        /// Skip items in the enumeration
        /// </summary>
        /// <param name="celt"></param>
        void Skip(uint celt);

        /// <summary>
        /// Reset the enumerator
        /// </summary>
        void Reset();

        /// <summary>
        /// Make a copy of the enumerator
        /// </summary>
        /// <param name="ppEnum"></param>
        void Clone(out IEnumProgressItems ppEnum);
    }


    [TypeLibType(TypeLibTypeFlags.FDual |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2735413D-7F64-5B0F-8F00-5D77AFBE261E")]
    public interface IDiscFormat2DataEventArgs
    {
        // IWriteEngine2EventArgs
        /// <summary>
        /// The starting logical block address for the current write operation.
        /// </summary>
        [DispId(256)]
        int StartLba { get; }

        /// <summary>
        /// The number of sectors being written for the current write operation.
        /// </summary>
        [DispId(257)]
        int SectorCount { get; }

        /// <summary>
        /// The last logical block address of data read for the current write operation.
        /// </summary>
        [DispId(258)]
        int LastReadLba { get; }

        /// <summary>
        /// The last logical block address of data written for the current write operation
        /// </summary>
        [DispId(259)]
        int LastWrittenLba { get; }

        /// <summary>
        /// The total bytes available in the system's cache buffer
        /// </summary>
        [DispId(262)]
        int TotalSystemBuffer { get; }

        /// <summary>
        /// The used bytes in the system's cache buffer
        /// </summary>
        [DispId(263)]
        int UsedSystemBuffer { get; }

        /// <summary>
        /// The free bytes in the system's cache buffer
        /// </summary>
        [DispId(264)]
        int FreeSystemBuffer { get; }

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
        IMAPI_FORMAT2_DATA_WRITE_ACTION CurrentAction { get; }
    }

    // CoClass - Specifies the class identifier of a coclass 
    // imported from a type library.
    [CoClass(typeof(MsftDiscMaster2Class)), ComImport]
    [Guid("27354130-7F64-5B0F-8F00-5D77AFBE261E")]
    public interface MsftDiscMaster2 : IDiscMaster2
    {
    }

    [CoClass(typeof(MsftDiscRecorder2Class)), ComImport]
    [Guid("27354133-7F64-5B0F-8F00-5D77AFBE261E")]
    public interface MsftDiscRecorder2 : IDiscRecorder2
    {
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [CoClass(typeof(MsftDiscRecorder2Class)), ComImport]
    [Guid("27354132-7F64-5B0F-8F00-5D77AFBE261E")]
    public interface MsftDiscRecorder2Ex : IDiscRecorder2Ex
    {
    }

    [CoClass(typeof(MsftDiscFormat2DataClass)), ComImport]
    [Guid("27354153-9F64-5B0F-8F00-5D77AFBE261E")]
    public interface MsftDiscFormat2Data : IDiscFormat2Data, DiscFormat2Data_Events
    {
    }

    [CoClass(typeof(MsftFileSystemImageClass)), ComImport]
    [Guid("2C941FE1-975B-59BE-A960-9A2A262853A5")]
    public interface MsftFileSystemImage : IFileSystemImage
    {
    }

    [CoClass(typeof(MsftBootOptionsClass)), ComImport]
    [Guid("2C941FD4-975B-59BE-A960-9A2A262853A5")]
    public interface MsftBootOptions : IBootOptions
    {
    }

    [TypeLibType(4480)]
    [Guid("2735413C-7F64-5B0F-8F00-5D77AFBE261E"), ComImport]
    public interface DDiscFormat2DataEvents
    {
        [DispId(0x200)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
        void Update([In, MarshalAs(UnmanagedType.IDispatch)] object sender, [In, MarshalAs(UnmanagedType.IDispatch)] object progress);
    }

    //Events
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void DiscFormat2Data_EventsHandler([In, MarshalAs(UnmanagedType.IDispatch)] object sender, [In, MarshalAs(UnmanagedType.IDispatch)] object args);

    [TypeLibType((short)0x10), ComVisible(false), ComEventInterface(typeof(DDiscFormat2DataEvents), typeof(DiscFormat2Data_EventsProvider))]
    public interface DiscFormat2Data_Events
    {
        event DiscFormat2Data_EventsHandler Update;
    }

    [TypeLibType(TypeLibTypeFlags.FHidden), ClassInterface(ClassInterfaceType.None)]
    public sealed class DiscFormat2Data_SinkHelper : DDiscFormat2DataEvents
    {
        public DiscFormat2Data_SinkHelper(DiscFormat2Data_EventsHandler eventHandler)
        {
            if (eventHandler == null)
                throw new ArgumentNullException("Delegate (callback function) cannot be null");
            m_dwCookie = 0;
            m_UpdateDelegate = eventHandler;
        }

        public void Update([In, MarshalAs(UnmanagedType.IDispatch)] object sender, [In, MarshalAs(UnmanagedType.IDispatch)] object args)
        {
            m_UpdateDelegate(sender, args);
        }

        public int Cookie
        {
            get
            {
                return m_dwCookie;
            }
            set
            {
                m_dwCookie = value;
            }
        }

        public DiscFormat2Data_EventsHandler UpdateDelegate
        {
            get
            {
                return m_UpdateDelegate;
            }
            set
            {
                m_UpdateDelegate = value;
            }
        }

        private int m_dwCookie;
        private DiscFormat2Data_EventsHandler m_UpdateDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class DiscFormat2Data_EventsProvider : DiscFormat2Data_Events, IDisposable
    {
        public DiscFormat2Data_EventsProvider(object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    m_aEventSinkHelpers = new Hashtable();
                    Guid eventsGuid = typeof(DDiscFormat2DataEvents).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        public event DiscFormat2Data_EventsHandler Update
        {
            add
            {
                lock (this)
                {
                    DiscFormat2Data_SinkHelper helper = new DiscFormat2Data_SinkHelper(value);
                    int cookie = -1;

                    m_ConnectionPoint.Advise(helper, out cookie);
                    helper.Cookie = cookie;
                    m_aEventSinkHelpers.Add(helper.UpdateDelegate, helper);
                }
            }

            remove
            {
                lock (this)
                {
                    DiscFormat2Data_SinkHelper helper = m_aEventSinkHelpers[value] as DiscFormat2Data_SinkHelper;
                    if (helper != null)
                    {
                        m_ConnectionPoint.Unadvise(helper.Cookie);
                        m_aEventSinkHelpers.Remove(helper.UpdateDelegate);
                    }
                }
            }
        }

        public void Dispose()
        {
            Cleanup();
            GC.SuppressFinalize(this);
        }

        ~DiscFormat2Data_EventsProvider()
        {
            Cleanup();
        }

        private void Cleanup()
        {
            Monitor.Enter(this);
            try
            {
                foreach (DiscFormat2Data_SinkHelper helper in m_aEventSinkHelpers)
                {
                    m_ConnectionPoint.Unadvise(helper.Cookie);
                }

                m_aEventSinkHelpers.Clear();
                Marshal.ReleaseComObject(m_ConnectionPoint);
            }
            catch (SynchronizationLockException)
            {
                return;
            }
            finally
            {
                Monitor.Exit(this);
            }
        }

        private Hashtable m_aEventSinkHelpers;
        static private IConnectionPoint m_ConnectionPoint = null;
    }

    // Class
    /// <summary>
    /// This is a representation of an IO.Stream and IStream object. 
    /// </summary>
    public class AStream : Stream, IStream, IDisposable
    {
        /// <summary>
        /// Gets a value indicating whether the current stream supports reading.
        /// </summary>
        public override bool CanRead
        {
            get
            {
                if (TheIStream != null)
                {
                    return true;
                }
                else
                {
                    return TheStream.CanRead;
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether the current stream supports seeking.
        /// </summary>
        public override bool CanSeek
        {
            get
            {
                if (TheIStream != null)
                {
                    return true;
                }
                else
                {
                    return TheStream.CanSeek;
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether the current stream supports writing.
        /// </summary>
        public override bool CanWrite
        {
            get
            {
                if (TheIStream != null)
                {
                    return true;
                }
                else
                {
                    return TheStream.CanWrite;
                }
            }
        }

        public override bool CanTimeout
        {
            get
            {
                if (TheIStream != null)
                {
                    return false;
                }
                else
                {
                    return TheStream.CanTimeout;
                }
            }
        }

        /// <summary>
        /// Gets the length in bytes of the stream.
        /// </summary>
        public override long Length
        {
            get
            {
                if (TheIStream != null)
                {
                    // Call IStream.Stat to retrieve info about the stream,
                    // which includes the length. STATFLAG_NONAME means that we don't
                    // care about the name (STATSTG.pwcsName), so there is no need for
                    // the method to allocate memory for the string.
                    System.Runtime.InteropServices.ComTypes.STATSTG statstg;
                    TheIStream.Stat(out statstg, 1);
                    return statstg.cbSize;
                }
                else
                {
                    return TheStream.Length;
                }
            }
        }

        /// <summary>
        /// Gets or sets the position within the current stream.
        /// </summary>
        public override long Position
        {
            get
            {
                if (TheIStream != null)
                {
                    return Seek(0, SeekOrigin.Current);
                }
                else
                {
                    return TheStream.Position;
                }
            }
            set
            {
                if (TheIStream != null)
                {
                    Seek(value, SeekOrigin.Begin);
                }
                else
                {
                    TheStream.Position = value;
                }
            }
        }

        /// <summary>
        /// Clears all buffers for this stream and causes any buffered data to be written 
        /// to the underlying device.
        /// </summary>
        public override void Flush()
        {
            if (TheIStream != null)
            {
                TheIStream.Commit(0 /*STGC_DEFAULT*/);
            }
            else
            {
                TheStream.Flush();
            }
        }

        /// <summary>
        /// Reads a sequence of bytes from the current stream and advances the position 
        /// within the stream by the number of bytes read.
        /// </summary>
        /// <param name="buffer">An array of bytes. When this method returns, the buffer contains the specified byte array with the values between offset and (offset + count - 1) replaced by the bytes read from the current source.</param>
        /// <param name="offset">The zero-based byte offset in buffer at which to begin storing the data read from the current stream.</param>
        /// <param name="count">The maximum number of bytes to be read from the current stream.</param>
        /// <returns>The total number of bytes read into the buffer. This can be less than the number of bytes requested if that many bytes are not currently available, or zero (0) if the end of the stream has been reached.</returns>
        public override int Read(byte[] buffer, int offset, int count)
        {
            if (TheIStream != null)
            {
                if (offset != 0) throw new NotSupportedException("Only a zero offset is supported.");

                int bytesRead = 0;
                IntPtr br = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(int)));
                Marshal.WriteInt32(br, 0);

                // Include try catch for c++ eh exceptions. are they the same as comexceptions?
                TheIStream.Read(buffer, count, br);
                bytesRead = Marshal.ReadInt32(br);

                Marshal.FreeHGlobal(br);

                return bytesRead;
            }
            else
            {
                return TheStream.Read(buffer, offset, count);
            }
        }

        /// <summary>
        /// Sets the position within the current stream.
        /// </summary>
        /// <param name="offset">A byte offset relative to the origin parameter.</param>
        /// <param name="origin">A value of type SeekOrigin indicating the reference point used to obtain the new position.</param>
        /// <returns>The new position within the current stream.</returns>
        public override long Seek(long offset, System.IO.SeekOrigin origin)
        {
            if (TheIStream != null)
            {
                long position = 0;
                IntPtr pos = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(long)));
                Marshal.WriteInt64(pos, 0);

                // The enum values of SeekOrigin match the enum values of
                // STREAM_SEEK, so we can just cast the origin to an integer.
                TheIStream.Seek(offset, (int)origin, pos);
                position = Marshal.ReadInt64(pos);

                Marshal.FreeHGlobal(pos);

                return position;
            }
            else
            {
                return TheStream.Seek(offset, origin);
            }
        }

        /// <summary>
        /// Sets the length of the current stream.
        /// </summary>
        /// <param name="value">The desired length of the current stream in bytes.</param>
        public override void SetLength(long value)
        {
            if (TheIStream != null)
            {
                TheIStream.SetSize(value);
            }
            else
            {
                TheStream.SetLength(value);
            }
        }

        // Writes a sequence of bytes to the current stream and advances the
        // current position within this stream by the number of bytes written
        /// <summary>
        /// Writes a sequence of bytes to the current stream and advances the current position 
        /// within this stream by the number of bytes written.
        /// </summary>
        /// <param name="buffer">An array of bytes. This method copies count bytes from buffer to the current stream.</param>
        /// <param name="offset">The zero-based byte offset in buffer at which to begin copying bytes to the current stream.</param>
        /// <param name="count">The number of bytes to be written to the current stream.</param>
        public override void Write(byte[] buffer, int offset, int count)
        {
            if (TheIStream != null)
            {
                if (offset != 0) throw new NotSupportedException("Only a zero offset is supported.");

                // Pass "null" for the last parameter since we don't use the value
                TheIStream.Write(buffer, count, IntPtr.Zero);
            }
            else
            {
                TheStream.Write(buffer, offset, count);
            }
        }

        /// <summary>
        /// Creates a new stream object with its own seek pointer that references 
        /// the same bytes as the original stream.
        /// </summary>
        /// <remarks>
        /// This method is not used and always throws the exception.
        /// </remarks>
        /// <param name="ppstm">When successful, pointer to the location of an IStream pointer to the new stream object.</param>
        ///<exception cref="NotSupportedException">The IO.Streamtream cannot be cloned.</exception>
        public void Clone(out IStream ppstm)
        {
            if (TheStream != null) throw new NotSupportedException("The Stream cannot be cloned.");

            TheIStream.Clone(out ppstm);
        }

        /// <summary>
        /// Ensures that any changes made to an stream object that is open in transacted 
        /// mode are reflected in the parent storage.
        /// </summary>
        /// <remarks>
        /// The <paramref name="grfCommitFlags"/> parameter is not used and this method only does Stream.Flush()
        /// </remarks>
        /// <param name="grfCommitFlags">Controls how the changes for the stream object are committed. 
        /// See the STGC enumeration for a definition of these values.</param>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        public void Commit(int grfCommitFlags)
        {
            // Clears all buffers for this stream and causes any buffered data to be written 
            // to the underlying device.
            if (TheStream != null)
            {
                TheStream.Flush();
            }
            else
            {
                TheIStream.Commit(grfCommitFlags);
            }
        }

        /// <summary>
        /// Copies a specified number of bytes from the current seek pointer in the stream 
        /// to the current seek pointer in another stream.
        /// </summary>
        /// <param name="pstm">
        /// The destination stream. The pstm stream  can be a new stream or a clone of the source stream.
        /// </param>
        /// <param name="cb">
        /// The number of bytes to copy from the source stream.
        /// </param>
        /// <param name="pcbRead">
        /// The actual number of bytes read from the source. 
        /// It can be set to IntPtr.Zero. 
        /// In this case, this method does not provide the actual number of bytes read.
        /// </param>
        /// <typeparam name="pcbRead">Native UInt64</typeparam>
        /// <param name="pcbWritten">
        /// The actual number of bytes written to the destination. 
        /// It can be set this to IntPtr.Zero. 
        /// In this case, this method does not provide the actual number of bytes written.
        /// </param>
        /// <typeparam name="pcbWritten">Native UInt64</typeparam>
        /// <returns>
        /// The actual number of bytes read (<paramref name="pcbRead"/>) and written (<paramref name="pcbWritten"/>) from the source.
        /// </returns>
        ///<exception cref="ArgumentException">The sum of offset and count is larger than the buffer length.</exception>
        ///<exception cref="ArgumentNullException">buffer is a null reference.</exception>
        ///<exception cref="ArgumentOutOfRangeException">offset or count is negative.</exception>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void CopyTo(IStream pstm, long cb, IntPtr pcbRead, IntPtr pcbWritten)
        {
            if (TheStream != null)
            {
                byte[] sourceBytes = new byte[cb];
                int currentBytesRead = 0;
                long totalBytesRead = 0;
                int currentBytesWritten = 0;
                long totalBytesWritten = 0;

                IntPtr bw = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(int)));
                Marshal.WriteInt32(bw, 0);

                while (totalBytesWritten < cb)
                {
                    currentBytesRead = TheStream.Read(sourceBytes, 0, (int)(cb - totalBytesWritten));

                    // Has the end of the stream been reached?
                    if (currentBytesRead == 0) break;

                    totalBytesRead += currentBytesRead;

                    pstm.Write(sourceBytes, currentBytesRead, bw);
                    currentBytesWritten = Marshal.ReadInt32(bw);
                    if (currentBytesWritten != currentBytesRead)
                    {
                        Debug.WriteLine("ERROR!: The IStream Write is not writing all the bytes needed!");
                    }
                    totalBytesWritten += currentBytesWritten;
                }

                Marshal.FreeHGlobal(bw);

                if (pcbRead != IntPtr.Zero) Marshal.WriteInt64(pcbRead, totalBytesRead);
                if (pcbWritten != IntPtr.Zero) Marshal.WriteInt64(pcbWritten, totalBytesWritten);
            }
            else
            {
                TheIStream.CopyTo(pstm, cb, pcbRead, pcbWritten);
            }
        }

        /// <summary>
        /// Restricts access to a specified range of bytes in the stream.
        /// </summary>
        /// <remarks>
        /// This method is not used and always throws the exception.
        /// </remarks>
        /// <param name="libOffset">Integer that specifies the byte offset for the beginning of the range.</param>
        /// <param name="cb">Integer that specifies the length of the range, in bytes, to be restricted.</param>
        /// <param name="dwLockType">Specifies the restrictions being requested on accessing the range.</param>
        ///<exception cref="NotSupportedException">The IO.Stream does not support locking.</exception>
        public void LockRegion(long libOffset, long cb, int dwLockType)
        {
            if (TheStream != null) throw new NotSupportedException("Stream does not support locking.");

            TheIStream.LockRegion(libOffset, cb, dwLockType);
        }

        /// <summary>
        /// Reads a specified number of bytes from the stream object 
        /// into memory starting at the current seek pointer.
        /// </summary>
        /// <param name="pv">The buffer which the stream data is read into.</param>
        /// <param name="cb">The number of bytes of data to read from the stream object.</param>
        /// <param name="pcbRead">
        /// A pointer to a ULONG variable that receives the actual number of bytes read from the stream object.
        /// It can be set to IntPtr.Zero. 
        /// In this case, this method does not return the number of bytes read.
        /// </param>
        /// <typeparam name="pcbRead">Native UInt32</typeparam>
        /// <returns>
        /// The actual number of bytes read (<paramref name="pcbRead"/>) from the source.
        /// </returns>
        ///<exception cref="ArgumentException">The sum of offset and count is larger than the buffer length.</exception>
        ///<exception cref="ArgumentNullException">buffer is a null reference.</exception>
        ///<exception cref="ArgumentOutOfRangeException">offset or count is negative.</exception>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void Read(byte[] pv, int cb, IntPtr pcbRead)
        {
            if (TheStream != null)
            {
                if (pcbRead == IntPtr.Zero)
                {
                    // User isn't interested in how many bytes were read
                    TheStream.Read(pv, 0, cb);
                }
                else
                {
                    Marshal.WriteInt32(pcbRead, TheStream.Read(pv, 0, cb));
                }
            }
            else
            {
                TheIStream.Read(pv, cb, pcbRead);
            }
        }

        /// <summary>
        /// Discards all changes that have been made to a transacted 
        /// stream since the last stream.Commit call
        /// </summary>
        /// <remarks>
        /// This method is not used and always throws the exception.
        /// </remarks>
        ///<exception cref="NotSupportedException">The IO.Stream does not support reverting.</exception>
        public void Revert()
        {
            if (TheStream != null) throw new NotSupportedException("Stream does not support reverting.");

            TheIStream.Revert();
        }

        /// <summary>
        /// Changes the seek pointer to a new location relative to the beginning
        ///of the stream, the end of the stream, or the current seek pointer
        /// </summary>
        /// <param name="dlibMove">
        /// The displacement to be added to the location indicated by the dwOrigin parameter. 
        /// If dwOrigin is STREAM_SEEK_SET, this is interpreted as an unsigned value rather than a signed value.
        /// </param>
        /// <param name="dwOrigin">
        /// The origin for the displacement specified in dlibMove. 
        /// The origin can be the beginning of the file (STREAM_SEEK_SET), the current seek pointer (STREAM_SEEK_CUR), or the end of the file (STREAM_SEEK_END).
        /// </param>
        /// <param name="plibNewPosition">
        /// The location where this method writes the value of the new seek pointer from the beginning of the stream.
        /// It can be set to IntPtr.Zero. In this case, this method does not provide the new seek pointer.
        /// </param>
        /// <typeparam name="pcbRead">Native UInt64</typeparam>
        /// <returns>
        /// Returns in <paramref name="plibNewPosition"/> the location where this method writes the value of the new seek pointer from the beginning of the stream.
        /// </returns>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void Seek(long dlibMove, int dwOrigin, IntPtr plibNewPosition)
        {
            if (TheStream != null)
            {
                // The enum values of SeekOrigin match the enum values of
                // STREAM_SEEK, so we can just cast the dwOrigin to a SeekOrigin

                if (plibNewPosition == IntPtr.Zero)
                {
                    // User isn't interested in new position
                    TheStream.Seek(dlibMove, (SeekOrigin)dwOrigin);
                }
                else
                {
                    SeekOrigin origin = (SeekOrigin)dwOrigin;
                    if (origin != SeekOrigin.Begin &&
                        origin != SeekOrigin.Current &&
                        origin != SeekOrigin.End)
                    {
                        origin = SeekOrigin.Begin;
                    }
                    Marshal.WriteInt64(plibNewPosition, TheStream.Seek(dlibMove, origin));
                }
            }
            else
            {
                TheIStream.Seek(dlibMove, dwOrigin, plibNewPosition);
            }
        }

        /// <summary>
        /// Changes the size of the stream object.
        /// </summary>
        /// <param name="libNewSize">Specifies the new size of the stream as a number of bytes.</param>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void SetSize(long libNewSize)
        {
            if (TheStream != null)
            {
                // Sets the length of the current stream.
                TheStream.SetLength(libNewSize);
            }
            else
            {
                TheIStream.SetSize(libNewSize);
            }
        }

        /// <summary>
        /// Retrieves the STATSTG structure for this stream.
        /// </summary>
        /// <remarks>
        /// The <paramref name="grfStatFlag"/> parameter is not used
        /// </remarks>
        /// <param name="pstatstg">
        /// The STATSTG structure where this method places information about this stream object.
        /// </param>
        /// <param name="grfStatFlag">
        /// Specifies that this method does not return some of the members in the STATSTG structure, 
        /// thus saving a memory allocation operation. This parameter is not used internally.
        /// </param>
        ///<exception cref="NotSupportedException">The stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void Stat(out System.Runtime.InteropServices.ComTypes.STATSTG pstatstg, int grfStatFlag)
        {
            if (TheStream != null)
            {
                pstatstg = new System.Runtime.InteropServices.ComTypes.STATSTG();
                pstatstg.type = 2; // STGTY_STREAM
                // Gets the length in bytes of the stream.
                pstatstg.cbSize = TheStream.Length;
                pstatstg.grfMode = 2; // STGM_READWRITE;
                pstatstg.grfLocksSupported = 2; // LOCK_EXCLUSIVE
            }
            else
            {
                TheIStream.Stat(out pstatstg, grfStatFlag);
            }
        }

        /// <summary>
        /// Removes the access restriction on a range of bytes previously 
        /// restricted with the LockRegion method.
        /// </summary>
        /// <remarks>
        /// This method is not used and always throws the exception.
        /// </remarks>
        /// <param name="libOffset">Specifies the byte offset for the beginning of the range.</param>
        /// <param name="cb">Specifies, in bytes, the length of the range to be restricted.</param>
        /// <param name="dwLockType">Specifies the access restrictions previously placed on the range.</param>
        ///<exception cref="NotSupportedException">The IO.Stream does not support unlocking.</exception>
        public void UnlockRegion(long libOffset, long cb, int dwLockType)
        {
            if (TheStream != null) throw new NotSupportedException("Stream does not support unlocking.");

            TheIStream.UnlockRegion(libOffset, cb, dwLockType);
        }

        /// <summary>
        /// Writes a specified number of bytes into the stream object 
        ///starting at the current seek pointer.
        /// </summary>
        /// <param name="pv">The buffer that contains the data that is to be written to the stream. 
        /// A valid buffer must be provided for this parameter even when cb is zero.</param>
        /// <param name="cb">The number of bytes of data to attempt to write into the stream. This value can be zero.</param>
        /// <param name="pcbWritten">
        /// A variable where this method writes the actual number of bytes written to the stream object. 
        /// The caller can set this to IntPtr.Zero, in which case this method does not provide the actual number of bytes written.
        /// </param>
        /// <typeparam name="pcbWritten">Native UInt32</typeparam>
        /// <returns>
        /// The actual number of bytes written (<paramref name="pcbWritten"/>).
        /// </returns>
        ///<exception cref="ArgumentException">The sum of offset and count is larger than the buffer length.</exception>
        ///<exception cref="ArgumentNullException">buffer is a null reference.</exception>
        ///<exception cref="ArgumentOutOfRangeException">offset or count is negative.</exception>
        ///<exception cref="IOException">An I/O error occurs.</exception>
        ///<exception cref="NotSupportedException">The IO.Stream does not support reading.</exception>
        ///<exception cref="ObjectDisposedException">Methods were called after the stream was closed.</exception>
        public void Write(byte[] pv, int cb, IntPtr pcbWritten)
        {
            if (TheStream != null)
            {
                if (pcbWritten == IntPtr.Zero)
                {
                    // User isn't interested in how many bytes were written
                    TheStream.Write(pv, 0, cb);
                }
                else
                {
                    long currentPosition = TheStream.Position;
                    TheStream.Write(pv, 0, cb);
                    Marshal.WriteInt32(pcbWritten, (int)(TheStream.Position - currentPosition));
                }
            }
            else
            {
                TheIStream.Write(pv, cb, pcbWritten);
            }
        }

        // Default constructor. Should not be used to create an AStream object.
        private AStream()
        {
            TheStream = null;
            TheIStream = null;
        }

        // Copy constructor. It is not safe to only pass the Stream and IStream.
        private AStream(AStream previousAStream)
        {
            TheStream = previousAStream.TheStream;
            TheIStream = previousAStream.TheIStream;
        }

        /// <summary>
        /// Initializes a new instance of the AStream class.
        /// </summary>
        /// <param name="stream">An IO.Stream</param>
        ///<exception cref="ArgumentNullException">Stream cannot be null</exception>
        public AStream(Stream stream)
        {
            TheStream = null;
            TheIStream = null;

            if (stream == null)
            {
                throw new ArgumentNullException("Stream cannot be null");
            }
            TheStream = stream;
        }

        /// <summary>
        /// Initializes a new instance of the AStream class.
        /// </summary>
        /// <param name="stream">A ComTypes.IStream</param>
        ///<exception cref="ArgumentNullException">Stream cannot be null</exception>
        public AStream(IStream stream)
        {
            TheStream = null;
            TheIStream = null;

            if (stream == null)
            {
                throw new ArgumentNullException("IStream cannot be null");
            }
            TheIStream = stream;
        }

        // Allows the Object to attempt to free resources and perform other 
        // cleanup operations before the Object is reclaimed by garbage collection. 
        // (Inherited from Object.)
        ~AStream()
        {
            if (TheStream != null)
            {
                TheStream.Close();
            }
        }

        /// <summary>
        /// Releases all resources used by the Stream object.
        /// </summary>
        void IDisposable.Dispose()
        {
            Close();
        }

        /// <summary>
        /// Closes the current stream and releases any resources 
        /// (such as the Stream) associated with the current IStream.
        /// </summary>
        /// <remarks>
        /// This method is not a member in IStream.
        /// </remarks>
        public override void Close()
        {
            if (TheStream != null)
            {
                TheStream.Close();
            }
            else
            {
                TheIStream.Commit(0 /*STGC_DEFAULT*/);
//                Marshal.ReleaseComObject(TheIStream);    // Investigate this because we cannot release an IStream to the stash file
            }
            GC.SuppressFinalize(this);
        }

/*        public static IStream ToIStream(object stream)
        {
            IntPtr ppv;
            IntPtr pUnk = Marshal.GetIUnknownForObject(stream);
            Object iSteam = null;
            Guid iid = Marshal.GenerateGuidForType(typeof(IStream));   // ComTypes.IStream GUID

            if (Marshal.QueryInterface(pUnk, ref iid, out ppv) == 0)
                iSteam = Marshal.GetUniqueObjectForIUnknown(ppv);

            return (System.Runtime.InteropServices.ComTypes.IStream)iSteam;
        }*/

        public static IStream ToIStream(object stream)
        {
            if (stream is Stream)
            {
                return new AStream(stream as Stream);
            }

            if (stream is IStream)
            {
                return stream as IStream;
            }

            return null;
        }

        public static Stream ToStream(object stream)
        {
            if (stream is Stream)
            {
                return stream as Stream;
            }

            if (stream is IStream)
            {
                return new AStream(stream as IStream);
            }

            return null;
        }

        private Stream TheStream;   // The Stream being wrapped
        private IStream TheIStream; // The IStream being wrapped

    }

    [TypeLibType(TypeLibTypeFlags.FCanCreate)]
    [ClassInterface(ClassInterfaceType.None)]
    [ComSourceInterfaces("DDiscMaster2Events\0")]
    [Guid("2735412E-7F64-5B0F-8F00-5D77AFBE261E"), ComImport]
    public class MsftDiscMaster2Class
    {
    }

    [TypeLibType(TypeLibTypeFlags.FCanCreate)]
    [ClassInterface(ClassInterfaceType.None)]
    [Guid("2735412D-7F64-5B0F-8F00-5D77AFBE261E"), ComImport]
    public class MsftDiscRecorder2Class
    {
    }

    [TypeLibType(TypeLibTypeFlags.FCanCreate)]
    [ClassInterface(ClassInterfaceType.None)]
    [ComSourceInterfaces("DDiscFormat2DataEvents\0")]
    [Guid("2735412A-7F64-5B0F-8F00-5D77AFBE261E"), ComImport]
    public class MsftDiscFormat2DataClass
    {
    }

    [TypeLibType(TypeLibTypeFlags.FCanCreate)]
    [ClassInterface(ClassInterfaceType.None)]
    [ComSourceInterfaces("DFileSystemImageEvents\0")]
    [Guid("2C941FC5-975B-59BE-A960-9A2A262853A5"), ComImport]
    public class MsftFileSystemImageClass
    {
    }

    [TypeLibType(TypeLibTypeFlags.FCanCreate)]
    [ClassInterface(ClassInterfaceType.None)]
    [Guid("2C941FCE-975B-59BE-A960-9A2A262853A5")]
    public class MsftBootOptionsClass
    {
    }

    // Enumerator
    public enum IMAPI_FEATURE_PAGE_TYPE
    {
        IMAPI_FEATURE_PAGE_TYPE_PROFILE_LIST = 0,
        IMAPI_FEATURE_PAGE_TYPE_CORE = 1,
        IMAPI_FEATURE_PAGE_TYPE_MORPHING = 2,
        IMAPI_FEATURE_PAGE_TYPE_REMOVABLE_MEDIUM = 3,
        IMAPI_FEATURE_PAGE_TYPE_WRITE_PROTECT = 4,
        IMAPI_FEATURE_PAGE_TYPE_RANDOMLY_READABLE = 16,
        IMAPI_FEATURE_PAGE_TYPE_CD_MULTIREAD = 29,
        IMAPI_FEATURE_PAGE_TYPE_CD_READ = 30,
        IMAPI_FEATURE_PAGE_TYPE_DVD_READ = 31,
        IMAPI_FEATURE_PAGE_TYPE_RANDOMLY_WRITABLE = 32,
        IMAPI_FEATURE_PAGE_TYPE_INCREMENTAL_STREAMING_WRITABLE = 33,
        IMAPI_FEATURE_PAGE_TYPE_SECTOR_ERASABLE = 34,
        IMAPI_FEATURE_PAGE_TYPE_FORMATTABLE = 35,
        IMAPI_FEATURE_PAGE_TYPE_HARDWARE_DEFECT_MANAGEMENT = 36,
        IMAPI_FEATURE_PAGE_TYPE_WRITE_ONCE = 37,
        IMAPI_FEATURE_PAGE_TYPE_RESTRICTED_OVERWRITE = 38,
        IMAPI_FEATURE_PAGE_TYPE_CDRW_CAV_WRITE = 39,
        IMAPI_FEATURE_PAGE_TYPE_MRW = 40,
        IMAPI_FEATURE_PAGE_TYPE_ENHANCED_DEFECT_REPORTING = 41,
        IMAPI_FEATURE_PAGE_TYPE_DVD_PLUS_RW = 42,
        IMAPI_FEATURE_PAGE_TYPE_DVD_PLUS_R = 43,
        IMAPI_FEATURE_PAGE_TYPE_RIGID_RESTRICTED_OVERWRITE = 44,
        IMAPI_FEATURE_PAGE_TYPE_CD_TRACK_AT_ONCE = 45,
        IMAPI_FEATURE_PAGE_TYPE_CD_MASTERING = 46,
        IMAPI_FEATURE_PAGE_TYPE_DVD_DASH_WRITE = 47,
        IMAPI_FEATURE_PAGE_TYPE_DOUBLE_DENSITY_CD_READ = 48,
        IMAPI_FEATURE_PAGE_TYPE_DOUBLE_DENSITY_CD_R_WRITE = 49,
        IMAPI_FEATURE_PAGE_TYPE_DOUBLE_DENSITY_CD_RW_WRITE = 50,
        IMAPI_FEATURE_PAGE_TYPE_LAYER_JUMP_RECORDING = 51,
        IMAPI_FEATURE_PAGE_TYPE_CD_RW_MEDIA_WRITE_SUPPORT = 55,
        IMAPI_FEATURE_PAGE_TYPE_BD_PSEUDO_OVERWRITE = 56,
        IMAPI_FEATURE_PAGE_TYPE_DVD_PLUS_R_DUAL_LAYER = 59,
        IMAPI_FEATURE_PAGE_TYPE_BD_READ = 64,
        IMAPI_FEATURE_PAGE_TYPE_BD_WRITE = 65,
        IMAPI_FEATURE_PAGE_TYPE_HD_DVD_READ = 80,
        IMAPI_FEATURE_PAGE_TYPE_HD_DVD_WRITE = 81,
        IMAPI_FEATURE_PAGE_TYPE_POWER_MANAGEMENT = 256,
        IMAPI_FEATURE_PAGE_TYPE_SMART = 257,
        IMAPI_FEATURE_PAGE_TYPE_EMBEDDED_CHANGER = 258,
        IMAPI_FEATURE_PAGE_TYPE_CD_ANALOG_PLAY = 259,
        IMAPI_FEATURE_PAGE_TYPE_MICROCODE_UPDATE = 260,
        IMAPI_FEATURE_PAGE_TYPE_TIMEOUT = 261,
        IMAPI_FEATURE_PAGE_TYPE_DVD_CSS = 262,
        IMAPI_FEATURE_PAGE_TYPE_REAL_TIME_STREAMING = 263,
        IMAPI_FEATURE_PAGE_TYPE_LOGICAL_UNIT_SERIAL_NUMBER = 264,
        IMAPI_FEATURE_PAGE_TYPE_MEDIA_SERIAL_NUMBER = 265,
        IMAPI_FEATURE_PAGE_TYPE_DISC_CONTROL_BLOCKS = 266,
        IMAPI_FEATURE_PAGE_TYPE_DVD_CPRM = 267,
        IMAPI_FEATURE_PAGE_TYPE_FIRMWARE_INFORMATION = 268,
        IMAPI_FEATURE_PAGE_TYPE_AACS = 269,
        IMAPI_FEATURE_PAGE_TYPE_VCPS = 272,
    }

    public enum IMAPI_MODE_PAGE_TYPE
    {
        IMAPI_MODE_PAGE_TYPE_READ_WRITE_ERROR_RECOVERY = 1,
        IMAPI_MODE_PAGE_TYPE_MRW = 3,
        IMAPI_MODE_PAGE_TYPE_WRITE_PARAMETERS = 5,
        IMAPI_MODE_PAGE_TYPE_CACHING = 8,
        IMAPI_MODE_PAGE_TYPE_POWER_CONDITION = 26,
        IMAPI_MODE_PAGE_TYPE_INFORMATIONAL_EXCEPTIONS = 28,
        IMAPI_MODE_PAGE_TYPE_TIMEOUT_AND_PROTECT = 29,
        IMAPI_MODE_PAGE_TYPE_LEGACY_CAPABILITIES = 42,
    }

    public enum IMAPI_MODE_PAGE_REQUEST_TYPE
    {
        IMAPI_MODE_PAGE_REQUEST_TYPE_CURRENT_VALUES = 0,
        IMAPI_MODE_PAGE_REQUEST_TYPE_CHANGABLE_VALUES = 1,
        IMAPI_MODE_PAGE_REQUEST_TYPE_DEFAULT_VALUES = 2,
        IMAPI_MODE_PAGE_REQUEST_TYPE_SAVED_VALUES = 3,
    }

    public enum IMAPI_READ_TRACK_ADDRESS_TYPE
    {
        IMAPI_READ_TRACK_ADDRESS_TYPE_LBA = 0,
        IMAPI_READ_TRACK_ADDRESS_TYPE_TRACK = 1,
        IMAPI_READ_TRACK_ADDRESS_TYPE_SESSION = 2,
    }

    public enum IMAPI_FORMAT2_DATA_MEDIA_STATE
    {
        [TypeLibVar(TypeLibVarFlags.FHidden)]
        IMAPI_FORMAT2_DATA_MEDIA_STATE_UNKNOWN = 0,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_OVERWRITE_ONLY = 1,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_RANDOMLY_WRITABLE = 1,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_BLANK = 2,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_APPENDABLE = 4,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_FINAL_SESSION = 8,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_INFORMATIONAL_MASK = 15,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_DAMAGED = 1024,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_ERASE_REQUIRED = 2048,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_NON_EMPTY_SESSION = 4096,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_WRITE_PROTECTED = 8192,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_FINALIZED = 16384,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MEDIA = 32768,
        IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MASK = 64512,
    }

    public enum IMAPI_FORMAT2_DATA_WRITE_ACTION
    {
        IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA = 0,
        IMAPI_FORMAT2_DATA_WRITE_ACTION_FORMATTING_MEDIA = 1,
        IMAPI_FORMAT2_DATA_WRITE_ACTION_INITIALIZING_HARDWARE = 2,
        IMAPI_FORMAT2_DATA_WRITE_ACTION_CALIBRATING_POWER = 3,
        IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA = 4,
        IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION = 5,
        IMAPI_FORMAT2_DATA_WRITE_ACTION_COMPLETED = 6,
    }

    public enum IMAPI_MEDIA_PHYSICAL_TYPE
    {
        IMAPI_MEDIA_TYPE_UNKNOWN = 0,
        IMAPI_MEDIA_TYPE_CDROM = 1,
        IMAPI_MEDIA_TYPE_CDR = 2,
        IMAPI_MEDIA_TYPE_CDRW = 3,
        IMAPI_MEDIA_TYPE_DVDROM = 4,
        IMAPI_MEDIA_TYPE_DVDRAM = 5,
        IMAPI_MEDIA_TYPE_DVDPLUSR = 6,
        IMAPI_MEDIA_TYPE_DVDPLUSRW = 7,
        IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER = 8,
        IMAPI_MEDIA_TYPE_DVDDASHR = 9,
        IMAPI_MEDIA_TYPE_DVDDASHRW = 10,
        IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER = 11,
        IMAPI_MEDIA_TYPE_DISK = 12,
        IMAPI_MEDIA_TYPE_DVDPLUSRW_DUALLAYER = 13,
        IMAPI_MEDIA_TYPE_HDDVDROM = 14,
        IMAPI_MEDIA_TYPE_HDDVDR = 15,
        IMAPI_MEDIA_TYPE_HDDVDRAM = 16,
        IMAPI_MEDIA_TYPE_BDROM = 17,
        IMAPI_MEDIA_TYPE_BDR = 18,
        IMAPI_MEDIA_TYPE_MAX = 19,
        IMAPI_MEDIA_TYPE_BDRE = 19,
    }

    public enum IMAPI_MEDIA_WRITE_PROTECT_STATE
    {
        IMAPI_WRITEPROTECTED_UNTIL_POWERDOWN = 1,
        IMAPI_WRITEPROTECTED_BY_CARTRIDGE = 2,
        IMAPI_WRITEPROTECTED_BY_MEDIA_SPECIFIC_REASON = 4,
        IMAPI_WRITEPROTECTED_BY_SOFTWARE_WRITE_PROTECT = 8,
        IMAPI_WRITEPROTECTED_BY_DISC_CONTROL_BLOCK = 16,
        IMAPI_WRITEPROTECTED_READ_ONLY_MEDIA = 16384,
    }

    public enum IMAPI_PROFILE_TYPE
    {
        IMAPI_PROFILE_TYPE_INVALID = 0,
        IMAPI_PROFILE_TYPE_NON_REMOVABLE_DISK = 1,
        IMAPI_PROFILE_TYPE_REMOVABLE_DISK = 2,
        IMAPI_PROFILE_TYPE_MO_ERASABLE = 3,
        IMAPI_PROFILE_TYPE_MO_WRITE_ONCE = 4,
        IMAPI_PROFILE_TYPE_AS_MO = 5,
        IMAPI_PROFILE_TYPE_CDROM = 8,
        IMAPI_PROFILE_TYPE_CD_RECORDABLE = 9,
        IMAPI_PROFILE_TYPE_CD_REWRITABLE = 10,
        IMAPI_PROFILE_TYPE_DVDROM = 16,
        IMAPI_PROFILE_TYPE_DVD_DASH_RECORDABLE = 17,
        IMAPI_PROFILE_TYPE_DVD_RAM = 18,
        IMAPI_PROFILE_TYPE_DVD_DASH_REWRITABLE = 19,
        IMAPI_PROFILE_TYPE_DVD_DASH_RW_SEQUENTIAL = 20,
        IMAPI_PROFILE_TYPE_DVD_DASH_R_DUAL_SEQUENTIAL = 21,
        IMAPI_PROFILE_TYPE_DVD_DASH_R_DUAL_LAYER_JUMP = 22,
        IMAPI_PROFILE_TYPE_DVD_PLUS_RW = 26,
        IMAPI_PROFILE_TYPE_DVD_PLUS_R = 27,
        IMAPI_PROFILE_TYPE_DDCDROM = 32,
        IMAPI_PROFILE_TYPE_DDCD_RECORDABLE = 33,
        IMAPI_PROFILE_TYPE_DDCD_REWRITABLE = 34,
        IMAPI_PROFILE_TYPE_DVD_PLUS_RW_DUAL = 42,
        IMAPI_PROFILE_TYPE_DVD_PLUS_R_DUAL = 43,
        IMAPI_PROFILE_TYPE_BD_ROM = 64,
        IMAPI_PROFILE_TYPE_BD_R_SEQUENTIAL = 65,
        IMAPI_PROFILE_TYPE_BD_R_RANDOM_RECORDING = 66,
        IMAPI_PROFILE_TYPE_BD_REWRITABLE = 67,
        IMAPI_PROFILE_TYPE_HD_DVD_ROM = 80,
        IMAPI_PROFILE_TYPE_HD_DVD_RECORDABLE = 81,
        IMAPI_PROFILE_TYPE_HD_DVD_RAM = 82,
        IMAPI_PROFILE_TYPE_NON_STANDARD = 65535,
    }

    public enum EmulationType
    {
        EmulationNone = 0,
        Emulation12MFloppy = 1,
        Emulation144MFloppy = 2,
        Emulation288MFloppy = 3,
        EmulationHardDisk = 4,
    }

    public enum PlatformId
    {
        PlatformX86 = 0,
        PlatformPowerPC = 1,
        PlatformMac = 2,
    }

    public enum FsiFileSystems
    {
        FsiFileSystemNone = 0,
        FsiFileSystemISO9660 = 1,
        FsiFileSystemJoliet = 2,
        FsiFileSystemUDF = 4,
        FsiFileSystemUnknown = 1073741824,
    }

    public enum FsiItemType
    {
        FsiItemNotFound = 0,
        FsiItemDirectory = 1,
        FsiItemFile = 2,
    }
}