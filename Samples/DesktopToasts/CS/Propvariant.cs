using System;
using System.Runtime.InteropServices;

namespace DesktopToastsSample
{
    internal class PropVariantHelper
    {
        private static class NativeMethods
        {
            [DllImport("Ole32.dll", PreserveSig = false)]
            internal static extern void PropVariantClear(ref ShellHelpers.PROPVARIANT pvar);
        }

        private ShellHelpers.PROPVARIANT variant;
        public ShellHelpers.PROPVARIANT Propvariant
        {
            get { return variant; }
        }

        public VarEnum VarType
        {
            get { return (VarEnum)variant.vt; }
            set { variant.vt= (ushort)value; }
        }

        public void SetValue(Guid value)
        {
            NativeMethods.PropVariantClear(ref variant);
            byte[] guid = ((Guid)value).ToByteArray();
            variant.vt = (ushort)VarEnum.VT_CLSID;
            variant.unionmember = Marshal.AllocCoTaskMem(guid.Length);
            Marshal.Copy(guid, 0, variant.unionmember, guid.Length);
        }

        public void SetValue(string val)
        {
            NativeMethods.PropVariantClear(ref variant);
            variant.vt = (ushort)VarEnum.VT_LPWSTR;
            variant.unionmember = Marshal.StringToCoTaskMemUni(val);
        }
    }
}
