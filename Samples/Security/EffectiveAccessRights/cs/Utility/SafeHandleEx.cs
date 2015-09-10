// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Security;

using Microsoft.Win32.SafeHandles;

namespace Microsoft.Samples.DynamicAccessControl.Utility
{
    using HANDLE = System.IntPtr;

    internal sealed class SafeHGlobalHandle : IDisposable
    {
        #region Constructor and Destructor
        SafeHGlobalHandle()
        {
            pointer = IntPtr.Zero;
        }

        SafeHGlobalHandle(IntPtr handle)
        {
            pointer = handle;
        }

        ~SafeHGlobalHandle()
        {
            Dispose();
        }
        #endregion

        #region Public methods
        public static SafeHGlobalHandle InvalidHandle
        {
            get { return new SafeHGlobalHandle(IntPtr.Zero); }
        }

        /// <summary>
        /// Adds reference to other SafeHGlobalHandle objects, the pointer to
        /// which are refered to by this object. This is to ensure that such
        /// objects being referred to wouldn't be unreferenced until this object
        /// is active.
        /// 
        /// For e.g. when this object is an array of pointers to other objects
        /// </summary>
        /// <param name="children">Collection of SafeHGlobalHandle objects
        /// referred to by this object.</param>
        public void AddSubReference(IEnumerable<SafeHGlobalHandle> children)
        {
            if (references == null)
            {
                references = new List<SafeHGlobalHandle>();
            }

            references.AddRange(children);
        }

        /// <summary>
        /// Allocates from unmanaged memory to represent an array of pointers
        /// and marshals the unmanaged pointers (IntPtr) to the native array
        /// equivalent.
        /// </summary>
        /// <param name="values">Array of unmanaged pointers</param>
        /// <returns>SafeHGlobalHandle object to an native (unmanaged) array of pointers</returns>
        public static SafeHGlobalHandle AllocHGlobal(IntPtr[] values)
        {
            SafeHGlobalHandle result = AllocHGlobal(IntPtr.Size * values.Length);

            Marshal.Copy(values, 0, result.pointer, values.Length);

            return result;
        }

        public static SafeHGlobalHandle AllocHGlobalStruct<T>(T obj) where T : struct
        {
            Debug.Assert(typeof(T).StructLayoutAttribute.Value == LayoutKind.Sequential);

            SafeHGlobalHandle result = AllocHGlobal(Marshal.SizeOf(typeof(T)));

            Marshal.StructureToPtr(obj, result.pointer, false);

            return result;
        }
        
        /// <summary>
        /// Allocates from unmanaged memory to represent an array of structures
        /// and marshals the structure elements to the native array of
        /// structures. ONLY structures with attribute StructLayout of
        /// LayoutKind.Sequential are supported.
        /// </summary>
        /// <typeparam name="T">Native structure type</typeparam>
        /// <param name="values">Collection of structure objects</param>
        /// <param name="count">Number of elements in the collection</param>
        /// <returns>SafeHGlobalHandle object to an native (unmanaged) array of structures</returns>
        public static SafeHGlobalHandle AllocHGlobal<T>(ICollection<T> values) where T : struct
        {
            Debug.Assert(typeof(T).StructLayoutAttribute.Value == LayoutKind.Sequential);

            return AllocHGlobal(0, values, values.Count);
        }
        
        /// <summary>
        /// Allocates from unmanaged memory to represent a structure with a
        /// variable length array at the end and marshal these structure
        /// elements. It is the callers responsibility to marshal what preceeds
        /// the trailinh array into the unmanaged memory. ONLY structures with
        /// attribute StructLayout of LayoutKind.Sequential are supported.
        /// </summary>
        /// <typeparam name="T">Type of the trailing array of structures</typeparam>
        /// <param name="prefixBytes">Number of bytes preceeding the trailing array of structures</param>
        /// <param name="values">Collection of structure objects</param>
        /// <param name="count"></param>
        /// <returns>SafeHGlobalHandle object to an native (unmanaged) structure
        /// with a trail array of structures</returns>
        public static SafeHGlobalHandle AllocHGlobal<T>(int prefixBytes, IEnumerable<T> values, int count) where T
                                                                                                           : struct
        {
            Debug.Assert(typeof(T).StructLayoutAttribute.Value == LayoutKind.Sequential);

            SafeHGlobalHandle result = AllocHGlobal(prefixBytes + Marshal.SizeOf(typeof(T)) * count);

            IntPtr ptr = result.pointer + prefixBytes;
            foreach (var value in values)
            {
                Marshal.StructureToPtr(value, ptr, false);
                ptr += Marshal.SizeOf(typeof(T));
            }

            return result;
        }
        
        /// <summary>
        /// Allocates from unmanaged memory to represent a unicode string (WSTR)
        /// and marshal this to a native PWSTR.
        /// </summary>
        /// <param name="s">String</param>
        /// <returns>SafeHGlobalHandle object to an native (unmanaged) unicode string</returns>
        public static SafeHGlobalHandle AllocHGlobal(string s)
        {
            return new SafeHGlobalHandle(Marshal.StringToHGlobalUni(s));
        }

        /// <summary>
        /// Operator to obtain the unmanaged pointer wrapped by the object. Note
        /// that the returned pointer is only valid for the lifetime of this
        /// object.
        /// </summary>
        /// <param name="safeHandle">SafeHGlobalHandle object</param>
        /// <returns>Unmanaged pointer wrapped by the object</returns>
        public IntPtr ToIntPtr()
        {
            return pointer;
        }
        #endregion

        #region IDisposable implmentation
        public void Dispose()
        {
            if (pointer != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(pointer);
                pointer = IntPtr.Zero;
            }

            GC.SuppressFinalize(this);
        }
        #endregion

        #region Private implementation
        [SuppressMessage("Microsoft.Reliability", "CA2000:Dispose objects before losing scope",
                         Justification="Caller will dispose result")]
        static SafeHGlobalHandle AllocHGlobal(int cb)
        {
            if (cb < 0)
            {
                throw new ArgumentOutOfRangeException("cb", "The value of this argument must be non-negative");
            }

            SafeHGlobalHandle result = new SafeHGlobalHandle();

            //
            // CER
            //
            RuntimeHelpers.PrepareConstrainedRegions();
            try { }
            finally
            {
                result.pointer = Marshal.AllocHGlobal(cb);
            }

            return result;
        }
        #endregion

        #region Private members
        /// <summary>
        /// Maintainsreference to other SafeHGlobalHandle objects, the pointer
        /// to which are refered to by this object. This is to ensure that such
        /// objects being referred to wouldn't be unreferenced until this object
        /// is active.
        /// </summary>
        List<SafeHGlobalHandle> references;

        //
        // Using SafeHandle here doesn't buy much since the pointer is
        // eventually stashed into a native structure. Using a SafeHandle would
        // involve calling DangerousGetHandle in place of ToIntPtr which makes
        // code analysis report CA2001: Avoid calling problematic methods.
        //
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Reliability", "CA2006:UseSafeHandleToEncapsulateNativeResources")]
        /// <summary>
        /// Unmanaged pointer wrapped by this object
        /// </summary>
        IntPtr pointer;
        #endregion
    }

    //
    // Adopted from: http://msdn.microsoft.com/en-us/magazine/cc163823.aspx
    //
    /// <summary>
    /// Safe wrapper for HANDLE to a token.
    /// </summary>
    internal class SafeTokenHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        #region Constructors
        /// <summary>
        /// This safehandle instance "owns" the handle, hence base(true)
        /// is being called. When safehandle is no longer in use it will
        /// call this class's ReleaseHandle method which will release
        /// the resources
        /// </summary>
        private SafeTokenHandle() : base(true) { }

        // 0 is an Invalid Handle
        internal SafeTokenHandle(HANDLE handle)
            : base(true)
        {
            SetHandle(handle);
        }
        #endregion
        
        internal static SafeTokenHandle InvalidHandle
        {
            [SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode", Justification="Retain to illustrate semantics")]
            get { return new SafeTokenHandle(HANDLE.Zero); }
        }

        #region Private implementation
        /// <summary>
        /// Release the HANDLE held by this instance
        /// </summary>
        /// <returns>true if the release was successful. false otherwise.</returns>        
        override protected bool ReleaseHandle()
        {
            return NativeMethods.CloseHandle(handle);
        }
        #endregion

        #region Nested class for P/Invokes
        static class NativeMethods
        {
            [DllImport(Win32.KERNEL32_DLL, SetLastError = true),
             SuppressUnmanagedCodeSecurity,
             ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool CloseHandle(HANDLE handle);
        }
        #endregion
    }

    /// <summary>
    /// Safe wrapper for AUTHZ_RESOURCE_MANAGER_HANDLE.
    /// </summary>
    internal class SafeAuthzRMHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        #region Constructors
        /// <summary>
        /// This safehandle instance "owns" the handle, hence base(true)
        /// is being called. When safehandle is no longer in use it will
        /// call this class's ReleaseHandle method which will release
        /// the resources
        /// </summary>
        SafeAuthzRMHandle() : base(true) { }

        [SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode",
                         Justification = "Retain to illustrate semantics and for reuse")]
        SafeAuthzRMHandle(HANDLE handle)
            : base(true)
        {
            SetHandle(handle);
        }
        #endregion

        public static SafeAuthzRMHandle InvalidHandle
        {
            [SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode",
                             Justification = "Retain to illustrate semantics")]
            get { return new SafeAuthzRMHandle(HANDLE.Zero); }
        }

        #region Private implementation
        /// <summary>
        /// Release the resource manager handle held by this instance
        /// </summary>
        /// <returns>true if the release was successful. false otherwise.</returns>        
        override protected bool ReleaseHandle()
        {
            return NativeMethods.AuthzFreeResourceManager(handle);
        }
        #endregion

        #region Nested class for P/Invokes
        static class NativeMethods
        {
            [DllImport(Win32.AUTHZ_DLL, SetLastError = true),
             SuppressUnmanagedCodeSecurity,
             ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool AuthzFreeResourceManager(HANDLE handle);
        }
        #endregion
    }
}
