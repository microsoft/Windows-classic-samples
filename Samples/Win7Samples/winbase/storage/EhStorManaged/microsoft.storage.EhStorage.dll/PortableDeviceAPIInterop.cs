// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// Note: Portable device API interop has been implemented for the needs of Enhanced Storage.
// Some functions and interfaces may not be present or present in limited form.

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace Microsoft.storage.EhStorage
{
    #region PortableDeviceManager
    [
        ComImport,
        Guid("a1567595-4c2f-4574-a6fa-ecef917b9a40"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
    ]
    public interface IPortableDeviceManager
    {
        // MIDL:  HRESULT GetDevices(LPWSTR* pPnPDeviceIDs, DWORD* pcPnPDeviceIDs);
        // pcPnPDeviceIDs - returning value, PnPDeviceIDs might be 'null' to retreive devices count
        void GetDevices(
            [In, Out, MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr, SizeParamIndex = 1)] string[] PnPDeviceIDs,
            ref Int32 count);

        void RefreshDeviceList();

        void GetDeviceFriendlyName(
            [In, MarshalAs(UnmanagedType.LPWStr)] string PnPDeviceID,
            [In, Out, MarshalAs(UnmanagedType.LPWStr)] StringBuilder deviceFriendlyName,
            [In, Out] ref uint CharsCount);

        void GetDeviceDescription(
            [In, MarshalAs(UnmanagedType.LPWStr)] string PnPDeviceID,
            [In, Out, MarshalAs(UnmanagedType.LPWStr)] StringBuilder deviceDescription,
            [In, Out] ref uint CharsCount);

        void GetDeviceManufacturer(
            [In, MarshalAs(UnmanagedType.LPWStr)] string PnPDeviceID,
            [In, Out, MarshalAs(UnmanagedType.LPWStr)] StringBuilder deviceManufacturer,
            [In, Out] ref uint CharsCount);

        /*
        HRESULT GetDeviceProperty(
            [in]                LPCWSTR pszPnPDeviceID,
            [in]                LPCWSTR pszDevicePropertyName,
            [in, out, unique]   BYTE*   pData,
            [in, out, unique]   DWORD*  pcbData,
            [in, out, unique]   DWORD*  pdwType);

        HRESULT GetPrivateDevices(
            [in, out, unique]   LPWSTR* pPnPDeviceIDs,
            [in, out]           DWORD*  pcPnPDeviceIDs);
         */
    }

    [
        ComImport,
        Guid("a1567595-4c2f-4574-a6fa-ecef917b9a40"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
        CoClass(typeof(PortableDeviceManagerClass))
    ]
    public interface PortableDeviceManager : IPortableDeviceManager
    {
    }

    [
        ComImport,
        Guid("0af10cec-2ecd-4b92-9581-34f6ae0637f3"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class PortableDeviceManagerClass
    {
    }
    #endregion

    #region PortableDevice

    [
        ComImport,
        Guid("625e2df8-6392-4cf0-9ad1-3cfa5f17775c"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
    ]
    public interface IPortableDevice
    {
        void Open(
            [In, MarshalAs(UnmanagedType.LPWStr)] string PnPDeviceID,
            [In] IPortableDeviceValues clientInfo);

        IPortableDeviceValues SendCommand(
            UInt32 flags,
            [In] IPortableDeviceValues parameters);

        void Content(IntPtr ppContent);

        void Capabilities(IntPtr ppCapabilities);
        
        void Cancel();

        void Close();

        /*
         * Not implemented, not required for Enhanced Storage
        HRESULT Advise(
            [in]   const DWORD                      dwFlags,
            [in]   IPortableDeviceEventCallback*    pCallback,
            [in, unique]   IPortableDeviceValues*   pParameters,
            [out]  LPWSTR*                          ppszCookie);
         */
        void Advise();

        void Unadvise([In, MarshalAs(UnmanagedType.LPWStr)] string cookie);

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetPnPDeviceID();
    }

    [
        ComImport,
        Guid("625e2df8-6392-4cf0-9ad1-3cfa5f17775c"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
        CoClass(typeof(PortableDeviceClass))
    ]
    public interface PortableDevice : IPortableDevice
    {
    }

    [
        ComImport,
        Guid("728a21c5-3d9e-48d7-9810-864848f0f404"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class PortableDeviceClass
    {
    }
    #endregion

    #region PortableDeviceValues
    [
        ComImport,
        Guid("6848f6f2-3155-4f86-b6f5-263eeeab3143"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
    ]
    public interface IPortableDeviceValues
    {
        UInt32 GetCount();
        void GetAt([In] UInt32 index, [Out] out PROPERTYKEY key, [Out] out PropVariant value);

        void SetValue([In] ref PROPERTYKEY key, [In] ref PropVariant value);
        void GetValue([In] ref PROPERTYKEY key, [Out] out PropVariant value);

        void SetStringValue([In] ref PROPERTYKEY key, [In, MarshalAs(UnmanagedType.LPWStr)] string value);
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetStringValue([In] ref PROPERTYKEY key);

        void SetUnsignedIntegerValue([In] ref PROPERTYKEY key, [In] UInt32 value);
        UInt32 GetUnsignedIntegerValue([In] ref PROPERTYKEY key);

        void SetSignedIntegerValue([In] ref PROPERTYKEY key, [In] Int32 value);
        Int32 GetSignedIntegerValue([In] ref PROPERTYKEY key);

        void SetUnsignedLargeIntegerValue([In] ref PROPERTYKEY key, [In] UInt64 value);
        UInt64 GetUnsignedLargeIntegerValue([In] ref PROPERTYKEY key);

        void SetSignedLargeIntegerValue([In] ref PROPERTYKEY key, [In] Int64 value);
        Int64 GetSignedLargeIntegerValue([In] ref PROPERTYKEY key);

        void SetFloatValue([In] ref PROPERTYKEY key, [In] float value);
        float GetFloatValue([In] ref PROPERTYKEY key);

        void SetErrorValue([In] ref PROPERTYKEY key, [In] Int32 value);
        Int32 GetErrorValue([In] ref PROPERTYKEY key);

        void SetKeyValue([In] ref PROPERTYKEY key, [In] ref PROPERTYKEY value);
        PROPERTYKEY GetKeyValue([In] ref PROPERTYKEY key);

        void SetBoolValue([In] ref PROPERTYKEY key, [In] Int32 value);
        Int32 GetBoolValue([In] ref PROPERTYKEY key);

        void SetIUnknownValue([In] ref PROPERTYKEY key, [In] IntPtr value);
        IntPtr GetIUnknownValue([In] ref PROPERTYKEY key);

        void SetGuidValue([In] ref PROPERTYKEY key, [In] ref Guid value);
        Guid GetGuidValue([In] ref PROPERTYKEY key);

        void SetBufferValue([In] ref PROPERTYKEY key, [In] IntPtr value, [In] Int32 valueLen);
        void GetBufferValue([In] ref PROPERTYKEY key, [Out] out IntPtr value, [Out] out Int32 valueLen);

        void SetIPortableDeviceValuesValue([In] ref PROPERTYKEY key, [In]IPortableDeviceValues value);
        void GetIPortableDeviceValuesValue([In] ref PROPERTYKEY key, [Out] out IPortableDeviceValues value);

        /*
         * The Get/Get pairs below are not used for Enhanced Storage and corresponding interfaces are not implemented
    HRESULT SetIPortableDevicePropVariantCollectionValue(
    [in]    REFPROPERTYKEY                        key,
    [in]    IPortableDevicePropVariantCollection* pValue);

    HRESULT GetIPortableDevicePropVariantCollectionValue(
        [in]    REFPROPERTYKEY                          key,
        [out]   IPortableDevicePropVariantCollection**  ppValue);

    HRESULT SetIPortableDeviceKeyCollectionValue(
        [in]    REFPROPERTYKEY                key,
        [in]    IPortableDeviceKeyCollection* pValue);

    HRESULT GetIPortableDeviceKeyCollectionValue(
        [in]    REFPROPERTYKEY                  key,
        [out]   IPortableDeviceKeyCollection**  ppValue);

    HRESULT SetIPortableDeviceValuesCollectionValue(
        [in]    REFPROPERTYKEY                   key,
        [in]    IPortableDeviceValuesCollection* pValue);

    HRESULT GetIPortableDeviceValuesCollectionValue(
        [in]    REFPROPERTYKEY                    key,
        [out]   IPortableDeviceValuesCollection** ppValue);

    HRESULT RemoveValue(
        [in] REFPROPERTYKEY key);

    HRESULT CopyValuesFromPropertyStore(
        [in] IPropertyStore* pStore);

    HRESULT CopyValuesToPropertyStore(
        [in] IPropertyStore* pStore);

    HRESULT Clear();
         */
    }

    [
        ComImport,
        Guid("6848f6f2-3155-4f86-b6f5-263eeeab3143"),
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
        CoClass(typeof(PortableDeviceValuesClass))
    ]
    public interface PortableDeviceValues : IPortableDeviceValues
    {
    }

    [
        ComImport,
        Guid("0c15d503-d017-47ce-9016-7b3f978721cc"),
        ClassInterface(ClassInterfaceType.None),
        TypeLibType(TypeLibTypeFlags.FCanCreate),
    ]
    public class PortableDeviceValuesClass
    {
    }
    #endregion

    #region PropertyKeys

    [StructLayout(LayoutKind.Sequential)]
    public struct PROPERTYKEY
    {
        public Guid fmtid;
        public UInt32 pid;

        public static bool operator ==(PROPERTYKEY p1, PROPERTYKEY p2)
        {
            return p1.Equals(p2);
        }

        public static bool operator !=(PROPERTYKEY p1, PROPERTYKEY p2)
        {
            return !p1.Equals(p2);
        }

        public override bool Equals(object obj)
        {
            if (!(obj is PROPERTYKEY))
            {
                return false;
            }
            return Equals((PROPERTYKEY)obj);
        }

        public bool Equals(PROPERTYKEY other)
        {
            return ((fmtid == other.fmtid) && (pid == other.pid));
        }

        public override int GetHashCode() 
        {
            return fmtid.GetHashCode() ^ (int)pid;
        }
    }

    public class PortableDevicePKeys
    {
        static PortableDevicePKeys()
        {
            // WPD_OBJECT_PROPERTIES_V1
            WPD_OBJECT_ID.fmtid = new Guid(0xEF6B490D, 0x5CD8, 0x437A, 0xAF, 0xFC, 0xDA, 0x8B, 0x60, 0xEE, 0x4A, 0x3C);
            WPD_OBJECT_ID.pid = 2;

            // WPD_CLIENT_INFORMATION_PROPERTIES_V1
            WPD_CLIENT_NAME.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_NAME.pid = 2;

            WPD_CLIENT_MAJOR_VERSION.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_MAJOR_VERSION.pid = 3;

            WPD_CLIENT_MINOR_VERSION.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_MINOR_VERSION.pid = 4;

            WPD_CLIENT_REVISION.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_REVISION.pid = 5;

            WPD_CLIENT_WMDRM_APPLICATION_PRIVATE_KEY.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_WMDRM_APPLICATION_PRIVATE_KEY.pid = 6;

            WPD_CLIENT_WMDRM_APPLICATION_CERTIFICATE.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_WMDRM_APPLICATION_CERTIFICATE.pid = 7;

            WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE.pid = 8;

            WPD_CLIENT_DESIRED_ACCESS.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_DESIRED_ACCESS.pid = 9;

            WPD_CLIENT_SHARE_MODE.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_SHARE_MODE.pid = 10;

            WPD_CLIENT_EVENT_COOKIE.fmtid = new Guid(0x204D9F0C, 0x2292, 0x4080, 0x9F, 0x42, 0x40, 0x66, 0x4E, 0x70, 0xF8, 0x59);
            WPD_CLIENT_EVENT_COOKIE.pid = 11;
        }

        // WPD_OBJECT_PROPERTIES_V1
        public static PROPERTYKEY WPD_OBJECT_ID;

        // WPD_CLIENT_INFORMATION_PROPERTIES_V1
        public static PROPERTYKEY WPD_CLIENT_NAME;
        public static PROPERTYKEY WPD_CLIENT_MAJOR_VERSION;
        public static PROPERTYKEY WPD_CLIENT_MINOR_VERSION;
        public static PROPERTYKEY WPD_CLIENT_REVISION;
        public static PROPERTYKEY WPD_CLIENT_WMDRM_APPLICATION_PRIVATE_KEY;
        public static PROPERTYKEY WPD_CLIENT_WMDRM_APPLICATION_CERTIFICATE;
        public static PROPERTYKEY WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE;
        public static PROPERTYKEY WPD_CLIENT_DESIRED_ACCESS;
        public static PROPERTYKEY WPD_CLIENT_SHARE_MODE;
        public static PROPERTYKEY WPD_CLIENT_EVENT_COOKIE;
    }

    public class WPD_CATEGORY_COMMON
    {
        static WPD_CATEGORY_COMMON()
        {
            WPD_COMMAND_COMMON_RESET_DEVICE.fmtid = _guid;
            WPD_COMMAND_COMMON_RESET_DEVICE.pid = 2;

            WPD_COMMAND_COMMON_GET_OBJECT_IDS_FROM_PERSISTENT_UNIQUE_IDS.fmtid = _guid;
            WPD_COMMAND_COMMON_GET_OBJECT_IDS_FROM_PERSISTENT_UNIQUE_IDS.pid = 3;

            WPD_COMMAND_COMMON_SAVE_CLIENT_INFORMATION.fmtid = _guid;
            WPD_COMMAND_COMMON_SAVE_CLIENT_INFORMATION.pid = 4;

            WPD_PROPERTY_COMMON_COMMAND_CATEGORY.fmtid = _guid;
            WPD_PROPERTY_COMMON_COMMAND_CATEGORY.pid = 1001;

            WPD_PROPERTY_COMMON_COMMAND_ID.fmtid = _guid;
            WPD_PROPERTY_COMMON_COMMAND_ID.pid = 1002;

            WPD_PROPERTY_COMMON_HRESULT.fmtid = _guid;
            WPD_PROPERTY_COMMON_HRESULT.pid = 1003;

            WPD_PROPERTY_COMMON_DRIVER_ERROR_CODE.fmtid = _guid;
            WPD_PROPERTY_COMMON_DRIVER_ERROR_CODE.pid = 1004;

            WPD_PROPERTY_COMMON_COMMAND_TARGET.fmtid = _guid;
            WPD_PROPERTY_COMMON_COMMAND_TARGET.pid = 1006;

            WPD_PROPERTY_COMMON_COMMAND_TARGET.fmtid = _guid;
            WPD_PROPERTY_COMMON_COMMAND_TARGET.pid = 1007;

            WPD_PROPERTY_COMMON_OBJECT_IDS.fmtid = _guid;
            WPD_PROPERTY_COMMON_OBJECT_IDS.pid = 1008;

            WPD_PROPERTY_COMMON_CLIENT_INFORMATION.fmtid = _guid;
            WPD_PROPERTY_COMMON_CLIENT_INFORMATION.pid = 1009;

            WPD_PROPERTY_COMMON_CLIENT_INFORMATION_CONTEXT.fmtid = _guid;
            WPD_PROPERTY_COMMON_CLIENT_INFORMATION_CONTEXT.pid = 1010;

            WPD_OPTION_VALID_OBJECT_IDS.fmtid = _guid;
            WPD_OPTION_VALID_OBJECT_IDS.pid = 5001;
        }


        private static Guid _guid = new Guid(0xF0422A9C, 0x5DC8, 0x4440, 0xB5, 0xBD, 0x5D, 0xF2, 0x88, 0x35, 0x65, 0x8A);

        public static PROPERTYKEY WPD_COMMAND_COMMON_RESET_DEVICE;
        public static PROPERTYKEY WPD_COMMAND_COMMON_GET_OBJECT_IDS_FROM_PERSISTENT_UNIQUE_IDS;
        public static PROPERTYKEY WPD_COMMAND_COMMON_SAVE_CLIENT_INFORMATION;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_COMMAND_CATEGORY;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_COMMAND_ID;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_HRESULT;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_DRIVER_ERROR_CODE;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_COMMAND_TARGET;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_PERSISTENT_UNIQUE_IDS;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_OBJECT_IDS;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_CLIENT_INFORMATION;
        public static PROPERTYKEY WPD_PROPERTY_COMMON_CLIENT_INFORMATION_CONTEXT;
        public static PROPERTYKEY WPD_OPTION_VALID_OBJECT_IDS;
    }

    public class WPD_CATEGORY_ENHANCED_STORAGE
    {
        static WPD_CATEGORY_ENHANCED_STORAGE()
        {
            ENHANCED_STORAGE_COMMAND_SILO_IS_AUTHENTICATION_SILO.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_SILO_IS_AUTHENTICATION_SILO.pid = 6;

            ENHANCED_STORAGE_COMMAND_SILO_GET_AUTHENTICATION_STATE.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_SILO_GET_AUTHENTICATION_STATE.pid = 7;

            ENHANCED_STORAGE_COMMAND_SILO_START_AUTHENTICATION.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_SILO_START_AUTHENTICATION.pid = 9;

            ENHANCED_STORAGE_COMMAND_SILO_START_UNAUTHENTICATION.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_SILO_START_UNAUTHENTICATION.pid = 10;

            ENHANCED_STORAGE_COMMAND_SILO_ENUMERATE_SILOS.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_SILO_ENUMERATE_SILOS.pid = 11;

            //
            ENHANCED_STORAGE_COMMAND_CERT_HOST_CERTIFICATE_AUTHENTICATION.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_HOST_CERTIFICATE_AUTHENTICATION.pid = 101;

            ENHANCED_STORAGE_COMMAND_CERT_DEVICE_CERTIFICATE_AUTHENTICATION.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_DEVICE_CERTIFICATE_AUTHENTICATION.pid = 102;

            ENHANCED_STORAGE_COMMAND_CERT_ADMIN_CERTIFICATE_AUTHENTICATION.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_ADMIN_CERTIFICATE_AUTHENTICATION.pid = 103;

            ENHANCED_STORAGE_COMMAND_CERT_INITIALIZE_TO_MANUFACTURED_STATE.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_INITIALIZE_TO_MANUFACTURED_STATE.pid = 104;

            ENHANCED_STORAGE_COMMAND_CERT_GET_CERTIFICATE_COUNT.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_GET_CERTIFICATE_COUNT.pid = 105;

            ENHANCED_STORAGE_COMMAND_CERT_GET_CERTIFICATE.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_GET_CERTIFICATE.pid = 106;

            ENHANCED_STORAGE_COMMAND_CERT_SET_CERTIFICATE.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_SET_CERTIFICATE.pid = 107;

            ENHANCED_STORAGE_COMMAND_CERT_CREATE_CERTIFICATE_REQUEST.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_CREATE_CERTIFICATE_REQUEST.pid = 108;

            ENHANCED_STORAGE_COMMAND_CERT_UNAUTHENTICATION.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_UNAUTHENTICATION.pid = 110;

            ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_CAPABILITY.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_CAPABILITY.pid = 111;

            ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_CAPABILITIES.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_CAPABILITIES.pid = 112;

            ENHANCED_STORAGE_COMMAND_CERT_GET_ACT_FRIENDLY_NAME.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_GET_ACT_FRIENDLY_NAME.pid = 113;

            ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_GUID.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_GUID.pid = 114;

            //
            ENHANCED_STORAGE_COMMAND_PASSWORD_AUTHORIZE_ACT_ACCESS.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_AUTHORIZE_ACT_ACCESS.pid = 203;

            ENHANCED_STORAGE_COMMAND_PASSWORD_UNAUTHORIZE_ACT_ACCESS.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_UNAUTHORIZE_ACT_ACCESS.pid = 204;

            ENHANCED_STORAGE_COMMAND_PASSWORD_QUERY_INFORMATION.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_QUERY_INFORMATION.pid = 205;

            ENHANCED_STORAGE_COMMAND_PASSWORD_CONFIG_ADMINISTRATOR.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_CONFIG_ADMINISTRATOR.pid = 206;

            ENHANCED_STORAGE_COMMAND_PASSWORD_CREATE_USER.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_CREATE_USER.pid = 207;

            ENHANCED_STORAGE_COMMAND_PASSWORD_DELETE_USER.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_DELETE_USER.pid = 208;

            ENHANCED_STORAGE_COMMAND_PASSWORD_CHANGE_PASSWORD.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_CHANGE_PASSWORD.pid = 209;

            ENHANCED_STORAGE_COMMAND_PASSWORD_INITIALIZE_USER_PASSWORD.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_INITIALIZE_USER_PASSWORD.pid = 210;

            ENHANCED_STORAGE_COMMAND_PASSWORD_START_INITIALIZE_TO_MANUFACTURER_STATE.fmtid = _guid;
            ENHANCED_STORAGE_COMMAND_PASSWORD_START_INITIALIZE_TO_MANUFACTURER_STATE.pid = 211;

            //
            ENHANCED_STORAGE_PROPERTY_AUTHORIZATION_STATE.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_AUTHORIZATION_STATE.pid = 1005;

            ENHANCED_STORAGE_PROPERTY_AUTHENTICATION_STATE.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_AUTHENTICATION_STATE.pid = 1006;

            ENHANCED_STORAGE_PROPERTY_ACT_DRIVER_STATE.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_ACT_DRIVER_STATE.pid = 1007;

            ENHANCED_STORAGE_PROPERTY_IS_AUTHENTICATION_SILO.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_IS_AUTHENTICATION_SILO.pid = 1009;

            ENHANCED_STORAGE_PROPERTY_AUTHENTICATION_NEEDS_UI.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_AUTHENTICATION_NEEDS_UI.pid = 1010;

            ENHANCED_STORAGE_PROPERTY_QUERY_SILO_TYPE.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_QUERY_SILO_TYPE.pid = 2016;         // fixme!!!

            ENHANCED_STORAGE_PROPERTY_QUERY_SILO_RESULTS.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_QUERY_SILO_RESULTS.pid = 2017;      // fixme!!!

            //
            ENHANCED_STORAGE_PROPERTY_MAX_AUTH_FAILURES.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_MAX_AUTH_FAILURES.pid = 2001;

            ENHANCED_STORAGE_PROPERTY_PASSWORD.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_PASSWORD.pid = 2004;

            ENHANCED_STORAGE_PROPERTY_OLD_PASSWORD.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_OLD_PASSWORD.pid = 2005;

            ENHANCED_STORAGE_PROPERTY_PASSWORD_INDICATOR.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_PASSWORD_INDICATOR.pid = 2006;

            ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD_INDICATOR.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD_INDICATOR.pid = 2007;

            ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD.pid = 2008;

            ENHANCED_STORAGE_PROPERTY_USER_HINT.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_USER_HINT.pid = 2009;

            ENHANCED_STORAGE_PROPERTY_USER_NAME.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_USER_NAME.pid = 2010;

            ENHANCED_STORAGE_PROPERTY_ADMIN_HINT.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_ADMIN_HINT.pid = 2011;

            ENHANCED_STORAGE_PROPERTY_SILO_NAME.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_SILO_NAME.pid = 2012;

            ENHANCED_STORAGE_PROPERTY_SILO_FRIENDLYNAME_SPECIFIED.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_SILO_FRIENDLYNAME_SPECIFIED.pid = 2013;

            ENHANCED_STORAGE_PROPERTY_PASSWORD_SILO_INFO.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_PASSWORD_SILO_INFO.pid = 2014;

            ENHANCED_STORAGE_PROPERTY_SECURITY_IDENTIFIER.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_SECURITY_IDENTIFIER.pid = 2015;
            //
            ENHANCED_STORAGE_PROPERTY_MAX_CERTIFICATE_COUNT.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_MAX_CERTIFICATE_COUNT.pid = 3001;

            ENHANCED_STORAGE_PROPERTY_STORED_CERTIFICATE_COUNT.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_STORED_CERTIFICATE_COUNT.pid = 3002;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_INDEX.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_INDEX.pid = 3003;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_TYPE.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_TYPE.pid = 3004;

            ENHANCED_STORAGE_PROPERTY_VALIDATION_POLICY.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_VALIDATION_POLICY.pid = 3005;

            ENHANCED_STORAGE_PROPERTY_NEXT_CERTIFICATE_INDEX.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_NEXT_CERTIFICATE_INDEX.pid = 3006;

            ENHANCED_STORAGE_PROPERTY_NEXT_CERTIFICATE_OF_TYPE_INDEX.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_NEXT_CERTIFICATE_OF_TYPE_INDEX.pid = 3007;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_LENGTH.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_LENGTH.pid = 3008;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE.pid = 3009;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_REQUEST.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_REQUEST.pid = 3010;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_CAPABILITY_TYPE.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_CAPABILITY_TYPE.pid = 3011;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_CAPABILITY.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_CAPABILITY.pid = 3012;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_CAPABILITIES.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_CAPABILITIES.pid = 3013;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_ACT_FRIENDLY_NAME.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_ACT_FRIENDLY_NAME.pid = 3014;

            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_GUID.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_GUID.pid = 3015;

            ENHANCED_STORAGE_PROPERTY_SIGNER_CERTIFICATE_INDEX.fmtid = _guid;
            ENHANCED_STORAGE_PROPERTY_SIGNER_CERTIFICATE_INDEX.pid = 3016;

            //
            ENHANCED_STORAGE_CAPABILITY_HASH_ALGS.fmtid = _guid;
            ENHANCED_STORAGE_CAPABILITY_HASH_ALGS.pid = 4001;

            ENHANCED_STORAGE_CAPABILITY_ASYMMETRIC_KEY_CRYPTOGRAPHY.fmtid = _guid;
            ENHANCED_STORAGE_CAPABILITY_ASYMMETRIC_KEY_CRYPTOGRAPHY.pid = 4002;

            ENHANCED_STORAGE_CAPABILITY_SIGNING_ALGS.fmtid = _guid;
            ENHANCED_STORAGE_CAPABILITY_SIGNING_ALGS.pid = 4003;

            ENHANCED_STORAGE_CAPABILITY_RENDER_USER_DATA_UNUSABLE.fmtid = _guid;
            ENHANCED_STORAGE_CAPABILITY_RENDER_USER_DATA_UNUSABLE.pid = 4004;

            ENHANCED_STORAGE_CAPABILITY_CERTIFICATE_EXTENSION_PARSING.fmtid = _guid;
            ENHANCED_STORAGE_CAPABILITY_CERTIFICATE_EXTENSION_PARSING.pid = 4005;
        }

        private static Guid _guid = new Guid(0x91248166, 0xb832, 0x4ad4, 0xba, 0xa4, 0x7c, 0xa0, 0xb6, 0xb2, 0x79, 0x8c);

        // Authentication specific commands
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_SILO_IS_AUTHENTICATION_SILO;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_SILO_GET_AUTHENTICATION_STATE;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_SILO_START_AUTHENTICATION;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_SILO_START_UNAUTHENTICATION;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_SILO_ENUMERATE_SILOS;

        // Certificate specific commands
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_HOST_CERTIFICATE_AUTHENTICATION;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_DEVICE_CERTIFICATE_AUTHENTICATION;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_ADMIN_CERTIFICATE_AUTHENTICATION;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_INITIALIZE_TO_MANUFACTURED_STATE;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_GET_CERTIFICATE_COUNT;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_GET_CERTIFICATE;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_SET_CERTIFICATE;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_CREATE_CERTIFICATE_REQUEST;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_UNAUTHENTICATION;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_CAPABILITY;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_CAPABILITIES;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_GET_ACT_FRIENDLY_NAME;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_GUID;

        // Password specific commands
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_AUTHORIZE_ACT_ACCESS;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_UNAUTHORIZE_ACT_ACCESS;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_QUERY_INFORMATION;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_CONFIG_ADMINISTRATOR;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_CREATE_USER;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_DELETE_USER;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_CHANGE_PASSWORD;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_INITIALIZE_USER_PASSWORD;
        public static PROPERTYKEY ENHANCED_STORAGE_COMMAND_PASSWORD_START_INITIALIZE_TO_MANUFACTURER_STATE;

        // This section defines all WPD Enhanced Storage Properties
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_AUTHORIZATION_STATE;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_AUTHENTICATION_STATE;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_ACT_DRIVER_STATE;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_IS_AUTHENTICATION_SILO;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_AUTHENTICATION_NEEDS_UI;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_QUERY_SILO_TYPE;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_QUERY_SILO_RESULTS;

        // Password silo specific properties
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_MAX_AUTH_FAILURES;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_PASSWORD;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_OLD_PASSWORD;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_PASSWORD_INDICATOR;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD_INDICATOR;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_USER_HINT;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_USER_NAME;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_ADMIN_HINT;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_SILO_NAME;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_SILO_FRIENDLYNAME_SPECIFIED;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_PASSWORD_SILO_INFO;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_SECURITY_IDENTIFIER;
        
        // Certificate silo specific properties.
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_MAX_CERTIFICATE_COUNT;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_STORED_CERTIFICATE_COUNT;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_INDEX;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_TYPE;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_VALIDATION_POLICY;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_NEXT_CERTIFICATE_INDEX;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_NEXT_CERTIFICATE_OF_TYPE_INDEX;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_LENGTH;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_REQUEST;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_CAPABILITY_TYPE;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_CAPABILITY;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_CAPABILITIES;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_ACT_FRIENDLY_NAME;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_GUID;
        public static PROPERTYKEY ENHANCED_STORAGE_PROPERTY_SIGNER_CERTIFICATE_INDEX;

        // Silo capability specific properties.
        public static PROPERTYKEY ENHANCED_STORAGE_CAPABILITY_HASH_ALGS;
        public static PROPERTYKEY ENHANCED_STORAGE_CAPABILITY_ASYMMETRIC_KEY_CRYPTOGRAPHY;
        public static PROPERTYKEY ENHANCED_STORAGE_CAPABILITY_SIGNING_ALGS;
        public static PROPERTYKEY ENHANCED_STORAGE_CAPABILITY_RENDER_USER_DATA_UNUSABLE;
        public static PROPERTYKEY ENHANCED_STORAGE_CAPABILITY_CERTIFICATE_EXTENSION_PARSING;
    }

    #endregion

    [StructLayout(LayoutKind.Explicit)]
    public struct PropVariant
    {
        [FieldOffset(0)] public VarEnum variantType;
        [FieldOffset(2)] private short reserved1;
        [FieldOffset(4)] private short reserved2;
        [FieldOffset(6)] private short reserved3;
        [FieldOffset(8)] public IntPtr pointerValue;
        [FieldOffset(8)]
        public byte byteValue;
        [FieldOffset(8)]
        public long longValue;
        [FieldOffset(8)]
        public double dateValue;

        public string StringValue
        {
            get
            {
                // Check if object has been already disposed (and unmanaged memory already freed)
                switch (variantType)
                {
                    case VarEnum.VT_LPWSTR:
                        return Marshal.PtrToStringUni(pointerValue);
                    case VarEnum.VT_LPSTR:
                        return Marshal.PtrToStringAnsi(pointerValue);
                    case VarEnum.VT_BSTR:
                        return Marshal.PtrToStringBSTR(pointerValue);
                    case VarEnum.VT_BOOL:
                        return (longValue != 0) ? "TRUE" : "FALSE";
                    default:
                        return String.Empty;
                }
             }
        }

        public Guid GuidValue
        {
            get 
            {
                if (variantType == VarEnum.VT_CLSID)
                {
                    return (Guid)Marshal.PtrToStructure(pointerValue, typeof(Guid));
                }
                else
                {
                    return Guid.Empty;
                }
            }
        }
    }


    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct ENHANCED_STORAGE_PASSWORD_SILO_INFORMATION
    {
        public byte CurrentAdminFailures;
        public byte CurrentUserFailures;
        public UInt32 TotalUserAuthenticationCount;
        public UInt32 TotalAdminAuthenticationCount;

        public UInt32 FipsCompliant;
        public UInt32 SecurityIDAvailable;
        public UInt32 InitializeInProgress;
        public UInt32 ITMSArmed;
        public UInt32 ITMSArmable;
        public UInt32 UserCreated;
        public UInt32 ResetOnPORDefault;
        public UInt32 ResetOnPORCurrent;

        public byte MaxAdminFailures;
        public byte MaxUserFailures;

        public UInt32 TimeToCompleteInitialization;
        public UInt32 TimeRemainingToCompleteInitialization;
        public UInt32 MinTimeToAuthenticate;

        public byte MaxAdminPasswordSize;
        public byte MinAdminPasswordSize;
        public byte MaxAdminHintSize;
        public byte MaxUserPasswordSize;
        public byte MinUserPasswordSize;
        public byte MaxUserHintSize;
        public byte MaxUserNameSize;
        public byte MaxSiloNameSize;
        public Int16 MaxChallengeSize;
    }

    public enum CERTIFICATE_TYPE
    {
        Remove = 0,
        PCp = 2,
        ASCh = 3,
        HCh = 4,
        SCh = 6
    }

    public enum CERTIFICATE_VALIDATION_POLICY
    {
        None = 1,
        Basic = 2,
        Extended = 3,
        Invalid = 4,
    }
}
