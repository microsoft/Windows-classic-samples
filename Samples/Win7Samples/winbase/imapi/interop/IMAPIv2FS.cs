using System;
using System.Collections;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.ComTypes;
using System.Runtime.InteropServices.CustomMarshalers;

namespace Storage.Interop.IMAPIv2FileSystem
{
    // Interfaces

    #region IFileSystemImage
    [TypeLibType(TypeLibTypeFlags.FDual |
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
        void SetMaxMediaBlocksFromDevice(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Number of blocks in use
        /// </summary>
        [DispId(3)]
        int UsedBlocks { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(4)]
        string VolumeName { get; set; }

        /// <summary>
        /// Imported Volume name
        /// </summary>
        [DispId(5)]
        string ImportedVolumeName { get; }

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
        string WorkingDirectory { get; set; }

        /// <summary>
        /// Change point identifier
        /// </summary>
        [DispId(10)]
        int ChangePoint { get; }

        /// <summary>
        /// Strict file system compliance option
        /// </summary>
        [DispId(11)]
        bool StrictFileSystemCompliance { get; set; }

        /// <summary>
        /// If true, indicates restricted character set is being used for file and directory names
        /// </summary>
        [DispId(12)]
        bool UseRestrictedCharacterSet { get; set; }

        /// <summary>
        /// File systems to create
        /// </summary>
        [DispId(13)]
        FileSystems FileSystemsToCreate { get; set; }

        /// <summary>
        /// File systems supported
        /// </summary>
        [DispId(14)]
        FileSystems FileSystemsSupported { get; }

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
        void ChooseImageDefaults(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Select filesystem types and image size based on the media type
        /// </summary>
        /// <param name="value"></param>
        [DispId(33)]
        void ChooseImageDefaultsForMediaType(Storage.Interop.IMAPIv2.MediaPhysicalType value);

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
        FSItemType Exists(string FullPath);

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
        FileSystems IdentifyFileSystemsOnDisc(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Identify which of the specified file systems would be imported by default
        /// </summary>
        /// <param name="fileSystems"></param>
        /// <returns></returns>
        [DispId(20)]
        FileSystems GetDefaultFileSystemForImport(FileSystems fileSystems);

        /// <summary>
        /// Import the default file system on the current disc
        /// </summary>
        /// <returns></returns>
        [DispId(21)]
        FileSystems ImportFileSystem();

        /// <summary>
        /// Import a specific file system on the current disc
        /// </summary>
        /// <param name="fileSystemToUse"></param>
        [DispId(22)]
        void ImportSpecificFileSystem(FileSystems fileSystemToUse);

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
        string VolumeNameUDF { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(28)]
        string VolumeNameJoliet { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(29)]
        string VolumeNameISO9660 { get; }

        /// <summary>
        /// Indicates whether or not IMAPI should stage the filesystem before the burn.
        /// Set to false to force IMAPI to not stage the filesystem prior to the burn.
        /// </summary>
        [DispId(30)]
        bool StageFiles { get; set; }

        /// <summary>
        /// available multi-session interfaces.
        /// </summary>
        [DispId(40)]
        Object[] MultisessionInterfaces { get; set; }
    }

    [TypeLibType(TypeLibTypeFlags.FDual |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("D7644B2C-1537-4767-B62F-F1387B02DDFD")]
    public interface IFileSystemImage2
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
        void SetMaxMediaBlocksFromDevice(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Number of blocks in use
        /// </summary>
        [DispId(3)]
        int UsedBlocks { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(4)]
        string VolumeName { get; set; }

        /// <summary>
        /// Imported Volume name
        /// </summary>
        [DispId(5)]
        string ImportedVolumeName { get; }

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
        string WorkingDirectory { get; set; }

        /// <summary>
        /// Change point identifier
        /// </summary>
        [DispId(10)]
        int ChangePoint { get; }

        /// <summary>
        /// Strict file system compliance option
        /// </summary>
        [DispId(11)]
        bool StrictFileSystemCompliance { get; set; }

        /// <summary>
        /// If true, indicates restricted character set is being used for file and directory names
        /// </summary>
        [DispId(12)]
        bool UseRestrictedCharacterSet { get; set; }

        /// <summary>
        /// File systems to create
        /// </summary>
        [DispId(13)]
        FileSystems FileSystemsToCreate { get; set; }

        /// <summary>
        /// File systems supported
        /// </summary>
        [DispId(14)]
        FileSystems FileSystemsSupported { get; }

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
        void ChooseImageDefaults(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Select filesystem types and image size based on the media type
        /// </summary>
        /// <param name="value"></param>
        [DispId(33)]
        void ChooseImageDefaultsForMediaType(Storage.Interop.IMAPIv2.MediaPhysicalType value);

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
        FSItemType Exists(string FullPath);

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
        FileSystems IdentifyFileSystemsOnDisc(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Identify which of the specified file systems would be imported by default
        /// </summary>
        /// <param name="fileSystems"></param>
        /// <returns></returns>
        [DispId(20)]
        FileSystems GetDefaultFileSystemForImport(FileSystems fileSystems);

        /// <summary>
        /// Import the default file system on the current disc
        /// </summary>
        /// <returns></returns>
        [DispId(21)]
        FileSystems ImportFileSystem();

        /// <summary>
        /// Import a specific file system on the current disc
        /// </summary>
        /// <param name="fileSystemToUse"></param>
        [DispId(22)]
        void ImportSpecificFileSystem(FileSystems fileSystemToUse);

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
        string VolumeNameUDF { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(28)]
        string VolumeNameJoliet { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(29)]
        string VolumeNameISO9660 { get; }

        /// <summary>
        /// Indicates whether or not IMAPI should stage the filesystem before the burn.
        /// Set to false to force IMAPI to not stage the filesystem prior to the burn.
        /// </summary>
        [DispId(30)]
        bool StageFiles { get; set; }

        /// <summary>
        /// available multi-session interfaces.
        /// </summary>
        [DispId(40)]
        Object[] MultisessionInterfaces { get; set; }

        // IFileSystemImage2
        /// <summary>
        /// Get or Set boot options array for supporting multi-boot
        /// </summary>
        [DispId(60)]
        Object[] BootImageOptionsArray { get; set; }
    }

    [TypeLibType(TypeLibTypeFlags.FDual |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("7CFF842C-7E97-4807-8304-910DD8F7C051")]
    public interface IFileSystemImage3
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
        void SetMaxMediaBlocksFromDevice(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Number of blocks in use
        /// </summary>
        [DispId(3)]
        int UsedBlocks { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(4)]
        string VolumeName { get; set; }

        /// <summary>
        /// Imported Volume name
        /// </summary>
        [DispId(5)]
        string ImportedVolumeName { get; }

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
        string WorkingDirectory { get; set; }

        /// <summary>
        /// Change point identifier
        /// </summary>
        [DispId(10)]
        int ChangePoint { get; }

        /// <summary>
        /// Strict file system compliance option
        /// </summary>
        [DispId(11)]
        bool StrictFileSystemCompliance { get; set; }

        /// <summary>
        /// If true, indicates restricted character set is being used for file and directory names
        /// </summary>
        [DispId(12)]
        bool UseRestrictedCharacterSet { get; set; }

        /// <summary>
        /// File systems to create
        /// </summary>
        [DispId(13)]
        FileSystems FileSystemsToCreate { get; set; }

        /// <summary>
        /// File systems supported
        /// </summary>
        [DispId(14)]
        FileSystems FileSystemsSupported { get; }

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
        void ChooseImageDefaults(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Select filesystem types and image size based on the media type
        /// </summary>
        /// <param name="value"></param>
        [DispId(33)]
        void ChooseImageDefaultsForMediaType(Storage.Interop.IMAPIv2.MediaPhysicalType value);

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
        FSItemType Exists(string FullPath);

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
        FileSystems IdentifyFileSystemsOnDisc(Storage.Interop.IMAPIv2.IDiscRecorder2 discRecorder);

        /// <summary>
        /// Identify which of the specified file systems would be imported by default
        /// </summary>
        /// <param name="fileSystems"></param>
        /// <returns></returns>
        [DispId(20)]
        FileSystems GetDefaultFileSystemForImport(FileSystems fileSystems);

        /// <summary>
        /// Import the default file system on the current disc
        /// </summary>
        /// <returns></returns>
        [DispId(21)]
        FileSystems ImportFileSystem();

        /// <summary>
        /// Import a specific file system on the current disc
        /// </summary>
        /// <param name="fileSystemToUse"></param>
        [DispId(22)]
        void ImportSpecificFileSystem(FileSystems fileSystemToUse);

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
        string VolumeNameUDF { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(28)]
        string VolumeNameJoliet { get; }

        /// <summary>
        /// Volume name
        /// </summary>
        [DispId(29)]
        string VolumeNameISO9660 { get; }

        /// <summary>
        /// Indicates whether or not IMAPI should stage the filesystem before the burn.
        /// Set to false to force IMAPI to not stage the filesystem prior to the burn.
        /// </summary>
        [DispId(30)]
        bool StageFiles { get; set; }

        /// <summary>
        /// available multi-session interfaces.
        /// </summary>
        [DispId(40)]
        object[] MultisessionInterfaces { get; set; }

        // IFileSystemImage2
        /// <summary>
        /// Get or Set boot options array for supporting multi-boot
        /// </summary>
        [DispId(60)]
        object[] BootImageOptionsArray { get; set; }

        // IFileSystemImage3

        /// <summary>
        /// Set CreateRedundantUdfMetadataFiles property
        /// If true, indicates that UDF Metadata and Metadata Mirror
        /// files are truly redundant, i.e. reference different extents
        /// </summary>
        [DispId(61)]
        bool CreateRedundantUdfMetadataFiles { [return: MarshalAs(UnmanagedType.VariantBool)] get; set; }

        /// <summary>
        /// Probe if a specific file system on the disc is appendable through IMAPI
        /// </summary>
        [DispId(70)]
        bool ProbeSpecificFileSystem(FileSystems fileSystemToProbe);
    }

    #endregion

    [TypeLibType(TypeLibTypeFlags.FDual |
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
        string DiscId { get; }
    }

    [TypeLibType(TypeLibTypeFlags.FDual |
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
        string Manufacturer { get; set; }

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

    [TypeLibType(TypeLibTypeFlags.FDual |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FD9-975B-59BE-A960-9A2A262853A5")]
    public interface IFsiItem
    {
        /// <summary>
        /// Item name
        /// </summary>
        [DispId(11)]
        string Name { get; }

        /// <summary>
        /// Path
        /// </summary>
        [DispId(12)]
        string FullPath { get; }

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
        bool IsHidden { get; set; }

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(17)]
        string FileSystemName(FileSystems fileSystem);

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(18)]
        string FileSystemPath(FileSystems fileSystem);
    }

    [TypeLibType(TypeLibTypeFlags.FDual |
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
        string Name { get; }

        /// <summary>
        /// Path
        /// </summary>
        [DispId(12)]
        string FullPath { get; }

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
        bool IsHidden { get; set; }

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(17)]
        string FileSystemName(FileSystems fileSystem);

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(18)]
        string FileSystemPath(FileSystems fileSystem);

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

    [TypeLibType(TypeLibTypeFlags.FDual |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("199D0C19-11E1-40eb-8EC2-C8C822A07792")]
    public interface IFsiFileItem2
    {
        // IFsiItem
        /// <summary>
        /// Item name
        /// </summary>
        [DispId(11)]
        string Name { get; }

        /// <summary>
        /// Path
        /// </summary>
        [DispId(12)]
        string FullPath { get; }

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
        bool IsHidden { get; set; }

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(17)]
        string FileSystemName(FileSystems fileSystem);

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(18)]
        string FileSystemPath(FileSystems fileSystem);

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

        // IFsiFileItem2

        /// <summary>
        /// Get the list of the named streams of the file
        /// </summary>
        [DispId(45)]
        IFsiNamedStreams FsiNamedStreams { get;}

        /// <summary>
        /// Flag indicating if file item is a named stream of a file
        /// </summary>
        [DispId(46)]
        bool IsNamedStream { get;}

        /// <summary>
        /// Add a new named stream to the collection
        /// </summary>
        [DispId(47)]
        void AddStream(string name, IStream streamData);

        /// <summary>
        /// Remove a specific named stream of the collection
        /// </summary>
        [DispId(48)]
        void RemoveStream(string name);

        /// <summary>
        /// Flag indicating if file is Real-Time
        /// </summary>
        [DispId(49)]
        bool IsRealTime { get; set;}
    };

    [TypeLibType(TypeLibTypeFlags.FDual |
             TypeLibTypeFlags.FDispatchable |
             TypeLibTypeFlags.FNonExtensible)]
    [Guid("ED79BA56-5294-4250-8D46-F9AECEE23459")]
    public interface IFsiNamedStreams : IEnumerable
    {
        /// <summary>
        /// Get an enumerator for the collection
        /// </summary>
        /// <returns></returns>
        [DispId(-4)]
        [return: MarshalAs(UnmanagedType.CustomMarshaler,
                 MarshalTypeRef=typeof(EnumeratorToEnumVariantMarshaler))]
        new IEnumerator GetEnumerator();

        /// <summary>
        /// Get a named stream from the collection
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        [DispId(0)]
        IFsiFileItem2 this[int index] { [return: MarshalAs(UnmanagedType.Interface)] get; }

        /// <summary>
        /// Number of named streams in the collection
        /// </summary>
        [DispId(81)]
        int Count { get; }

        /// <summary>
        /// Get a non-variant enumerator for the named stream collection
        /// </summary>
        [DispId(82)]
        IEnumFsiItems EnumNamedStreams { [return: MarshalAs(UnmanagedType.Interface)] get; }
    };

    [TypeLibType(TypeLibTypeFlags.FDual |
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
        string Name { get; }

        /// <summary>
        /// Path
        /// </summary>
        [DispId(12)]
        string FullPath { get; }

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
        bool IsHidden { get; set; }

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(17)]
        string FileSystemName(FileSystems fileSystem);

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(18)]
        string FileSystemPath(FileSystems fileSystem);

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

    [TypeLibType(TypeLibTypeFlags.FDual |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("F7FB4B9B-6D96-4d7b-9115-201B144811EF")]
    public interface IFsiDirectoryItem2
    {
        // IFsiItem
        /// <summary>
        /// Item name
        /// </summary>
        [DispId(11)]
        string Name { get; }

        /// <summary>
        /// Path
        /// </summary>
        [DispId(12)]
        string FullPath { get; }

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
        bool IsHidden { get; set; }

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(17)]
        string FileSystemName(FileSystems fileSystem);

        /// <summary>
        /// Name of item in the specified file system
        /// </summary>
        /// <param name="fileSystem"></param>
        /// <returns></returns>
        [DispId(18)]
        string FileSystemPath(FileSystems fileSystem);

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

        // IFsiDirectoryItem2
        /// <summary>
        /// Add files and directories from the specified source directory
        /// including named streams
        /// </summary>       
        [DispId(36)]
        void AddTreeWithNamedStreams(string sourceDirectory, bool includeBaseDirectory);
    };

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

    [TypeLibType(TypeLibTypeFlags.FDual |
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

    [TypeLibType(TypeLibTypeFlags.FDual |
                 TypeLibTypeFlags.FDispatchable |
                 TypeLibTypeFlags.FNonExtensible)]
    [Guid("2C941FD5-975B-59BE-A960-9A2A262853A5")]
    public interface IProgressItem
    {
        /// <summary>
        /// Progress item description
        /// </summary>
        [DispId(1)]
        string Description { get; }

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

    // CoClass - Specifies the class identifier of a coclass 
    // imported from a type library.
    #region MsftFileSystemImage
    [CoClass(typeof(MsftFileSystemImageClass)), ComImport]
    [Guid("2C941FE1-975B-59BE-A960-9A2A262853A5")]
    public interface MsftFileSystemImage : IFileSystemImage, DFileSystemImage_Events
    {
    }
    [CoClass(typeof(MsftFileSystemImageClass)), ComImport]
    [Guid("D7644B2C-1537-4767-B62F-F1387B02DDFD")]
    public interface MsftFileSystemImage2 : IFileSystemImage2, DFileSystemImage_Events
    {
    }
    [CoClass(typeof(MsftFileSystemImageClass)), ComImport]
    [Guid("7CFF842C-7E97-4807-8304-910DD8F7C051")]
    public interface MsftFileSystemImage3 : IFileSystemImage3, DFileSystemImage_Events, DFileSystemImageImport_Events
    {
    }
    #endregion

    #region DFileSystemImageEvents
    
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void IMAPIEventHandlerDelegate<TSender, TArg1, TArg2, TArg3>(TSender sender, TArg1 arg1, TArg2 arg2, TArg3 arg3)
        where TSender : class;

    [
        ComEventInterface(typeof(DFileSystemImageEvents),
                          typeof(DFileSystemImage_EventsProvider)),
        TypeLibType(TypeLibTypeFlags.FHidden)
    ]
    public interface DFileSystemImage_Events
    {
        event IMAPIEventHandlerDelegate<IFileSystemImage, String, long, long> Update;
    }


    [
        ComImport,
        Guid("2C941FDF-975B-59BE-A960-9A2A262853A5"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface DFileSystemImageEvents
    {
        [DispId(0x100)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
        void Update([In, MarshalAs(UnmanagedType.IDispatch)] IFileSystemImage sender,
                    [In, MarshalAs(UnmanagedType.BStr)] String currentFile,
                    long copiedSectors,
                    long totalSectors);
    }

    [
        TypeLibType(TypeLibTypeFlags.FHidden),
        ClassInterface(ClassInterfaceType.None)
    ]
    public sealed class DFileSystemImage_SinkHelper : DFileSystemImageEvents
    {
        public void Update([In, MarshalAs(UnmanagedType.IDispatch)] IFileSystemImage sender,
                    [In, MarshalAs(UnmanagedType.BStr)] String currentFile,
                    long copiedSectors,
                    long totalSectors)
        {
            UpdateDelegate(sender, currentFile, copiedSectors, totalSectors);
        }

        public int Cookie;

        public IMAPIEventHandlerDelegate<IFileSystemImage, String, long, long> UpdateDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class DFileSystemImage_EventsProvider : DFileSystemImage_Events, IDisposable
    {
        public DFileSystemImage_EventsProvider(Object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    Guid eventsGuid = typeof(DFileSystemImageEvents).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        public event IMAPIEventHandlerDelegate<IFileSystemImage, String, long, long> Update
        {
            add
            {
                lock (this)
                {
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new DFileSystemImage_SinkHelper();
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
        ~DFileSystemImage_EventsProvider()
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
        private DFileSystemImage_SinkHelper m_SinkHelper = null;
    }
    #endregion

    #region DFileSystemImageImportEvents

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void IMAPIImportEventHandlerDelegate<TSender, TArg1, TArg2, TArg3, TArg4, TArg5, TArg6>(TSender sender, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6)
        where TSender : class;

    [
        ComEventInterface(typeof(DFileSystemImageImportEvents),
                          typeof(DFileSystemImageImport_EventsProvider)),
        TypeLibType(TypeLibTypeFlags.FHidden)
    ]
    public interface DFileSystemImageImport_Events
    {
        event IMAPIImportEventHandlerDelegate<IFileSystemImage, FileSystems, String, long, long, long, long> UpdateImport;
    }


    [
        ComImport,
        Guid("D25C30F9-4087-4366-9E24-E55BE286424B"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]
    public interface DFileSystemImageImportEvents
    {
        [DispId(0x101)]
        [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
        void UpdateImport([In, MarshalAs(UnmanagedType.IDispatch)] IFileSystemImage sender,
                          [In, MarshalAs(UnmanagedType.I4)] FileSystems fileSystem,
                          [In, MarshalAs(UnmanagedType.BStr)] String currentItem,
                          long importedDirectoryItems,
                          long totalDirectoryItems,
                          long importedFileItems,
                          long totalFileItems);
    }

    [
        TypeLibType(TypeLibTypeFlags.FHidden),
        ClassInterface(ClassInterfaceType.None)
    ]
    public sealed class DFileSystemImageImport_SinkHelper : DFileSystemImageImportEvents
    {
        public void UpdateImport([In, MarshalAs(UnmanagedType.IDispatch)] IFileSystemImage sender,
                          [In, MarshalAs(UnmanagedType.I4)] FileSystems fileSystem,
                          [In, MarshalAs(UnmanagedType.BStr)] String currentItem,
                          long importedDirectoryItems,
                          long totalDirectoryItems,
                          long importedFileItems,
                          long totalFileItems)
        {
            UpdateImportDelegate(sender, fileSystem, currentItem, importedDirectoryItems, totalDirectoryItems, importedFileItems, totalFileItems);
        }

        public int Cookie;

        public IMAPIImportEventHandlerDelegate<IFileSystemImage, FileSystems, String, long, long, long, long> UpdateImportDelegate;
    }

    [ClassInterface(ClassInterfaceType.None)]
    internal sealed class DFileSystemImageImport_EventsProvider : DFileSystemImageImport_Events, IDisposable
    {
        public DFileSystemImageImport_EventsProvider(Object pointContainer)
        {
            lock (this)
            {
                if (m_ConnectionPoint == null)
                {
                    Guid eventsGuid = typeof(DFileSystemImageImportEvents).GUID;
                    IConnectionPointContainer connectionPointContainer = pointContainer as IConnectionPointContainer;

                    connectionPointContainer.FindConnectionPoint(ref eventsGuid, out m_ConnectionPoint);
                }
            }
        }

        public event IMAPIImportEventHandlerDelegate<IFileSystemImage, FileSystems, String, long, long, long, long> UpdateImport
        {
            add
            {
                lock (this)
                {
                    if (m_SinkHelper == null)
                    {
                        m_SinkHelper = new DFileSystemImageImport_SinkHelper();
                    }

                    // Add the passed-in value to the sink helper's delegate list 
                    m_SinkHelper.UpdateImportDelegate += value;

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
                        m_SinkHelper.UpdateImportDelegate == null)    //.GetInvocationList().Length == 0)
                    {
                        // TODO: ERROR, cannot remove w/o adding
                    }
                    else
                    {
                        m_SinkHelper.UpdateImportDelegate -= value;
                        if (m_SinkHelper.UpdateImportDelegate == null)    //m_SinkHelper.UpdateDelegate.GetInvocationList().Length == 0)
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
        ~DFileSystemImageImport_EventsProvider()
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
        private DFileSystemImageImport_SinkHelper m_SinkHelper = null;
    }
    #endregion

    // Class
    [TypeLibType(TypeLibTypeFlags.FCanCreate)]
    [ClassInterface(ClassInterfaceType.None)]
    [ComSourceInterfaces("DFileSystemImageEvents\0")]
    [Guid("2C941FC5-975B-59BE-A960-9A2A262853A5"), ComImport]
    public class MsftFileSystemImageClass
    {
    }

    [CoClass(typeof(MsftBootOptionsClass)), ComImport]
    [Guid("2C941FD4-975B-59BE-A960-9A2A262853A5")]
    public interface MsftBootOptions : IBootOptions
    {
    }

    [TypeLibType(TypeLibTypeFlags.FCanCreate)]
    [ClassInterface(ClassInterfaceType.None)]
    [Guid("2C941FCE-975B-59BE-A960-9A2A262853A5"), ComImport]
    public class MsftBootOptionsClass
    {
    }

    #region IIsoImageManager
    [
        ComImport,
        Guid("6CA38BE5-FBBB-4800-95A1-A438865EB0D4"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
        // ISO Image Manager: Helper object for ISO image file manipulation
    ]
    public interface IIsoImageManager
    {
        [DispId(0x100)]
        // Path to the ISO image file
        String Path { [return: MarshalAs(UnmanagedType.BStr)] get;}

        [DispId(0x101)]
        // Stream from the ISO image
        IStream Stream { [return: MarshalAs(UnmanagedType.Interface)] get;}

        [DispId(0x200)]
        // Set path to the ISO image file, overwrites stream
        void SetPath([In, MarshalAs(UnmanagedType.BStr)] String Val);

        [DispId(0x201)]
        // Set stream from the ISO image, overwrites path
        void SetStream([In, MarshalAs(UnmanagedType.Interface)] IStream data);

        [DispId(0x202)]
        // Validate if the ISO image file is a valid file
        void IsValid();
    }

    [CoClass(typeof(MsftIsoImageManagerClass)), ComImport]
    [Guid("6CA38BE5-FBBB-4800-95A1-A438865EB0D4")]
    public interface MsftIsoImageManager : IIsoImageManager
    {
    }

    [TypeLibType(TypeLibTypeFlags.FCanCreate)]
    [ClassInterface(ClassInterfaceType.None)]
    [Guid("CEEE3B62-8F56-4056-869B-EF16917E3EFC"), ComImport]
    public class MsftIsoImageManagerClass
    {
    }
    #endregion IIsoImageManager

    #region Enumerators
    public enum EmulationType
    {
        None       = 0,
        Floppy12M  = 1,
        Floppy144M = 2,
        Floppy288M = 3,
        HardDisk   = 4,
    }

    public enum PlatformId
    {
        x86     = 0,
        PowerPC = 1,
        Mac     = 2,
        EFI     = 0xef,
    }

    [Flags]
    public enum FileSystems
    {
        None    = 0x000000000,
        ISO9660 = 0x000000001,
        Joliet  = 0x000000002,
        UDF     = 0x000000004,
        Unknown = 0x040000000
    }

    public enum FSItemType
    {
        NotFound  = 0,
        Directory = 1,
        File      = 2,
    }

    #endregion Enumerators
}
