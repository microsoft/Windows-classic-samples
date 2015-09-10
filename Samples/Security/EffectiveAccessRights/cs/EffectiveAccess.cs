// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Runtime.InteropServices;
using System.Text;
using System.Linq;

using Microsoft.Win32.SafeHandles;

namespace Microsoft.Samples.DynamicAccessControl
{
    using Utility;

    using DWORD = System.UInt32;
    using ULONG = System.UInt32;

    using LPVOID = System.IntPtr;
    using PUCHAR = System.IntPtr;
    using PDWORD = System.IntPtr;

    using PSID = System.IntPtr;
    using PACL = System.IntPtr;
    using PLARGE_INTEGER = System.IntPtr;
    using PSECURITY_DESCRIPTOR = System.IntPtr;

    using AUTHZ_RESOURCE_MANAGER_HANDLE = System.IntPtr;
    using AUTHZ_CLIENT_CONTEXT_HANDLE = System.IntPtr;
    using AUTHZ_AUDIT_EVENT_HANDLE = System.IntPtr;
    using AUTHZ_ACCESS_CHECK_RESULTS_HANDLE = System.IntPtr;
    using PAUTHZ_RPC_INIT_INFO_CLIENT = System.IntPtr;
    using POBJECT_TYPE_LIST = System.IntPtr;
    using PACCESS_MASK = System.IntPtr;

    internal class EffectiveAccess
    {
        #region Constructor
        public EffectiveAccess(string path,
                               string targetMachine,
                               RawSecurityDescriptor shareSD,
                               SecurityIdentifier userSid,
                               SecurityIdentifier deviceSid,
                               ClaimValueDictionary userClaims,
                               ClaimValueDictionary deviceClaims,
                               GroupsCollection userGroups,
                               GroupsCollection deviceGroups)
        {
            if (string.IsNullOrEmpty(targetMachine) && shareSD != null)
            {
                throw new ArgumentException("targetMachine must be value when shareSD is not-empty", "targetMachine");
            }

            handle = NativeMethods.CreateFile(path,
                                                NativeMethods.FileAccess.GenericRead,
                                                NativeMethods.FileShare.Read
                                              | NativeMethods.FileShare.Write
                                              | NativeMethods.FileShare.Delete,
                                              IntPtr.Zero,
                                              NativeMethods.FileMode.OpenExisting,
                                              NativeMethods.FileFlagAttrib.BackupSemantics,
                                              IntPtr.Zero);
            if (handle.IsInvalid)
            {
                throw new Win32Exception(Marshal.GetLastWin32Error());
            }

            this.targetMachine = targetMachine;
            this.shareSD = shareSD;
            this.userSid = userSid;
            this.deviceSid = deviceSid;
            this.userClaims = userClaims;
            this.deviceClaims = deviceClaims;
            this.userGroups = userGroups;
            this.deviceGroups = deviceGroups;
        }
        #endregion

        #region Public methods
        /// <summary>
        /// Reports the results of computing effective access for the NTFSAccess list
        /// </summary>
        /// <remarks>This methods generates a report in the following format
        /// 
        /// Permission                         Access Granted     Limited By
        /// ----------                         --------------     ----------
        /// Full control                       [ N ]              Share permissions
        /// Traverse folder / execute file     [ N ]              File permissions
        /// List folder / read data            [ Y ]              -
        /// Read attributes                    [ Y ]              -
        /// Read extended attributes           [ N ]              Central Access Rule Name
        /// Create files / write data          [ N ]              Central Policy Name
        /// Create folders / append data       [ Y ]              -
        /// Write attributes                   [ Y ]              -
        /// Write extended attributes          [ Y ]              -
        /// Delete subfolders and files        [ Y ]              -
        /// Delete                             [ Y ]              -
        /// Read permissions                   [ Y ]              -
        /// Change permissions                 [ Y ]              -
        /// Take ownership                     [ Y ]              -
        /// </remarks>
        public void GenerateReport()
        {
            if (results == null)
            {
                return;
            }

            int COLUMN1_SPACING = COLUMN_SPACING + Math.Max(HEADER_PERMISSION.Length, MAX_NTFSACCESS_PERM_DESC_WIDTH);
            int COLUMN2_SPACING = COLUMN_SPACING + HEADER_ACCESS.Length;

            Console.Write(HEADER_PERMISSION.PadRight(COLUMN1_SPACING));
            Console.Write(HEADER_ACCESS.PadRight(COLUMN2_SPACING));
            Console.WriteLine(HEADER_LIMITED_BY);

            Console.Write((new String('-', HEADER_PERMISSION.Length)).PadRight(COLUMN1_SPACING));
            Console.Write((new String('-', HEADER_ACCESS.Length)).PadRight(COLUMN2_SPACING));
            Console.WriteLine(new String('-', HEADER_LIMITED_BY.Length));
            foreach (var permResult in results)
            {
                Console.Write(NTFSAccess[permResult.Key].PadRight(COLUMN1_SPACING));
                Console.Write((string.IsNullOrEmpty(permResult.Value) ? "[ Y ]" : "[ N ]").PadRight(COLUMN2_SPACING));
                Console.WriteLine(string.IsNullOrEmpty(permResult.Value) ? "-" : permResult.Value);
            }

        }

        /// <summary>
        /// Computes effective access rights upon acquiring the necessary security information such as Share's security,
        /// Central Policy and Resource's security as maybe applicable.
        /// </summary>
        public void Evaluate()
        {
            using (TokenPrivilege seSecurityPrivilege = new TokenPrivilege(TokenPrivilege.Security))
            {
                try
                {
                    //
                    // SeSecurityPrivilege is required for querying the Central Policy ID (Scope information) from the
                    // file's SD
                    //
                    seSecurityPrivilege.Enable();

                    var securityObjects = new Dictionary<string, FileSecurityObject>();

                    //
                    // First include the Share's SD if available. If the share denies a permission, anything else that
                    // denies a permission is of lesser consequence.
                    //
                    if (shareSD != null)
                    {
                        securityObjects.Add("Share", new FileSecurityObject(shareSD));
                    }

                    SecurityIdentifier capId = null;
                    {
                        RawSecurityDescriptor scopeSD = GetFileSecInfoSD(handle,
                                                                         NativeMethods.SecurityInformationClass.Scope);
                        if (scopeSD != null)
                        {
                            if (scopeSD.SystemAcl != null)
                            {
                                foreach (GenericAce ace in scopeSD.SystemAcl)
                                {
                                    //
                                    // Scoped Policy ID ACE not yet part of System.Security.AccessControl.AceType
                                    //
                                    // Since SYSTEM_SCOPED_POLICY_ID_ACE essentially has a SID which is the identifier
                                    // for a Central Policy, extract it explicitly.
                                    //
                                    byte[] rawAce = new byte[ace.BinaryLength];
                                    ace.GetBinaryForm(rawAce, 0);

                                    if ((ace.AceFlags & AceFlags.InheritOnly) == 0)
                                    {
                                        long sidOffset = (long)Marshal.OffsetOf(
                                                                    typeof(NativeMethods.SYSTEM_SCOPED_POLICY_ID_ACE),
                                                                    "SidStart");

                                        if (sidOffset < ace.BinaryLength)
                                        {
                                            capId = new SecurityIdentifier(rawAce, (int)sidOffset);

                                            //
                                            // The first Central Policy is the only one honored when there are multiple
                                            // in a Security Descriptor. Ignore everything else.
                                            //
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    //
                    // Obtain file SD
                    //
                    RawSecurityDescriptor fileSD = GetFileSecInfoSD(
                                                        handle,
                                                          NativeMethods.SecurityInformationClass.Owner
                                                        | NativeMethods.SecurityInformationClass.Group
                                                        | NativeMethods.SecurityInformationClass.Dacl
                                                        | NativeMethods.SecurityInformationClass.Label
                                                        | NativeMethods.SecurityInformationClass.Attribute);
                    if (capId != null)
                    {
                        var availableCaps = new AvailableCentralPolicies(targetMachine);

                        Dictionary<string, CentralAccessRule> CARs = availableCaps.FetchCentralAccessRules(capId);

                        if (CARs == null || CARs.Count == 0)
                        {
                            //
                            // If the Central Policy or any of its Central Access Rules could not be retrieved, fall
                            // back to just the granularity of Central Policy. For this, use the file's SD with the
                            // CAP ID.
                            //
                            securityObjects.Add("Central Policy",
                                                new FileSecurityObject(
                                                        GetFileSecInfoSD(handle,
                                                          NativeMethods.SecurityInformationClass.Owner
                                                        | NativeMethods.SecurityInformationClass.Group
                                                        | NativeMethods.SecurityInformationClass.Label
                                                        | NativeMethods.SecurityInformationClass.Attribute
                                                        | NativeMethods.SecurityInformationClass.Scope)));
                        }
                        else
                        {
                            //
                            // The merged security descriptor for each Central Access Rule is generated as follows:
                            //   Owner + Group + SACL (all from resource's SD) + DACL from Central Access Rule
                            //
                            // If the AppliesTo predicate is present, then we do the following:
                            //   Owned + Group + SACL (all from resource's SD)
                            //   + A fabricated DACL with an allowed callback ACE containingn the selected user
                            //     principal and the AppliesTo predicate appended.
                            //
                            foreach (var rule in CARs)
                            {
                                var sd = new RawSecurityDescriptor(fileSD.ControlFlags,
                                                                   fileSD.Owner,
                                                                   fileSD.Group,
                                                                   fileSD.SystemAcl,
                                                                   rule.Value.EffectivePolicy.DiscretionaryAcl);

                                if (!string.IsNullOrEmpty(rule.Value.AppliesToCondition))
                                {
                                    string appliesToSDDL = "D:(XA;;FR;;;" + userSid.ToString() + ";" +
                                                           rule.Value.AppliesToCondition + ")";

                                    var appliesToSD = new RawSecurityDescriptor(
                                                            fileSD.ControlFlags,
                                                            fileSD.Owner,
                                                            fileSD.Group,
                                                            fileSD.SystemAcl,
                                                            (new RawSecurityDescriptor(appliesToSDDL)).DiscretionaryAcl);

                                    securityObjects.Add(rule.Key, new FileSecurityObject(sd, appliesToSD));
                                }
                                else
                                {
                                    securityObjects.Add(rule.Key, new FileSecurityObject(sd));
                                }
                            }
                        }
                    }

                    securityObjects.Add("File permissions", new FileSecurityObject(fileSD));

                    results = ComputeEffectiveAccessResult(targetMachine, securityObjects);
                }
                finally
                {
                    seSecurityPrivilege.Revert();
                }
            }
        }
        #endregion

        #region Private implementation
        /// <summary>
        /// Computes the effective access permissions
        /// </summary>
        /// <param name="userSid">User account for which effective access rights are to be determined</param>
        /// <param name="deviceSid">Account to simulate the device where from the resource is being accessed</param>
        /// <param name="serverName">Target machine on which the resource</param>
        /// <param name="securityObjects">Security objects that affect that result of the access check</param>
        /// <returns>Dictionary of rights from NTFSAccess and the security object name that limits access to this
        /// permission</returns>
        Dictionary<NativeMethods.FileAccess, string> ComputeEffectiveAccessResult(
                                                        string serverName,
                                                        Dictionary<string, FileSecurityObject> securityObjects)
        {
            AUTHZ_CLIENT_CONTEXT_HANDLE userClientCtxt = AUTHZ_CLIENT_CONTEXT_HANDLE.Zero;
            AUTHZ_CLIENT_CONTEXT_HANDLE deviceClientCtxt = AUTHZ_CLIENT_CONTEXT_HANDLE.Zero;
            AUTHZ_CLIENT_CONTEXT_HANDLE compoundCtxt = AUTHZ_CLIENT_CONTEXT_HANDLE.Zero;

            try
            {
                var rpcInitInfo = new NativeMethods.AUTHZ_RPC_INIT_INFO_CLIENT();

                rpcInitInfo.version = NativeMethods.AuthzRpcClientVersion.V1;
                rpcInitInfo.objectUuid = NativeMethods.AUTHZ_OBJECTUUID_WITHCAP;
                rpcInitInfo.protocol = NativeMethods.RCP_OVER_TCP_PROTOCOL;
                rpcInitInfo.server = serverName;

                SafeAuthzRMHandle authzRM;

                SafeHGlobalHandle pRpcInitInfo = SafeHGlobalHandle.AllocHGlobalStruct(rpcInitInfo);
                if (!NativeMethods.AuthzInitializeRemoteResourceManager(pRpcInitInfo.ToIntPtr(), out authzRM))
                {
                    int error = Marshal.GetLastWin32Error();

                    if (error != Win32Error.EPT_S_NOT_REGISTERED)
                    {
                        throw new Win32Exception(error);
                    }

                    //
                    // As a fallback we do AuthzInitializeResourceManager. But the results can be inaccurate.
                    //
                    Helper.LogWarning("The effective rights can only be computed based on group membership on this" +
                                      " computer. For more accurate results, calculate effective access rights on " +
                                      "the target computer.", true);

                    if (!NativeMethods.AuthzInitializeResourceManager(
                                    NativeMethods.AuthzResourceManagerFlags.NO_AUDIT,
                                    IntPtr.Zero,
                                    IntPtr.Zero,
                                    IntPtr.Zero,
                                    "EffectiveAccessCheck",
                                    out authzRM))
                    {
                        throw new Win32Exception(Marshal.GetLastWin32Error());
                    }
                }

                byte[] rawSid = new byte[userSid.BinaryLength];
                userSid.GetBinaryForm(rawSid, 0);

                //
                // Create an AuthZ context based on the user account
                //
                if (!NativeMethods.AuthzInitializeContextFromSid(NativeMethods.AuthzInitFlags.Default,
                                                                 rawSid,
                                                                 authzRM,
                                                                 PLARGE_INTEGER.Zero,
                                                                 Win32.LUID.NullLuid,
                                                                 LPVOID.Zero,
                                                                 out userClientCtxt))
                {
                    Win32Exception win32Expn = new Win32Exception(Marshal.GetLastWin32Error());

                    if (win32Expn.NativeErrorCode != Win32Error.RPC_S_SERVER_UNAVAILABLE)
                    {
                        throw win32Expn;
                    }

                    Helper.LogWarning(string.Format(CultureInfo.CurrentCulture,
                                              "{0}. Please enable the inward firewall rule: Netlogon Service " +
                                              "Authz(RPC), on the target machine and try again.", win32Expn.Message),
                                      true);
                    return null;
                }

                //
                // Create an Authz Compound context based on the userClientCtxt and the device account
                //
                if (deviceSid != null)
                {
                    rawSid = new byte[deviceSid.BinaryLength];
                    deviceSid.GetBinaryForm(rawSid, 0);

                    if (!NativeMethods.AuthzInitializeContextFromSid(NativeMethods.AuthzInitFlags.Default,
                                                                     rawSid,
                                                                     authzRM,
                                                                     PLARGE_INTEGER.Zero,
                                                                     Win32.LUID.NullLuid,
                                                                     LPVOID.Zero,
                                                                     out deviceClientCtxt))
                    {
                        throw new Win32Exception(Marshal.GetLastWin32Error());
                    }

                    if (!NativeMethods.AuthzInitializeCompoundContext(userClientCtxt,
                                                                      deviceClientCtxt,
                                                                      out compoundCtxt))
                    {
                        throw new Win32Exception(Marshal.GetLastWin32Error());
                    }
                }
                else
                {
                    compoundCtxt = userClientCtxt;
                }

                //
                // Modify user claims in the Authz context
                //
                if (userClaims.Count != 0)
                {
                    int result = userClaims.ApplyClaims(compoundCtxt);

                    if (result != Win32Error.ERROR_SUCCESS)
                    {
                        throw new Win32Exception(result);
                    }
                }

                //
                // Modify device claims in the Authz context
                //
                if (deviceClaims.Count != 0)
                {
                    int result = deviceClaims.ApplyClaims(compoundCtxt);

                    if (result != Win32Error.ERROR_SUCCESS)
                    {
                        throw new Win32Exception(result);
                    }
                }

                //
                // Include additional group membership in the Authz context
                //
                if (userGroups.Count != 0)
                {
                    int result = userGroups.ApplyGroups(compoundCtxt);

                    if (result != Win32Error.ERROR_SUCCESS)
                    {
                        throw new Win32Exception(result);
                    }
                }

                //
                // Include additional device membership in the Authz context
                //
                if (deviceGroups.Count != 0)
                {
                    int result = deviceGroups.ApplyGroups(compoundCtxt);

                    if (result != Win32Error.ERROR_SUCCESS)
                    {
                        throw new Win32Exception(result);
                    }
                }

                PACCESS_MASK[] grantedAccess = new PACCESS_MASK[securityObjects.Count];
                PDWORD[] errorSecObj = new PACCESS_MASK[securityObjects.Count];

                try
                {
                    uint Index = 0;
                    foreach (var securityObject in securityObjects)
                    {
                        NativeMethods.AUTHZ_ACCESS_REQUEST request = new NativeMethods.AUTHZ_ACCESS_REQUEST();
                        request.DesiredAccess = NativeMethods.StdAccess.MAXIMUM_ALLOWED;
                        request.PrincipalSelfSid = null;
                        request.ObjectTypeList = POBJECT_TYPE_LIST.Zero;
                        request.ObjectTypeListLength = 0;
                        request.OptionalArguments = LPVOID.Zero;

                        var reply = new NativeMethods.AUTHZ_ACCESS_REPLY();
                        reply.ResultListLength = 1;
                        reply.SaclEvaluationResults = PDWORD.Zero;
                        reply.GrantedAccessMask = grantedAccess[Index] = Marshal.AllocHGlobal(sizeof(uint));
                        reply.Error = errorSecObj[Index] = Marshal.AllocHGlobal(sizeof(uint));

                        byte[] rawSD = new byte[securityObject.Value.securityDescriptor.BinaryLength];
                        securityObject.Value.securityDescriptor.GetBinaryForm(rawSD, 0);

                        //
                        // If a security object has the AppliesTo predicate, then we are processing a Central Access
                        // Rule. If the AppliesTo predicate is not satisfied, we do not have to bother performing an
                        // access check on the Central Access Rule's DACL.
                        //
                        if (securityObject.Value.appliesTo != null)
                        {
                            byte[] rawSD2 = new byte[securityObject.Value.appliesTo.BinaryLength];
                            securityObject.Value.appliesTo.GetBinaryForm(rawSD2, 0);

                            if (!NativeMethods.AuthzAccessCheck(NativeMethods.AuthzACFlags.None,
                                                                compoundCtxt,
                                                                ref request,
                                                                AUTHZ_AUDIT_EVENT_HANDLE.Zero,
                                                                rawSD2,
                                                                null,
                                                                0,
                                                                ref reply,
                                                                AUTHZ_ACCESS_CHECK_RESULTS_HANDLE.Zero))
                            {
                                throw new Win32Exception(Marshal.GetLastWin32Error());
                            }

                            //
                            // The applies to DACL is D:(XA;;FR;;;<user-sid>;<resource condition>).
                            // Check if we were successful in acquiring this particular access
                            //
                            var whatWasGranted = (NativeMethods.FileAccess)Marshal.ReadInt32(grantedAccess[Index]);
                            if ((NativeMethods.FileAccess.ReadData & whatWasGranted) == 0)
                            {
                                securityObject.Value.result.grantedAccess = NativeMethods.FileAccess.CategoricalAll;
                                continue;
                            }
                        }

                        if (!NativeMethods.AuthzAccessCheck(NativeMethods.AuthzACFlags.None,
                                                    compoundCtxt,
                                                    ref request,
                                                    AUTHZ_AUDIT_EVENT_HANDLE.Zero,
                                                    rawSD,
                                                    null,
                                                    0,
                                                    ref reply,
                                                    AUTHZ_ACCESS_CHECK_RESULTS_HANDLE.Zero))
                        {
                            throw new Win32Exception(Marshal.GetLastWin32Error());
                        }

                        securityObject.Value.result.grantedAccess = (NativeMethods.FileAccess)
                                                                    Marshal.ReadInt32(grantedAccess[Index]);
                        ++Index;
                    }

                    //
                    // Build the effective permission results
                    //
                    var accessLimitedBy = new Dictionary<NativeMethods.FileAccess, string>();

                    foreach (var permission in NTFSAccess)
                    {
                        foreach (var securityObject in securityObjects)
                        {
                            if (!accessLimitedBy.ContainsKey(permission.Key))
                            {
                                accessLimitedBy.Add(permission.Key, "");
                            }

                            if ((permission.Key & securityObject.Value.result.grantedAccess) != permission.Key)
                            {
                                if (!string.IsNullOrEmpty(accessLimitedBy[permission.Key]))
                                {
                                    accessLimitedBy[permission.Key] += ", ";
                                }

                                accessLimitedBy[permission.Key] += securityObject.Key;
                            }
                        }
                    }

                    return accessLimitedBy;
                }
                finally
                {
                    for (int Index = 0 ; Index < grantedAccess.Length ; ++Index) 
                    {
                        Marshal.FreeHGlobal(grantedAccess[Index]);
                    }

                    for (int Index = 0; Index < errorSecObj.Length ; ++Index)
                    {
                        Marshal.FreeHGlobal(errorSecObj[Index]);
                    }
                }
            }
            finally
            {
                if (userClientCtxt != AUTHZ_CLIENT_CONTEXT_HANDLE.Zero)
                {
                    NativeMethods.AuthzFreeContext(userClientCtxt);
                    userClientCtxt = AUTHZ_CLIENT_CONTEXT_HANDLE.Zero;
                }

                if (deviceClientCtxt != AUTHZ_CLIENT_CONTEXT_HANDLE.Zero)
                {
                    NativeMethods.AuthzFreeContext(deviceClientCtxt);
                    deviceClientCtxt = AUTHZ_CLIENT_CONTEXT_HANDLE.Zero;

                    if (compoundCtxt != AUTHZ_CLIENT_CONTEXT_HANDLE.Zero)
                    {
                        NativeMethods.AuthzFreeContext(compoundCtxt);
                        compoundCtxt = AUTHZ_CLIENT_CONTEXT_HANDLE.Zero;
                    }
                }
            }
        }

        /// <summary>
        /// Retrieves a copy of the security descriptor for an object
        /// </summary>
        /// <param name="handle">A handle to the object from which to retrieve the security information</param>
        /// <param name="infoClass">Flags that indicate the type(s) of security information to retrieve.</param>
        /// <returns>RawSecurityDescriptor object with the requested type(s) of security information on success.
        /// </returns>
        static RawSecurityDescriptor GetFileSecInfoSD(SafeFileHandle handle,
                                                      NativeMethods.SecurityInformationClass infoClass)
        {
            PSECURITY_DESCRIPTOR tempSD = PSECURITY_DESCRIPTOR.Zero;
            try
            {
                uint error = NativeMethods.GetSecurityInfo(handle,
                                                    NativeMethods.ObjectType.File,
                                                    infoClass,
                                                    IntPtr.Zero,
                                                    IntPtr.Zero,
                                                    IntPtr.Zero,
                                                    IntPtr.Zero,
                                                    out tempSD);
                if (error != Win32Error.ERROR_SUCCESS)
                {
                    throw new Win32Exception(Marshal.GetLastWin32Error());
                }

                return new RawSecurityDescriptor(Helper.ConvertSecurityDescriptorToByteArray(tempSD), 0);
            }
            finally
            {
                Marshal.FreeHGlobal(tempSD);
                tempSD = PSECURITY_DESCRIPTOR.Zero;
            }
        }
        #endregion

        #region Private members
        static Dictionary<NativeMethods.FileAccess, string> NTFSAccess = new Dictionary<NativeMethods.FileAccess, string>()
        {
            { NativeMethods.FileAccess.GenericAll,          "Full control"},
            { NativeMethods.FileAccess.Execute,             "Traverse folder / execute file" },
            { NativeMethods.FileAccess.ReadData,            "List folder / read data" },
            { NativeMethods.FileAccess.ReadAttrib,          "Read attributes" },
            { NativeMethods.FileAccess.ReadExAttrib,        "Read extended attributes" },
            { NativeMethods.FileAccess.WriteData,           "Create files / write data" },
            { NativeMethods.FileAccess.AppendData,          "Create folders / append data" },
            { NativeMethods.FileAccess.WriteAttrib,         "Write attributes" },
            { NativeMethods.FileAccess.WriteExAttrib,       "Write extended attributes" },
            { NativeMethods.FileAccess.DeleteChild,         "Delete subfolders and files" },
            { NativeMethods.FileAccess.Delete,              "Delete" },
            { NativeMethods.FileAccess.ReadPermissions,     "Read permissions" },
            { NativeMethods.FileAccess.ChangePermissions,   "Change permissions" },
            { NativeMethods.FileAccess.TakeOwnership,       "Take ownership" },
        };

        static int MAX_NTFSACCESS_PERM_DESC_WIDTH = NTFSAccess.Max(n => n.Value.Length);
        static int COLUMN_SPACING = 5;

        const string HEADER_PERMISSION = "Permission";
        const string HEADER_ACCESS = "Access Granted";
        const string HEADER_LIMITED_BY = "Limited By";
        #endregion

        #region Nested class
        class FileSecurityObject
        {
            public RawSecurityDescriptor securityDescriptor;
            public RawSecurityDescriptor appliesTo;
            public AccessChkResult result;

            public FileSecurityObject()
            {
                securityDescriptor = null;
                appliesTo = null;
                result.grantedAccess = NativeMethods.FileAccess.None;
            }

            public FileSecurityObject(RawSecurityDescriptor SD)
            {
                securityDescriptor = SD;
                appliesTo = null;
                result.grantedAccess = NativeMethods.FileAccess.None;
            }

            public FileSecurityObject(RawSecurityDescriptor SD, RawSecurityDescriptor appliesSD)
            {
                securityDescriptor = SD;
                appliesTo = appliesSD;
                result.grantedAccess = NativeMethods.FileAccess.None;
            }

            public struct AccessChkResult
            {
                //public bool hasBeenEvaluated;
                public NativeMethods.FileAccess grantedAccess;
            };
        }
        #endregion

        #region Private members
        /// <summary>
        /// Handle to the file or folder
        /// </summary>
        SafeFileHandle handle;
        string targetMachine;
        RawSecurityDescriptor shareSD;
        SecurityIdentifier userSid;
        SecurityIdentifier deviceSid;
        ClaimValueDictionary userClaims;
        ClaimValueDictionary deviceClaims;
        GroupsCollection userGroups;
        GroupsCollection deviceGroups;

        /// <summary>
        /// Result of evaluating Effective Access
        /// </summary>
        /// <remarks>This is a map of permissions and a string identifying what
        /// (File permission, share permission, Central Access Rule, Central
        /// Polict etc.) denied this permission if any.</remarks>
        Dictionary<NativeMethods.FileAccess, string> results;
        #endregion

        #region Nested class for P/Invokes and native (Win32) structures
        static class NativeMethods
        {
            #region authz
            [StructLayout(LayoutKind.Sequential)]
            internal struct AUTHZ_ACCESS_REQUEST
            {
                public StdAccess DesiredAccess;
                public byte[] PrincipalSelfSid;
                public POBJECT_TYPE_LIST ObjectTypeList;
                public int ObjectTypeListLength;
                public LPVOID OptionalArguments;
            }

            [StructLayout(LayoutKind.Sequential)]
            internal struct AUTHZ_ACCESS_REPLY
            {
                public int ResultListLength;
                public PACCESS_MASK GrantedAccessMask;
                public PDWORD SaclEvaluationResults;
                public PDWORD Error;
            }

            internal enum AuthzACFlags : uint // DWORD
            {
                None = 0,
                NoDeepCopySD
            }

            [DllImport(Win32.AUTHZ_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            internal static extern bool AuthzAccessCheck(
                AuthzACFlags flags,
                AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
                ref AUTHZ_ACCESS_REQUEST pRequest,
                AUTHZ_AUDIT_EVENT_HANDLE AuditEvent,
                byte[] rawSecurityDescriptor,
                PSECURITY_DESCRIPTOR[] OptionalSecurityDescriptorArray,
                DWORD OptionalSecurityDescriptorCount,
                ref AUTHZ_ACCESS_REPLY pReply,
                AUTHZ_ACCESS_CHECK_RESULTS_HANDLE cachedResults);

            [DllImport(Win32.AUTHZ_DLL, CharSet = CharSet.Unicode, ExactSpelling = true, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            internal static extern bool AuthzFreeContext(AUTHZ_CLIENT_CONTEXT_HANDLE authzClientContext);

            internal enum AuthzRpcClientVersion : ushort // USHORT
            {
                V1 = 1
            }

            internal const string AUTHZ_OBJECTUUID_WITHCAP = "9a81c2bd-a525-471d-a4ed-49907c0b23da";

            internal const string RCP_OVER_TCP_PROTOCOL = "ncacn_ip_tcp";

            [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
            internal struct AUTHZ_RPC_INIT_INFO_CLIENT
            {
                public AuthzRpcClientVersion version;
                public string objectUuid;
                public string protocol;
                public string server;
                public string endPoint;
                public string options;
                public string serverSpn;
            }

            [DllImport(Win32.AUTHZ_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            internal static extern bool AuthzInitializeRemoteResourceManager(
                PAUTHZ_RPC_INIT_INFO_CLIENT rpcInitInfo,
                out SafeAuthzRMHandle authRM);

            [Flags]
            internal enum AuthzInitFlags : uint
            {
                Default           = 0x0,
                SkipTokenGroups   = 0x2,
                RequireS4ULogon   = 0x4,
                ComputePrivileges = 0x8,
            }

            [DllImport(Win32.AUTHZ_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            internal static extern bool AuthzInitializeContextFromSid(
                AuthzInitFlags flags,
                byte[] rawUserSid,
                SafeAuthzRMHandle authzRM,
                PLARGE_INTEGER expirationTime,
                Win32.LUID Identifier,
                LPVOID DynamicGroupArgs,
                out AUTHZ_CLIENT_CONTEXT_HANDLE authzClientContext);

            [DllImport(Win32.AUTHZ_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            internal static extern bool AuthzInitializeCompoundContext(
                AUTHZ_CLIENT_CONTEXT_HANDLE userClientContext,
                AUTHZ_CLIENT_CONTEXT_HANDLE deviceClientContext,
                out AUTHZ_CLIENT_CONTEXT_HANDLE compoundContext);

            [Flags]
            internal enum AuthzResourceManagerFlags : uint
            {
                NO_AUDIT = 0x1,
            }

            [DllImport(Win32.AUTHZ_DLL, CharSet = CharSet.Unicode, ExactSpelling = true, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            internal static extern bool AuthzInitializeResourceManager(
                AuthzResourceManagerFlags flags,
                IntPtr pfnAccessCheck,
                IntPtr pfnComputeDynamicGroups,
                IntPtr pfnFreeDynamicGroups,
                string szResourceManagerName,
                out SafeAuthzRMHandle phAuthzResourceManager);

            #endregion

            #region PInvoke kernel32
            [Flags]
            internal enum StdAccess : uint
            {
                None                     = 0x0,

                SYNCHRONIZE              = 0x100000,
                STANDARD_RIGHTS_REQUIRED = 0xF0000,

                MAXIMUM_ALLOWED          = 0x2000000,
            }

            [Flags]
            internal enum FileAccess : uint
            {
                None               = 0x0,
                ReadData           = 0x1,
                WriteData          = 0x2,
                AppendData         = 0x4,
                ReadExAttrib       = 0x8,
                WriteExAttrib      = 0x10,
                Execute            = 0x20,
                DeleteChild        = 0x40,
                ReadAttrib         = 0x80,
                WriteAttrib        = 0x100,

                Delete             = 0x10000,   // DELETE,
                ReadPermissions    = 0x20000,   // READ_CONTROL
                ChangePermissions  = 0x40000,   // WRITE_DAC,
                TakeOwnership      = 0x80000,   // WRITE_OWNER,

                GenericRead = ReadPermissions
                            | ReadData
                            | ReadAttrib
                            | ReadExAttrib
                            | StdAccess.SYNCHRONIZE,

                GenericAll = (StdAccess.STANDARD_RIGHTS_REQUIRED | 0x1FF),

                CategoricalAll     = uint.MaxValue
            }

            [Flags]
            internal enum FileShare : uint
            {
                None   = 0x0,
                Read   = 0x1,
                Write  = 0x2,
                Delete = 0x4
            }

            internal enum FileMode : uint
            {
                OpenExisting = 3,
            }

            [Flags]
            internal enum FileFlagAttrib : uint
            {
                BackupSemantics = 0x02000000,
            }

            [DllImport(Win32.KERNEL32_DLL, SetLastError = true, CharSet = CharSet.Unicode)]
            internal static extern SafeFileHandle CreateFile(string lpFileName,
                                                             FileAccess desiredAccess,
                                                             FileShare shareMode,
                                                             IntPtr lpSecurityAttributes,
                                                             FileMode mode,
                                                             FileFlagAttrib flagsAndAttributes,
                                                             IntPtr hTemplateFile);
            #endregion

            #region PInvoke advapi32
            [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
            internal struct ACE_HEADER
            {
                public byte AceType;
                public byte AceFlags;
                public ushort AceSize;
            }

            [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
            internal struct SYSTEM_SCOPED_POLICY_ID_ACE
            {
                public ACE_HEADER Header;
                public uint Mask;
                public uint SidStart;
            }

            internal enum ObjectType : uint
            {
                File = 1,
            }

            [Flags]
            internal enum SecurityInformationClass : uint
            {
                Owner     = 0x00001,
                Group     = 0x00002,
                Dacl      = 0x00004,
                Sacl      = 0x00008,
                Label     = 0x00010,
                Attribute = 0x00020,
                Scope     = 0x00040
            }

            [DllImport(Win32.ADVAPI32_DLL, CallingConvention = CallingConvention.Winapi, SetLastError = true, CharSet = CharSet.Unicode)]
            internal static extern DWORD GetSecurityInfo(
                SafeFileHandle handle,
                ObjectType objectType,
                SecurityInformationClass infoClass,
                PSID owner,
                PSID group,
                PACL dacl,
                PACL sacl,
                out PSECURITY_DESCRIPTOR securityDescriptor);
            #endregion
        }
        #endregion
    }
}
