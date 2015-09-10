// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Principal;

namespace Microsoft.Samples.DynamicAccessControl
{
    using ULONG = System.UInt32;

    using PSID = System.IntPtr;
    using AUTHZ_CLIENT_CONTEXT_HANDLE = System.IntPtr;

    using Utility;

    /// <summary>
    /// Enumeration used to identify if a GroupsCollection comprises of user 
    /// or device groups.
    /// </summary>
    internal enum GroupType
    {
        User,
        Device
    }

    /// <summary>
    /// Class to represent a set of group accounts and to facilitate applying
    /// these to an Authz client context
    /// </summary>
    internal class GroupsCollection : List<SecurityIdentifier>
    {
        #region Constructor
        /// <summary>
        /// Identifies if this instance represents user's group membership or
        /// device's group membership.
        /// </summary>
        /// <param name="type">GroupType.User to indicate user group membership
        /// and GroupType.Device to indicate device's group membership.</param>
        /// <remarks>When GroupType.User, AuthzModifySids is invoked
        /// with SidClass AuthzContextInfoGroupSids and when GroupType.Device
        /// with SidClass AuthzContextInfoDeviceSids.</remarks>
        public GroupsCollection(GroupType type)
        {
            this.type = type;
        }
        #endregion

        #region Public methods
        /// <summary>
        /// Adds or replaces groups in the specified Authz Client Context.
        /// </summary>
        /// <remarks>This method invokes AuthzModifySids, modifying the groups
        /// using AUTHZ_SID_OPERATION_REPLACE. This ensures that a group that
        /// already exists is retained and the ones not present are added</remarks>
        /// <param name="hAuthzClientContext">Handle to the Authz Client Context to be modified</param>
        /// <returns>Win32Error.ERROR_SUCCESS on success and Win32 error code otherwise.</returns>
        public int ApplyGroups(AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext)
        {
            var toBeFreed = new List<SafeHGlobalHandle>();
            var sidAndAttributes = new List<NativeMethods.SID_AND_ATTRIBUTES>(this.Count);
            var sidOps = new NativeMethods.AuthzSIDOperation[this.Count];

            foreach(var sid in this)
            {
                byte[] rawSid = new byte[sid.BinaryLength];
                sid.GetBinaryForm(rawSid, 0);

                SafeHGlobalHandle safesid = SafeHGlobalHandle.AllocHGlobal(rawSid);
                toBeFreed.Add(safesid);

                sidAndAttributes.Add(new NativeMethods.SID_AND_ATTRIBUTES(safesid.ToIntPtr(),
                                                                          NativeMethods.GroupAddtibute.Enabled));
            }

            SafeHGlobalHandle tokenGroups = SafeHGlobalHandle.AllocHGlobal(Marshal.SizeOf(typeof(ULONG)),
                                                                           sidAndAttributes,
                                                                           sidAndAttributes.Count);
            Marshal.WriteInt32(tokenGroups.ToIntPtr(), sidAndAttributes.Count);

            for(int Idx = 0; Idx < this.Count; ++Idx)
            {
                sidOps[Idx] = NativeMethods.AuthzSIDOperation.Replace;
            }

            if (!NativeMethods.AuthzModifySids(hAuthzClientContext,
                                               type == GroupType.User
                                               ? NativeMethods.AuthzContextInformationClass.AuthzContextInfoGroupsSids
                                               : NativeMethods.AuthzContextInformationClass.AuthzContextInfoDeviceSids,
                                               sidOps,
                                               tokenGroups.ToIntPtr()))
            {
                return Marshal.GetLastWin32Error();
            }

            return Win32Error.ERROR_SUCCESS;
        }
        #endregion

        #region Private members
        GroupType type;
        #endregion

        #region Nested class for P/Invokes
        static class NativeMethods
        {

            public enum AuthzContextInformationClass : uint
            {
                AuthzContextInfoGroupsSids = 2,
                AuthzContextInfoDeviceSids = 12,
            };

            public enum AuthzSIDOperation : uint
            {
                None = 0,
                ReplaceAll,
                Add,
                Delete,
                Replace
            }

            [Flags]
            public enum GroupAddtibute : uint // ULONG
            {
                Enabled = 0x00000004,
            }

            [StructLayout(LayoutKind.Sequential)]
            public struct SID_AND_ATTRIBUTES
            {
                public PSID Sid;
                public GroupAddtibute Attributes;

                public SID_AND_ATTRIBUTES(PSID sid, GroupAddtibute attribute)
                {
                    Sid = sid;
                    Attributes = attribute;
                }
            }

            [DllImport(Win32.AUTHZ_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool AuthzModifySids(
                AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
                AuthzContextInformationClass infoClass,
                AuthzSIDOperation[] claimOperation,
                IntPtr tokenGroups);
        }
        #endregion
    }
}
