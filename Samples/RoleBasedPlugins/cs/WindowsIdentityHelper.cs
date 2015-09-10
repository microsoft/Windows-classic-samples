//-----------------------------------------------------------------------
// <copyright file="WindowsIdentityHelper.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.RoleBasedPlugins
{
    using System;
    using System.Runtime.InteropServices;
    using System.Security.Principal;

    /// <summary>
    /// Provide Helper functions for getting WindowsIdentity
    /// </summary>
    public static class WindowsIdentityHelper
    {
        /// <summary> Win32 constant for logon type in LogonUser API. Same as LOGON32_LOGON_INTERACTIVE </summary>
        private const int Logon32LogonInteractive = 2;

        /// <summary> Win32 constant for logon type in LogonUser API. Same as LOGON32_LOGON_NETWORK_CLEARTEXT </summary>
        private const int Logon32LogonNetworkClearText = 8;

        /// <summary> Win32 constant for logon provider in LogonUser API. Same as LOGON32_PROVIDER_DEFAULT </summary>
        private const int Logon32ProviderDefault = 0;

        /// <summary> Win32 constant of type SECURITY_IMPERSONATION_LEVEL </summary>
        private const int SecurityImpersonation = 2;

        /// <summary>
        /// Logs in a user using its credentials and returns the WindowsIdentity
        /// </summary>
        /// <param name="userName">User Name which needs to be logged in</param>
        /// <param name="password">Password of the user</param>
        /// <param name="domainName">Domain name for the user</param>
        /// <returns>WindosIdentity created after logging in the user</returns>
        public static WindowsIdentity GetWindowsIdentity(string userName, string password, string domainName)
        {
            IntPtr tokenHandle = new IntPtr(0);
            IntPtr dupeTokenHandle = new IntPtr(0);

            try
            {
                tokenHandle = IntPtr.Zero;
                dupeTokenHandle = IntPtr.Zero;

                if (NativeMethods.LogonUser(userName, domainName, password, Logon32LogonNetworkClearText, Logon32ProviderDefault, ref tokenHandle) == false)
                {
                    throw new ArgumentException("Error while trying to log user on");
                }

                if (NativeMethods.DuplicateToken(tokenHandle, SecurityImpersonation, ref dupeTokenHandle) == false)
                {
                    throw new ArgumentException("Error while trying to duplicate token");
                }

                return new WindowsIdentity(dupeTokenHandle);
            }
            finally
            {
                if (tokenHandle != IntPtr.Zero)
                {
                    NativeMethods.CloseHandle(tokenHandle);
                }

                if (dupeTokenHandle != IntPtr.Zero)
                {
                    NativeMethods.CloseHandle(dupeTokenHandle);
                }
            }
        }

        /// <summary>
        /// Gets current WindowsIdentity
        /// </summary>
        /// <returns>Current WindowsIdentity</returns>
        public static WindowsIdentity GetCurrentWindowsIdentity()
        {
            return WindowsIdentity.GetCurrent();
        }
    }
}