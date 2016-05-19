// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace Microsoft.storage.EhStorage
{
    #region EnumEnhancedStorageACT
    /*---------------------------------------------------------
       This interface is used to enumerate available 
       addressable command targets (ACTs).
      ---------------------------------------------------------*/
    [
        ComImport,
        Guid("09b224bd-1335-4631-a7ff-cfd3a92646d7"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
    ]
    public interface IEnumEnhancedStorageACT
    {
        void GetACTs(out IntPtr enhancedStorageACTs, out UInt32 actCount);
        IEnhancedStorageACT GetMatchingACT([In, MarshalAs(UnmanagedType.LPWStr)] string volume);
    }
    [
        ComImport,
        Guid("09b224bd-1335-4631-a7ff-cfd3a92646d7"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
        CoClass(typeof(EnumEnhancedStorageACTClass))
    ]
    public interface EnumEnhancedStorageACT : IEnumEnhancedStorageACT
    {
    }

    [
        ComImport,
        Guid("fe841493-835c-4fa3-b6cc-b4b2d4719848"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class EnumEnhancedStorageACTClass
    {
    }
    #endregion

    #region EnhancedStorageACT
    /* This interface is used to access an ACT. */
    [
        ComImport,
        Guid("6e7781f4-e0f2-4239-b976-a01abab52930"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
    ]
    public interface IEnhancedStorageACT
    {
        void Authorize([In] UInt32 hwndParent, [In] UInt32 flags);

        void Unauthorize();

        ACT_AUTHORIZATION_STATE GetAuthorizationState();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetMatchingVolume();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetUniqueIdentity();

        void GetSilos(out IntPtr enhancedStorageSilos, out UInt32 siloCount);
    }

    [
        ComImport,
        Guid("6e7781f4-e0f2-4239-b976-a01abab52930"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
        CoClass(typeof(EnhancedStorageACTClass))
    ]
    public interface EnhancedStorageACT : IEnhancedStorageACT
    {
    }

    [
        ComImport,
        Guid("af076a15-2ece-4ad4-bb21-29f040e176d8"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class EnhancedStorageACTClass
    {
    }
    #endregion

    #region EnhancedStorageSilo
    /* This interface is used to access a silo. */
    [
        ComImport,
        Guid("5aef78c6-2242-4703-bf49-44b29357a359"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
    ]
    public interface IEnhancedStorageSilo
    {
        SILO_INFO GetInfo();

        void GetActions(out IntPtr enhancedStorageSiloActions, out UInt32 actionsCount);

        void SendCommand([In] Byte command,
                         [In, MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.I8)] Byte[] commandBuffer,
                         [In] UInt32 commandBufferSize,
                         [In, Out, MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.I8, SizeParamIndex = 4)] Byte[] responseBuffer,
                         [In, Out] ref UInt32 responseBufferSize);

        IPortableDevice GetPortableDevice();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetDevicePath();
    }

    [
        ComImport,
        Guid("5aef78c6-2242-4703-bf49-44b29357a359"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
        CoClass(typeof(EnhancedStorageSiloClass))
    ]
    public interface EnhancedStorageSilo : IEnhancedStorageSilo
    {
    }

    [
        ComImport,
        Guid("cb25220c-76c7-4fee-842b-f3383cd022bc"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class EnhancedStorageSiloClass
    {
    }
    #endregion

    #region EnhancedStorageSiloAction
    /* This interface is used to access a silo action. */
    [
        ComImport,
        Guid("b6f7f311-206f-4ff8-9c4b-27efee77a86f"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
    ]
    public interface IEnhancedStorageSiloAction
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetDescription();

        void Invoke();
    }

    [
        ComImport,
        Guid("b6f7f311-206f-4ff8-9c4b-27efee77a86f"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
        CoClass(typeof(EnhancedStorageSiloActionClass))
    ]
    public interface EnhancedStorageSiloAction : IEnhancedStorageSiloAction
    {
    }

    [
        ComImport,
        Guid("886D29DD-B506-466B-9FBF-B44FF383FB3F"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class EnhancedStorageSiloActionClass
    {
    }
    #endregion

    [StructLayout(LayoutKind.Sequential)]
    public struct ACT_AUTHORIZATION_STATE
    {
        public UInt32 ulState;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SILO_INFO
    {
        public UInt32 ulSTID;
        public Byte SpecificationMajor;
        public Byte SpecificationMinor;
        public Byte ImplementationMajor;
        public Byte ImplementationMinor;
        public Byte type;
        public Byte capabilities;
    }
}
