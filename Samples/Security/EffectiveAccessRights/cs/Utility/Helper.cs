// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Text.RegularExpressions;

namespace Microsoft.Samples.DynamicAccessControl.Utility
{
    using DWORD = System.UInt32;
    using PSECURITY_DESCRIPTOR = IntPtr;

    static internal class Helper
    {
        #region Public methods
        public static void LogWarning(string text, bool terminateLine = false)
        {
            if (terminateLine)
            {
                text += Environment.NewLine;
            }
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.Write(text);
            Console.ResetColor();
        }

        public static void LogError(string text, bool terminateLine = false)
        {
            if (terminateLine)
            {
                text += Environment.NewLine;
            }
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write(text);
            Console.ResetColor();
        }

        public static void ReportDuplicateCmdLnParam(string parameter)
        {
            LogWarning("Ignoring duplicate command line parameter - ");
            Console.WriteLine(parameter);
        }

        public static void ReportIgnoredAccount(string objname, string inParam = null)
        {
            LogWarning("Ignoring account - ");
            Console.Write("'{0}'", objname);
            LogWarning(" that could not be resolved");
            if (!string.IsNullOrEmpty(inParam))
            {
                LogWarning(" in parameter: ");
                Console.WriteLine(inParam);
            }
            else
            {
                LogWarning(".", true);
            }
        }

        public static void ReportDuplicateClaim(string claimid, string param)
        {
            LogWarning("Ignoring duplicate claim - ");
            Console.Write("'{0}'", claimid);
            LogWarning(" in parameter - ");
            Console.WriteLine(param);
        }

        public static void ReportDuplicateValue(ValueType type, string value)
        {
            LogWarning(string.Format(CultureInfo.CurrentCulture, "Ignoring duplicate {0} value - ", type.ToString()));
            Console.WriteLine("{0}", value);
        }

        public static SecurityIdentifier GetSidForObject(string objname, bool device = false)
        {
            NTAccount objAccount = null;

            if (Regex.Match(objname, @"(S(-\d+){2,8})").Success)
            {
                return new SecurityIdentifier(objname);
            }

            Match result = Regex.Match(objname, @"(?<domain>[\w]+)[\\](?:<object>[\w]+)" + (device ? @"\$" : ""));
            if (result.Success)
            {
                objAccount = new NTAccount(result.Groups["domain"].Value, result.Groups["object"].Value);
            }
            else
            {
                objAccount = new NTAccount(objname);
            }

            return (SecurityIdentifier)objAccount.Translate(typeof(SecurityIdentifier));
        }

        public static byte[] ConvertSecurityDescriptorToByteArray(PSECURITY_DESCRIPTOR securityDescriptor)
        {
            DWORD sdLength = NativeMethods.GetSecurityDescriptorLength(securityDescriptor);

            byte[] buffer = new byte[sdLength];
            Marshal.Copy(securityDescriptor, buffer, 0, (int)sdLength);

            return buffer;
        }
        #endregion

        #region Nested class for P/Invokes
        static class NativeMethods
        {
            [DllImport(Win32.ADVAPI32_DLL, CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Unicode)]
            public static extern DWORD GetSecurityDescriptorLength(PSECURITY_DESCRIPTOR pSecurityDescriptor);
        }
        #endregion
    }
}
