using System;
using System.Threading;
using System.Collections;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.ComTypes;
using System.Runtime.InteropServices.CustomMarshalers;

using System.Text.RegularExpressions;

namespace Storage.Interop.IMAPIv2Raw
{
    #region Interfaces
    /// <summary>Use this interface to enumerate the CD and DVD devices installed on the computer.</summary>
    [
        ComImport,
        Guid("25983550-9D65-49CE-B335-40630D901227"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]

    public interface IRawCDImageCreator
    {
        // Methods
        [DispId(0x200)]
        IStream CreateResultImage();

        [DispId(0x201)]
        int AddTrack(CDSectorType dataType, IStream data);

        [DispId(0x202)]
        void AddSpecialPregap(IStream data);

        [DispId(0x203)]
        void AddSubcodeRWGenerator(IStream subcode);

        // Properties

        [DispId(0x100)]
        Storage.Interop.IMAPIv2.Format2RawCDDataSectorType ResultingImageType{ set; get; }

        [DispId(0x101)]
        int StartOfLeadout{ get; }

        [DispId(0x102)]
        int StartOfLeadoutLimit { set; get; }

        [DispId(0x103)]
        bool DisableGaplessAudio { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x104)]
        String MediaCatalogNumber { set; [return: MarshalAs(UnmanagedType.BStr)] get; }

        // BUGBUG: How to do range??? [in,range(1,99)] LONG value
        [DispId(0x105)]
        int StartingTrackNumber { set; get; }

        //[DispId(0x106)]
        //IRawCDImageTrackInfo TrackInfo{ int trackIndex get; }

        [DispId(0x107)]
        int NumberOfExistingTracks { get; }

        [DispId(0x108)]
        int LastUsedUserSectorInImage { get; }

        [DispId(0x109)]
        String[] ExpectedTableOfContents { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

    }



    /// <summary>Use this interface to enumerate the CD and DVD devices installed on the computer.</summary>
    [
        ComImport,
        Guid("25983551-9D65-49CE-B335-40630D901227"),
        InterfaceType(ComInterfaceType.InterfaceIsDual)
    ]

    public interface IRawCDImageTrackInfo
    {
        // Properties

        [DispId(0x100)]
        int StartingLba { get; }

        [DispId(0x101)]
        int SectorCount { get; }

        [DispId(0x102)]
        int TrackNumber { get; }

        [DispId(0x103)]
        CDSectorType SectorType { get; }

        [DispId(0x104)]
        bool ISRC { set; [return: MarshalAs(UnmanagedType.BStr)] get; }

        [DispId(0x105)]
        CDTrackDigitalCopySetting DigitalAudioCopySetting { set; get; }

        [DispId(0x106)]
        bool AudioHasPreemphasis { set; [return: MarshalAs(UnmanagedType.VariantBool)] get; }

        [DispId(0x107)]
        String[] TrackIndexes { [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_VARIANT)] get; }

        // Methods

        // range [in,range(0,0x7FFFFFFF)] LONG
        [DispId(0x200)]
        void  AddTrackIndex( int lbaOffset);

        // range [in, range(0,0x7FFFFFFF)] LONG
        [DispId(0x201)]
        void ClearTrackIndex( int lbaOffset);


    }

    #endregion Interfaces

    #region CoClass

    /// <summary>
    /// 
    /// </summary>
    [
        ComImport,
        Guid("25983550-9D65-49CE-B335-40630D901227"),
        CoClass(typeof(MsftRawCDImageCreatorClass))
    ]
    public interface MsftRawCDImageCreator : IRawCDImageCreator
    {
    }

    #endregion CoClass

    #region Class

    /// <summary>
    /// 
    /// </summary>
    [
        ComImport,
        Guid("25983561-9D65-49CE-B335-40630D901227"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate)
    ]
    public class MsftRawCDImageCreatorClass
    {
    }

    #endregion Class

    #region Enumerators

    [Serializable]
    public enum CDSectorType : int
    {
        Audio = 0x00,
        ModeZero = 0x01,
        Mode1 = 0x02,
        Mode2Form0 = 0x03,
        Mode2Form1 = 0x04,
        Mode2Form2 = 0x05,
        Mode1Raw = 0x06,
        Mode2Form0Raw = 0x07,
        Mode2Form1Raw = 0x08,
        Mode2Form2Raw = 0x09,
    }

    [Serializable]
    public enum CDTrackDigitalCopySetting : int
    {
        Permitted  = 0x00,
        Prohibited = 0x01,
        SCMS       = 0x02,
    }

    #endregion Enumerators

}