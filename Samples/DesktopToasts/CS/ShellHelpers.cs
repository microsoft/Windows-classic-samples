using System;
using System.Runtime.InteropServices;
using System.Text;

namespace DesktopToastsSample.ShellHelpers
{
    // These are the new APIs for Windows 10
    #region NewAPIs
    [StructLayout(LayoutKind.Sequential), Serializable]
    public struct NOTIFICATION_USER_INPUT_DATA
    {
        [MarshalAs(UnmanagedType.LPWStr)]
        public string Key;

        [MarshalAs(UnmanagedType.LPWStr)]
        public string Value;
    }

    [ComImport,
    Guid("53E31837-6600-4A81-9395-75CFFE746F94"), ComVisible(true),
    InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface INotificationActivationCallback
    {
        void Activate(
            [In, MarshalAs(UnmanagedType.LPWStr)]
            string appUserModelId,
            [In, MarshalAs(UnmanagedType.LPWStr)]
            string invokedArgs,
            [In, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]
            NOTIFICATION_USER_INPUT_DATA[] data,
            [In, MarshalAs(UnmanagedType.U4)]
            uint dataCount);
    }
    #endregion

    [ComImport,
    Guid("000214F9-0000-0000-C000-000000000046"),
    InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface IShellLinkW
    {
        void GetPath(
            [Out(), MarshalAs(UnmanagedType.LPWStr)] StringBuilder pszFile,
            int cchMaxPath,
            //ref _WIN32_FIND_DATAW pfd,
            IntPtr pfd,
            uint fFlags);
        void GetIDList(out IntPtr ppidl);
        void SetIDList(IntPtr pidl);
        void GetDescription(
            [Out(), MarshalAs(UnmanagedType.LPWStr)] StringBuilder pszFile,
            int cchMaxName);
        void SetDescription(
            [MarshalAs(UnmanagedType.LPWStr)] string pszName);
        void GetWorkingDirectory(
            [Out(), MarshalAs(UnmanagedType.LPWStr)] StringBuilder pszDir,
            int cchMaxPath
            );
        void SetWorkingDirectory(
            [MarshalAs(UnmanagedType.LPWStr)] string pszDir);
        void GetArguments(
            [Out(), MarshalAs(UnmanagedType.LPWStr)] StringBuilder pszArgs,
            int cchMaxPath);
        void SetArguments(
            [MarshalAs(UnmanagedType.LPWStr)] string pszArgs);
        void GetHotKey(out short wHotKey);
        void SetHotKey(short wHotKey);
        void GetShowCmd(out uint iShowCmd);
        void SetShowCmd(uint iShowCmd);
        void GetIconLocation(
            [Out(), MarshalAs(UnmanagedType.LPWStr)] out StringBuilder pszIconPath,
            int cchIconPath,
            out int iIcon);
        void SetIconLocation(
            [MarshalAs(UnmanagedType.LPWStr)] string pszIconPath,
            int iIcon);
        void SetRelativePath(
            [MarshalAs(UnmanagedType.LPWStr)] string pszPathRel,
            uint dwReserved);
        void Resolve(IntPtr hwnd, uint fFlags);
        void SetPath(
            [MarshalAs(UnmanagedType.LPWStr)] string pszFile);
    }

    [ComImport,
    Guid("0000010b-0000-0000-C000-000000000046"),
    InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface IPersistFile
    {
        void GetCurFile(
            [Out(), MarshalAs(UnmanagedType.LPWStr)] StringBuilder pszFile
        );
        void IsDirty();
        void Load(
            [MarshalAs(UnmanagedType.LPWStr)] string pszFileName,
            [MarshalAs(UnmanagedType.U4)] long dwMode);
        void Save(
            [MarshalAs(UnmanagedType.LPWStr)] string pszFileName,
            bool fRemember);
        void SaveCompleted(
            [MarshalAs(UnmanagedType.LPWStr)] string pszFileName);
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct PROPVARIANT
    {
        [FieldOffset(0)]
        public ushort vt;
        [FieldOffset(8)]
        public IntPtr unionmember;
        [FieldOffset(8)]
        public UInt64 forceStructToLargeEnoughSize;
    }

    [ComImport,
    Guid("886D8EEB-8CF2-4446-8D02-CDBA1DBDCF99"),
    InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IPropertyStore
    {
        void GetCount([Out] out uint propertyCount);
        void GetAt([In] uint propertyIndex, [Out, MarshalAs(UnmanagedType.Struct)] out PROPERTYKEY key);
        void GetValue([In, MarshalAs(UnmanagedType.Struct)] ref PROPERTYKEY key, [Out, MarshalAs(UnmanagedType.Struct)] out PROPVARIANT pv);
        void SetValue([In, MarshalAs(UnmanagedType.Struct)] ref PROPERTYKEY key, [In, MarshalAs(UnmanagedType.Struct)] ref PROPVARIANT pv);
        void Commit();
    }

    [ComImport,
    Guid("00021401-0000-0000-C000-000000000046"),
    ClassInterface(ClassInterfaceType.None)]
    internal class CShellLink { }
}