// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Specialized;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.ConstrainedExecution;
using System.Security;
using System.Threading;
using Microsoft.Win32.SafeHandles;

namespace Microsoft.Samples.DynamicAccessControl.Utility
{
    using HANDLE = System.IntPtr;

    using Win32Exception = System.ComponentModel.Win32Exception;
    using PrivilegeNotHeldException = System.Security.AccessControl.PrivilegeNotHeldException;

    using TokenAccessLevels = System.Security.Principal.TokenAccessLevels;
    using TokenImpersonationLevel = System.Security.Principal.TokenImpersonationLevel;

    public delegate void PrivilegedCallback(object state);

    //
    // Adopted from http://msdn.microsoft.com/en-us/magazine/cc163823.aspx
    //
    internal sealed class TokenPrivilege : IDisposable
    {
        #region Private static members
        private static LocalDataStoreSlot tlsSlot = Thread.AllocateDataSlot();
        private static HybridDictionary privileges = new HybridDictionary();
        private static HybridDictionary luids = new HybridDictionary();
        private static ReaderWriterLock privilegeLock = new ReaderWriterLock();
        #endregion

        #region Private members
        private bool needToRevert = false;
        private bool initialState = false;
        private bool stateWasChanged = false;
        private Win32.LUID luid;
        private readonly Thread currentThread = Thread.CurrentThread;
        private TlsContents tlsContents = null;
        private bool disposed = false;
        #endregion

        #region Privilege names
        public const string CreateToken = "SeCreateTokenPrivilege";
        public const string AssignPrimaryToken = "SeAssignPrimaryTokenPrivilege";
        public const string LockMemory = "SeLockMemoryPrivilege";
        public const string IncreaseQuota = "SeIncreaseQuotaPrivilege";
        public const string UnsolicitedInput = "SeUnsolicitedInputPrivilege";
        public const string MachineAccount = "SeMachineAccountPrivilege";
        public const string TrustedComputingBase = "SeTcbPrivilege";
        public const string Security = "SeSecurityPrivilege";
        public const string TakeOwnership = "SeTakeOwnershipPrivilege";
        public const string LoadDriver = "SeLoadDriverPrivilege";
        public const string SystemProfile = "SeSystemProfilePrivilege";
        public const string SystemTime = "SeSystemtimePrivilege";
        public const string ProfileSingleProcess = "SeProfileSingleProcessPrivilege";
        public const string IncreaseBasePriority = "SeIncreaseBasePriorityPrivilege";
        public const string CreatePageFile = "SeCreatePagefilePrivilege";
        public const string CreatePermanent = "SeCreatePermanentPrivilege";
        public const string Backup = "SeBackupPrivilege";
        public const string Restore = "SeRestorePrivilege";
        public const string Shutdown = "SeShutdownPrivilege";
        public const string Debug = "SeDebugPrivilege";
        public const string Audit = "SeAuditPrivilege";
        public const string SystemEnvironment = "SeSystemEnvironmentPrivilege";
        public const string ChangeNotify = "SeChangeNotifyPrivilege";
        public const string RemoteShutdown = "SeRemoteShutdownPrivilege";
        public const string Undock = "SeUndockPrivilege";
        public const string SyncAgent = "SeSyncAgentPrivilege";
        public const string EnableDelegation = "SeEnableDelegationPrivilege";
        public const string ManageVolume = "SeManageVolumePrivilege";
        public const string Impersonate = "SeImpersonatePrivilege";
        public const string CreateGlobal = "SeCreateGlobalPrivilege";
        public const string TrustedCredentialManagerAccess = "SeTrustedCredManAccessPrivilege";
        public const string ReserveProcessor = "SeReserveProcessorPrivilege";
        #endregion

        #region LUID caching logic

        //
        // This routine is a wrapper around a hashtable containing mappings
        // of privilege names to luids
        //

        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        private static Win32.LUID LuidFromPrivilege(string privilege)
        {
            Win32.LUID luid;
            luid.LowPart = 0;
            luid.HighPart = 0;

            //
            // Look up the privilege LUID inside the cache
            //

            RuntimeHelpers.PrepareConstrainedRegions();

            try
            {
                privilegeLock.AcquireReaderLock(Timeout.Infinite);

                if (luids.Contains(privilege))
                {
                    luid = (Win32.LUID)luids[privilege];

                    privilegeLock.ReleaseReaderLock();
                }
                else
                {
                    privilegeLock.ReleaseReaderLock();

                    if (!NativeMethods.LookupPrivilegeValue(null, privilege, ref luid))
                    {
                        int error = Marshal.GetLastWin32Error();

                        if (error == Win32Error.ERROR_NOT_ENOUGH_MEMORY)
                        {
                            throw new InsufficientMemoryException();
                        }
                        else if (error == Win32Error.ERROR_ACCESS_DENIED)
                        {
                            throw new UnauthorizedAccessException("Caller does not have the rights to look up " +
                                                                  "privilege local unique identifier");
                        }
                        else if (error == Win32Error.ERROR_NO_SUCH_PRIVILEGE)
                        {
                            throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                                                                      "{0} is not a valid privilege name",
                                                                      privilege),
                                                        "privilege");
                        }
                        else
                        {
                            throw new Win32Exception(error);
                        }
                    }

                    privilegeLock.AcquireWriterLock(Timeout.Infinite);
                }
            }
            finally
            {
                if (privilegeLock.IsReaderLockHeld)
                {
                    privilegeLock.ReleaseReaderLock();
                }

                if (privilegeLock.IsWriterLockHeld)
                {
                    if (!luids.Contains(privilege))
                    {
                        luids[privilege] = luid;
                        privileges[luid] = privilege;
                    }

                    privilegeLock.ReleaseWriterLock();
                }
            }

            return luid;
        }
        #endregion

        #region Nested classes
        private sealed class TlsContents : IDisposable
        {
            private int referenceCount = 1;
            private SafeTokenHandle threadHandle = new SafeTokenHandle(IntPtr.Zero);
            private bool isImpersonating = false;

            private static SafeTokenHandle processHandle = new SafeTokenHandle(IntPtr.Zero);
            private static readonly object syncRoot = new object();

            #region Constructor and finalizer
            public TlsContents()
            {
                int error = 0;
                int cachingError = 0;
                bool success = true;

                if (processHandle.IsInvalid)
                {
                    lock (syncRoot)
                    {
                        if (processHandle.IsInvalid)
                        {
                            if (!NativeMethods.OpenProcessToken(NativeMethods.GetCurrentProcess(),
                                                                TokenAccessLevels.Duplicate,
                                                                ref processHandle))
                            {
                                cachingError = Marshal.GetLastWin32Error();
                                success = false;
                            }
                        }
                    }
                }

                RuntimeHelpers.PrepareConstrainedRegions();

                try
                {
                    //
                    // Open the thread token; if there is no thread token,
                    // copy the process token onto the thread
                    //

                    if (!NativeMethods.OpenThreadToken(NativeMethods.GetCurrentThread(),
                                                         TokenAccessLevels.Query
                                                       | TokenAccessLevels.AdjustPrivileges,
                                                       true,
                                                       ref this.threadHandle))
                    {
                        if (success)
                        {
                            error = Marshal.GetLastWin32Error();

                            if (error != Win32Error.ERROR_NO_TOKEN)
                            {
                                success = false;
                            }

                            if (success)
                            {
                                error = 0;

                                if (!NativeMethods.DuplicateTokenEx(processHandle,
                                                                      TokenAccessLevels.Impersonate
                                                                    | TokenAccessLevels.Query
                                                                    | TokenAccessLevels.AdjustPrivileges,
                                                                    IntPtr.Zero,
                                                                    TokenImpersonationLevel.Impersonation,
                                                                    NativeMethods.TokenType.Impersonation,
                                                                    ref this.threadHandle))
                                {
                                    error = Marshal.GetLastWin32Error();
                                    success = false;
                                }
                            }

                            if (success)
                            {
                                if (!NativeMethods.SetThreadToken(HANDLE.Zero, this.threadHandle))
                                {
                                    error = Marshal.GetLastWin32Error();
                                    success = false;
                                }
                            }

                            if (success)
                            {
                                //
                                // This thread is now impersonating; it needs to be reverted to its original state
                                //

                                this.isImpersonating = true;
                            }
                        }
                        else
                        {
                            error = cachingError;
                        }
                    }
                    else
                    {
                        success = true;
                    }
                }
                finally
                {
                    if (!success)
                    {
                        Dispose();
                    }
                }

                switch (error)
                {
                    case Win32Error.ERROR_NOT_ENOUGH_MEMORY: throw new InsufficientMemoryException();
                    case Win32Error.ERROR_CANT_OPEN_ANONYMOUS: goto case Win32Error.ERROR_ACCESS_DENIED;
                    case Win32Error.ERROR_ACCESS_DENIED:
                        {
                            throw new UnauthorizedAccessException("The caller does not have the rights to perform the" +
                                                                  " operation");
                        }
                    default:
                        {
                            if (error != Win32Error.ERROR_SUCCESS)
                            {
                                throw new Win32Exception(error);
                            }
                            break;
                        }
                }
            }

            ~TlsContents()
            {
                Dispose(false);
            }
            #endregion

            #region IDisposable implementation
            public void Dispose()
            {
                Dispose(true);
                GC.SuppressFinalize(this);
            }

            private void Dispose(bool disposing)
            {
                if (disposing)
                {
                    if (this.threadHandle != null)
                    {
                        this.threadHandle.Dispose();
                        this.threadHandle = null;
                    }
                }

                if (this.isImpersonating)
                {
                    NativeMethods.RevertToSelf();
                    this.isImpersonating = false;
                }
            }
            #endregion

            #region Reference-counting
            public void IncrementReferenceCount()
            {
                this.referenceCount++;
            }

            public int DecrementReferenceCount()
            {
                int result = --this.referenceCount;

                if (result == 0)
                {
                    Dispose();
                }

                return result;
            }

            public int ReferenceCountValue
            {
                get { return this.referenceCount; }
            }
            #endregion

            #region Properties
            public SafeTokenHandle ThreadHandle
            {
                get { return this.threadHandle; }
            }

            public bool IsImpersonating
            {
                get { return this.isImpersonating; }
            }
            #endregion
        }
        #endregion

        #region Constructor and Destructor
        public TokenPrivilege(string privilegeName)
        {
            if (privilegeName == null)
            {
                throw new ArgumentNullException("privilegeName");
            }

            this.luid = LuidFromPrivilege(privilegeName);
        }
        
        ~TokenPrivilege()
        {
            Dispose(false);
        }
        
        #endregion

        #region Public methods and properties
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        public void Enable()
        {
            this.ToggleState(true);
        }

        [SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode",
                         Justification="Retain code adopted from MSDN Magazine sample as is")]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        public void Disable()
        {
            this.ToggleState(false);
        }

        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        public void Revert()
        {
            int error = 0;

            //
            // All privilege operations must take place on the same thread
            //

            if (!this.currentThread.Equals(Thread.CurrentThread))
            {
                throw new InvalidOperationException("Operation must take place on the thread that created the object");
            }

            if (!this.NeedToRevert)
            {
                return;
            }

            //
            // This code must be eagerly prepared and non-interruptible.
            //

            RuntimeHelpers.PrepareConstrainedRegions();

            try
            {
                //
                // The payload is entirely in the finally block
                // This is how we ensure that the code will not be
                // interrupted by catastrophic exceptions
                //
            }
            finally
            {
                bool success = true;

                try
                {
                    //
                    // Only call AdjustTokenPrivileges if we're not going to be reverting to self,
                    // on this Revert, since doing the latter obliterates the thread token anyway
                    //

                    if (this.stateWasChanged &&
                        (this.tlsContents.ReferenceCountValue > 1 ||
                        !this.tlsContents.IsImpersonating))
                    {
                        NativeMethods.TOKEN_PRIVILEGE newState = new NativeMethods.TOKEN_PRIVILEGE();
                        newState.PrivilegeCount = 1;
                        newState.Privilege.Luid = this.luid;
                        newState.Privilege.Attributes = (this.initialState
                                                        ? NativeMethods.PrivilegeAttribute.Enabled
                                                        : NativeMethods.PrivilegeAttribute.Disabled);

                        NativeMethods.TOKEN_PRIVILEGE previousState = new NativeMethods.TOKEN_PRIVILEGE();
                        uint previousSize = 0;

                        if (!NativeMethods.AdjustTokenPrivileges(this.tlsContents.ThreadHandle,
                                                                 false,
                                                                 ref newState,
                                                                 (uint)Marshal.SizeOf(previousState),
                                                                 ref previousState,
                                                                 ref previousSize))
                        {
                            error = Marshal.GetLastWin32Error();
                            success = false;
                        }
                    }
                }
                finally
                {
                    if (success)
                    {
                        this.Reset();
                    }
                }
            }

            if (error == Win32Error.ERROR_NOT_ENOUGH_MEMORY)
            {
                throw new InsufficientMemoryException();
            }
            else if (error == Win32Error.ERROR_ACCESS_DENIED)
            {
                throw new UnauthorizedAccessException("Caller does not have the permission to change the privilege");
            }
            else if (error != 0)
            {
                throw new Win32Exception(error);
            }
        }

        public bool NeedToRevert
        {
            get { return this.needToRevert; }
        }

        [SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode",
                         Justification = "Retain code adopted from MSDN Magazine sample as is")]
        public static void RunWithPrivilege(string privilege, bool enabled, PrivilegedCallback callback, object state)
        {
            if (callback == null)
            {
                throw new ArgumentNullException("callback");
            }

            using(TokenPrivilege p = new TokenPrivilege(privilege))
            {
                RuntimeHelpers.PrepareConstrainedRegions();
                try
                {
                    if (enabled)
                    {
                        p.Enable();
                    }
                    else
                    {
                        p.Disable();
                    }

                    callback(state);
                }
                finally
                {
                    p.Revert();
                }
            }
        }
        #endregion
        
        #region IDisposable implementation
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposed) return;

            if (disposing)
            {
                if (tlsContents != null)
                {
                    tlsContents.Dispose();
                    tlsContents = null;
                }
            }

            disposed = true;
        }
        #endregion

        #region Private implementation
        private void ToggleState(bool enable)
        {
            int error = 0;

            //
            // All privilege operations must take place on the same thread
            //

            if (!this.currentThread.Equals(Thread.CurrentThread))
            {
                throw new InvalidOperationException("Operation must take place on the thread that created the object");
            }

            //
            // This privilege was already altered and needs to be reverted before it can be altered again
            //

            if (this.NeedToRevert)
            {
                throw new InvalidOperationException("Must revert the privilege prior to attempting this operation");
            }

            //
            // Need to make this block of code non-interruptible so that it would preserve
            // consistency of thread oken state even in the face of catastrophic exceptions
            //

            RuntimeHelpers.PrepareConstrainedRegions();

            try
            {
                //
                // The payload is entirely in the finally block
                // This is how we ensure that the code will not be
                // interrupted by catastrophic exceptions
                //
            }
            finally
            {
                try
                {
                    //
                    // Retrieve TLS state
                    //

                    this.tlsContents = Thread.GetData(tlsSlot) as TlsContents;

                    if (this.tlsContents == null)
                    {
                        this.tlsContents = new TlsContents();
                        Thread.SetData(tlsSlot, this.tlsContents);
                    }
                    else
                    {
                        this.tlsContents.IncrementReferenceCount();
                    }

                    NativeMethods.TOKEN_PRIVILEGE newState = new NativeMethods.TOKEN_PRIVILEGE();
                    newState.PrivilegeCount = 1;
                    newState.Privilege.Luid = this.luid;
                    newState.Privilege.Attributes = (enable
                                                    ? NativeMethods.PrivilegeAttribute.Enabled
                                                    : NativeMethods.PrivilegeAttribute.Disabled);

                    NativeMethods.TOKEN_PRIVILEGE previousState = new NativeMethods.TOKEN_PRIVILEGE();
                    uint previousSize = 0;

                    //
                    // Place the new privilege on the thread token and remember the previous state.
                    //
                    bool fResult = NativeMethods.AdjustTokenPrivileges(this.tlsContents.ThreadHandle,
                                                                       false,
                                                                       ref newState,
                                                                       (uint)Marshal.SizeOf(previousState),
                                                                       ref previousState,
                                                                       ref previousSize);

                    error = Marshal.GetLastWin32Error();

                    if (fResult && error != Win32Error.ERROR_NOT_ALL_ASSIGNED)
                    {
                        //
                        // This is the initial state that revert will have to go back to
                        //

                        this.initialState = ((previousState.Privilege.Attributes
                                              & NativeMethods.PrivilegeAttribute.Enabled) != 0);

                        //
                        // Remember whether state has changed at all
                        //

                        this.stateWasChanged = (this.initialState != enable);

                        //
                        // If we had to impersonate, or if the privilege state changed we'll need to revert
                        //

                        this.needToRevert = this.tlsContents.IsImpersonating || this.stateWasChanged;
                    }
                }
                finally
                {
                    if (!this.needToRevert)
                    {
                        this.Reset();
                    }
                }
            }

            switch (error)
            {
                case Win32Error.ERROR_NOT_ALL_ASSIGNED: throw new PrivilegeNotHeldException(
                                                                    privileges[this.luid] as string);
                case Win32Error.ERROR_NOT_ENOUGH_MEMORY: throw new InsufficientMemoryException();
                case Win32Error.ERROR_CANT_OPEN_ANONYMOUS: goto case Win32Error.ERROR_ACCESS_DENIED;
                case Win32Error.ERROR_ACCESS_DENIED:
                    {
                        throw new UnauthorizedAccessException("The caller does not have the right to change the " +
                                                              "privilege");
                    }
                default:
                    {
                        if (error != Win32Error.ERROR_SUCCESS)
                        {
                            throw new Win32Exception(error);
                        }
                        break;
                    }
            }
        }

        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
        private void Reset()
        {
            RuntimeHelpers.PrepareConstrainedRegions();

            try
            {
                // Payload is in the finally block
                // as a way to guarantee execution
            }
            finally
            {
                this.stateWasChanged = false;
                this.initialState = false;
                this.needToRevert = false;

                if (this.tlsContents != null)
                {
                    if (0 == this.tlsContents.DecrementReferenceCount())
                    {
                        this.tlsContents = null;
                        Thread.SetData(tlsSlot, null);
                    }
                }
            }
        }
        #endregion

        #region Nested class for P/Invokes and native (Win32) structures
        static class NativeMethods
        {
            #region PInvoke advapi32
            [Flags]
            public enum PrivilegeAttribute : uint
            {
                Disabled         = 0x00000000,
                EnabledByDefault = 0x00000001,
                Enabled          = 0x00000002,
                Removed          = 0x00000004,
                UsedForAccess    = 0x80000000
            }

            [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
            public struct LUID_AND_ATTRIBUTES
            {
                public Win32.LUID Luid;
                public PrivilegeAttribute Attributes;
            }

            [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
            public struct TOKEN_PRIVILEGE
            {
                public uint PrivilegeCount;
                public LUID_AND_ATTRIBUTES Privilege;
            }

            [DllImport(Win32.ADVAPI32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool AdjustTokenPrivileges(
                SafeTokenHandle TokenHandle,
                [MarshalAs(UnmanagedType.Bool)]
                bool DisableAllPrivileges,
                ref TOKEN_PRIVILEGE NewState,
                uint BufferLength,
                ref TOKEN_PRIVILEGE PreviousState,
                ref uint ReturnLength);

            [DllImport(Win32.ADVAPI32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool LookupPrivilegeValue(
                string systemName,
                string privilegeName,
                ref Win32.LUID luid);

            [DllImport(Win32.ADVAPI32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool OpenProcessToken(
                HANDLE ProcessToken,
                TokenAccessLevels DesiredAccess,
                ref SafeTokenHandle TokenHandle);

            [DllImport(Win32.ADVAPI32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool OpenThreadToken(
                HANDLE ThreadToken,
                TokenAccessLevels DesiredAccess,
                [MarshalAs(UnmanagedType.Bool)]
                bool OpenAsSelf,
                ref SafeTokenHandle TokenHandle);

            internal enum TokenType
            {
                Primary = 1,
                Impersonation = 2,
            }

            [DllImport(Win32.ADVAPI32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern
            bool DuplicateTokenEx(
                SafeTokenHandle ExistingToken,
                TokenAccessLevels DesiredAccess,
                IntPtr TokenAttributes,
                TokenImpersonationLevel ImpersonationLevel,
                TokenType TokenType,
                ref SafeTokenHandle NewToken);

            [DllImport(Win32.ADVAPI32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool SetThreadToken(HANDLE Thread, SafeTokenHandle Token);

            [DllImport(Win32.ADVAPI32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool RevertToSelf();
            #endregion

            #region PInvoke kernel32
            [DllImport(Win32.KERNEL32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            public static extern
            HANDLE GetCurrentProcess();

            [DllImport(Win32.KERNEL32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
            public static extern HANDLE GetCurrentThread();
            #endregion
        }
        #endregion
    }
}
