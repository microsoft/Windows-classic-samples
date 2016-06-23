using System;
using System.Runtime.InteropServices;

namespace DesktopToastsSample
{
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    public struct PROPERTYKEY
    {
        public Guid fmtid;
        public uint pid;

        public PROPERTYKEY(Guid guid, uint id)
        {
            fmtid = guid;
            pid = id;
        }

        /// <summary>PKEY_AppUserModel_ID</summary>
        public static readonly PROPERTYKEY AppUserModel_ID = new PROPERTYKEY(new Guid("9F4C2855-9F79-4B39-A8D0-E1D42DE1D5F3"), 5);

        /// <summary>PKEY_AppUserModel_ID</summary>
        public static readonly PROPERTYKEY AppUserModel_ToastActivatorCLSID = new PROPERTYKEY(new Guid("9F4C2855-9F79-4B39-A8D0-E1D42DE1D5F3"), 26);
    }
}
