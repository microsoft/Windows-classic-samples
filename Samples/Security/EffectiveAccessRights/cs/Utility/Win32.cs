// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Runtime.InteropServices;

namespace Microsoft.Samples.DynamicAccessControl.Utility
{
    using BOOL = System.Int32;

    static internal class Win32
    {
        public const BOOL FALSE = 0;
        public const BOOL TRUE = 1;

        public const int MAX_PATH = 260;
        public const int MAX_LONG_PATH = 33000;

        public static bool NT_SUCCESS(int status)
        {
            return status >= 0;
        }

        public const string ADVAPI32_DLL = "advapi32.dll";
        public const string AUTHZ_DLL = "authz.dll";
        public const string KERNEL32_DLL = "kernel32.dll";
        public const string MPR_DLL = "Mpr.dll";
        public const string NETAPI32_DLL = "Netapi32.dll";
        public const string SHLWAPI_DLL = "shlwapi.dll";

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct LUID
        {
            public uint LowPart;
            public uint HighPart;

            public static LUID NullLuid
            {
                get
                {
                    LUID Empty;
                    Empty.LowPart = 0;
                    Empty.HighPart = 0;

                    return Empty;
                }
            }
        }
    }

    static internal class Win32Error
    {
        // Note - the error codes here should all match the definitions in winerror.h.

        /// <summary>
        /// Equal to ERROR_SUCCESS (The operation completed successfully).
        /// </summary>
        public const int NO_ERROR = 0;

        /// <summary>
        /// Error code indicating: The operation completed successfully.
        /// </summary>
        public const int ERROR_SUCCESS = 0;

        /// <summary>
        /// The system cannot find the file specified.
        /// </summary>
        public const int ERROR_FILE_NOT_FOUND = 2;

        /// <summary>
        /// Error code indicating: Access is denied.
        /// </summary>
        public const int ERROR_ACCESS_DENIED = 5;

        /// <summary>
        /// Error code indicating: Not enough storage is available to process this command
        /// </summary>
        public const int ERROR_NOT_ENOUGH_MEMORY = 8;
        /// <summary>
        /// The data area passed to a system call is too small.
        /// </summary>
        public const int ERROR_INSUFFICIENT_BUFFER = 122;

        /// <summary>
        /// The filename or extension is too long.
        /// </summary>
        public const int ERROR_FILENAME_EXCED_RANGE = 206;

        /// <summary>
        /// More data is available.
        /// </summary>
        public const int ERROR_MORE_DATA = 234;

        /// <summary>
        /// An attempt was made to reference a token that does not exist.
        /// </summary>
        public const int ERROR_NO_TOKEN = 1008;

        /// <summary>
        /// The specified device name is invalid.
        /// </summary>
        public const int ERROR_BAD_DEVICE = 1200;

        /// <summary>
        /// Not all privileges or groups referenced are assigned to the caller.
        /// </summary>
        public const int ERROR_NOT_ALL_ASSIGNED = 1300;

        /// <summary>
        /// A specified privilege does not exist.
        /// </summary>
        public const int ERROR_NO_SUCH_PRIVILEGE = 1313;

        /// <summary>
        /// Cannot open an anonymous level security token.
        /// </summary>
        public const int ERROR_CANT_OPEN_ANONYMOUS = 1347;

        /// <summary>
        /// The RPC server is unavailable.
        /// </summary>
        public const int RPC_S_SERVER_UNAVAILABLE = 1722;

        /// <summary>
        /// There are no more endpoints available from the endpoint mapper.
        /// </summary>
        public const int EPT_S_NOT_REGISTERED = 1753;

        /// <summary>
        /// This network connection does not exist.
        /// </summary>
        public const int ERROR_NOT_CONNECTED = 2250;
    }
}
