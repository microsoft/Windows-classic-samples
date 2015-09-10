// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.DirectoryServices;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text;

namespace Microsoft.Samples.DynamicAccessControl
{
    using DWORD = System.UInt32;
    using ULONG = System.UInt32;

    using PSID = System.IntPtr;
    using PGUID = System.IntPtr;
    using PSECURITY_DESCRIPTOR = System.IntPtr;
    using PDOMAIN_CONTROLLER_INFO = System.IntPtr;

    using Utility;

    /// <summary>
    /// Class to extract properties from a Central Access Rule object from AD
    /// </summary>
    internal class CentralAccessRule : IDisposable
    {
        #region Constructor
        public CentralAccessRule(string dnName)
        {
            carEntry = new DirectoryEntry("LDAP://" + dnName);
        }

        ~CentralAccessRule()
        {
            Dispose(false);
        }
        #endregion

        #region Public properties
        public string Name
        {
            get
            {
                return carEntry.Properties["cn"].Value as string;
            }
        }

        public RawSecurityDescriptor EffectivePolicy
        {
            get
            {
                return new RawSecurityDescriptor(carEntry.Properties["msAuthz-EffectiveSecurityPolicy"].Value as string);
            }
        }

        public string AppliesToCondition
        {
            get
            {
                return carEntry.Properties["msAuthz-ResourceCondition"].Value as string;
            }
        }
        #endregion

        #region IDisposable implementation
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (carEntry != null)
                {
                    carEntry.Dispose();
                    carEntry = null;
                }
            }
        }
        #endregion

        #region Private members
        DirectoryEntry carEntry;
        #endregion
    }

    /// <summary>
    /// Class to query for the Central Policies that are enforced on a target computer
    /// </summary>
    internal sealed class AvailableCentralPolicies
    {
        #region Constructor
        /// <summary>
        /// Determines the domain and the LDAP paths for Central Policy and Central Access Rule in AD
        /// to facilitate subsequent queries.
        /// </summary>
        /// <param name="target">Name of the computer from which to query</param>
        public AvailableCentralPolicies(string target)
        {
            PDOMAIN_CONTROLLER_INFO dsInfoPtr = PDOMAIN_CONTROLLER_INFO.Zero;
            try
            {
                DWORD result = NativeMethods.DsGetDcName(target,
                                                         null,
                                                         PGUID.Zero,
                                                         null,
                                                         NativeMethods.DsGetDcNameFlags.DirectoryServiceRequired,
                                                         out dsInfoPtr);
                if (result != Win32Error.ERROR_SUCCESS)
                {
                    throw new Win32Exception((int)result);
                }

                NativeMethods.DOMAIN_CONTROLLER_INFO dsInfo = (NativeMethods.DOMAIN_CONTROLLER_INFO)
                                                              Marshal.PtrToStructure(
                                                                        dsInfoPtr,
                                                                        typeof(NativeMethods.DOMAIN_CONTROLLER_INFO));

                string domainDN = "DC=" + string.Join(",DC=", dsInfo.domainName.Split('.'));

                capContainerDN = "CN=Central Access Policies," +
                                 "CN=Claims Configuration," +
                                 "CN=Services," +
                                 "CN=Configuration," +
                                 domainDN;

                availableCaps = GetAvailableCaps(target);
            }
            finally
            {
                NativeMethods.NetApiBufferFree(dsInfoPtr);
                dsInfoPtr = PDOMAIN_CONTROLLER_INFO.Zero;
            }
        }
        #endregion

        #region Public methods and properties
        /// <summary>
        /// The number of CAPs available on the target computer as reported by LsaGetAppliedCAPIDs
        /// </summary>
        public ULONG CapCount
        {
            get
            {
                return availableCaps != null? (ULONG)availableCaps.Count : 0;
            }
        }

        /// <summary>
        /// Retrieves the Central Access Rules for the Central Policy
        /// </summary>
        /// <param name="capId">Unique Identifier for the Central Policy</param>
        /// <returns>A Dictionary of CAR names and the Central Access Rule object</returns>
        public Dictionary<string, CentralAccessRule> FetchCentralAccessRules(SecurityIdentifier capId)
        {
            Dictionary<string, CentralAccessRule> carInfo = null;

            if (CapCount == 0 || !availableCaps.Contains(capId))
            {
                return null;
            }

            using (var CapContainer = new DirectoryEntry("LDAP://" + capContainerDN))
            {
                foreach (DirectoryEntry capEntry in CapContainer.Children)
                {
                    var entryId = capEntry.Properties["msAuthz-CentralAccessPolicyID"].Value;

                    if (entryId == null)
                    {
                        continue;
                    }

                    byte[] rawCapId = entryId as byte[];
                    if (rawCapId == null)
                    {
                        continue;
                    }

                    var entrySid = new SecurityIdentifier(rawCapId, 0);

                    if (capId == entrySid)
                    {
                        PropertyValueCollection CARs = capEntry.Properties["msAuthz-MemberRulesInCentralAccessPolicy"];

                        carInfo = new Dictionary<string, CentralAccessRule>(CARs.Count);

                        foreach (string carDN in CARs)
                        {
                            CentralAccessRule CAR = new CentralAccessRule(carDN);
                            carInfo.Add(CAR.Name, CAR);
                        }

                    }
                }
            }

            return carInfo;
        }
        #endregion

        #region Private methods
        static ICollection<SecurityIdentifier> GetAvailableCaps(string targetName)
        {
            var result = new List<SecurityIdentifier>();
            PSID capIdArray = PSID.Zero;

            try
            {
                var targetMachine = new NativeMethods.LSA_UNICODE_STRING(targetName);
                ULONG capCount;

                int ntStatus = NativeMethods.LsaGetAppliedCAPIDs(ref targetMachine, ref capIdArray, out capCount);
                if (!Win32.NT_SUCCESS(ntStatus))
                {
                    throw new Win32Exception(NativeMethods.LsaNtStatusToWinError(ntStatus));
                }

                if (capCount != 0 && capIdArray != PSID.Zero)
                {
                    PSID nextSid = capIdArray;
                    while(0 != capCount)
                    {
                        result.Add(new SecurityIdentifier(Marshal.ReadIntPtr(nextSid)));
                        nextSid += Marshal.SizeOf(typeof(IntPtr));
                        --capCount;
                    }
                }
            }
            finally
            {
                Marshal.FreeHGlobal(capIdArray);
            }

            return result;
        }
        #endregion

        #region Private members
        ICollection<SecurityIdentifier> availableCaps;
        string capContainerDN;
        #endregion

        #region Nested class for P/Invokes and native (Win32) structures
        static class NativeMethods
        {
            [Flags]
            public enum DsGetDcNameFlags : uint
            {
                DirectoryServiceRequired = 0x00000010,
            }

            [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
            public struct DOMAIN_CONTROLLER_INFO
            {
                public string dcName;
                public string dcAddress;
                public ULONG dcAddressType;
                public Guid domainGuid;
                public string domainName;
                public string dnsForestName;
                public DsGetDcNameFlags flags;
                public string dcSiteName;
                public string clientSiteName;
            }

            [DllImport(Win32.NETAPI32_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            public static extern DWORD DsGetDcName(string computerName,
                                                   string domainName,
                                                   PGUID domainGuid,
                                                   string siteName,
                                                   DsGetDcNameFlags flags,
                                                   out PDOMAIN_CONTROLLER_INFO dsInfo);

            [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
            public struct LSA_UNICODE_STRING
            {
                public ushort Length;
                public ushort MaximumLength;
                public string Buffer;

                public LSA_UNICODE_STRING(string str)
                {
                    if (str == null)
                    {
                        throw new ArgumentException("Null argument invalid", "str");
                    }

                    Debug.Assert(str.Length < ushort.MaxValue);
                    Length = MaximumLength = (ushort)(str.Length * UnicodeEncoding.CharSize);
                    Buffer = str;
                }

            }

            [DllImport(Win32.ADVAPI32_DLL, SetLastError = true, CharSet = CharSet.Unicode)]
            public static extern int LsaGetAppliedCAPIDs(
                ref LSA_UNICODE_STRING systemName,
                ref IntPtr CAPIDs,
                out ULONG CAPIDCount
                );

            [DllImport(Win32.ADVAPI32_DLL, SetLastError = false)]
            public static extern int LsaNtStatusToWinError(int status);

            public static void NetApiBufferFree(IntPtr buffer)
            {
                const int NERR_Success = 0;

                int nResult = NetApiFreeBuffer(buffer);

                //
                // A failure here is unexpected, but nevertheless nothing much
                // can be done to recover from this error when this does happen.
                //
                // Just assert the failure to flag any case of invalid argument.
                //
                Debug.Assert(nResult == NERR_Success);
            }

            [DllImport(Win32.NETAPI32_DLL, EntryPoint = "NetApiBufferFree")]
            private static extern int NetApiFreeBuffer(IntPtr Buffer);
        }
        #endregion
    }
}
