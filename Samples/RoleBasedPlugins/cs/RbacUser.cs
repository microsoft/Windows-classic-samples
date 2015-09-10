//-----------------------------------------------------------------------
// <copyright file="RbacUser.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.RoleBasedPlugins
{
    using System;
    using System.Management.Automation.Remoting;
    using System.Security.Principal;

    /// <summary>
    /// Class Represents a user in RBAC
    /// </summary>
    internal class RbacUser
    {
        /// <summary>
        /// Initializes a new instance of the RbacUser class.
        /// </summary>
        /// <param name="userInfo">Name of the user</param>
        /// <param name="quota">User quota value</param>
        public RbacUser(RbacUserInfo userInfo, XmlQuota quota)
        {
            this.UserInfo = userInfo;
            this.Quota = new RbacQuota(quota);
        }

        /// <summary> Gets user information </summary>
        public RbacUserInfo UserInfo { get; private set; }

        /// <summary> Gets or sets list of groups the user is member of </summary>
        public RbacGroup Group { get; set; }

        /// <summary> Gets quota limits for the user </summary>
        public RbacQuota Quota { get; private set; }

        /// <summary>
        /// Gets the membershipId of the user
        /// </summary>
        /// <returns>Membership Id of the user</returns>
        public string GetMembershipId()
        {
            return this.Group.GetMembershipId();
        }

        /// <summary>
        /// RBAC system user information 
        /// </summary>
        public class RbacUserInfo : IEquatable<RbacUserInfo>
        {
            /// <summary>
            /// Initializes a new instance of the RbacUserInfo class.
            /// </summary>
            /// <param name="name">Name of the user</param>
            /// <param name="authenticationType">Authentication type used</param>
            /// <param name="domainName">Domain name of the user. If the domain name is empty, localmachine name is used as domain</param>
            public RbacUserInfo(
                string name,
                string authenticationType,
                string domainName)
            {
                if (string.IsNullOrEmpty(domainName))
                {
                    domainName = Environment.MachineName;
                }

                this.Name = domainName + "\\" + name;
                this.AuthenticationType = authenticationType;
            }

            /// <summary>
            /// Initializes a new instance of the RbacUserInfo class.
            /// </summary>
            /// <param name="identity">User identity</param>
            public RbacUserInfo(IIdentity identity)
                : this(identity, null)
            {
            }

            /// <summary>
            /// Initializes a new instance of the RbacUserInfo class.
            /// </summary>
            /// <param name="identity">User identity</param>
            /// <param name="clientCert">Http client certificate</param>
            public RbacUserInfo(IIdentity identity, PSCertificateDetails clientCert)
            {
                if (identity == null)
                {
                    throw new ArgumentNullException("identity");
                }

                this.WindowsIdentity = identity as WindowsIdentity;

                this.Name = identity.Name;
                this.AuthenticationType = identity.AuthenticationType;
                this.CertificateDetails = clientCert;
            }

            /// <summary> Gets name of the user </summary>
            public string Name { get; private set; }

            /// <summary> Gets authentication type</summary>
            public string AuthenticationType { get; private set; }

            /// <summary> Gets windows identity for the user </summary>
            public WindowsIdentity WindowsIdentity { get; private set; }

            /// <summary>
            /// Gets details of the (optional) client certificate
            /// </summary>
            public PSCertificateDetails CertificateDetails { get; private set; }

            /// <summary>
            /// compare two PSCredentialDetails for equivalence.  
            /// </summary>
            /// <param name="first">one set of details</param>
            /// <param name="second">the other set of details</param>
            /// <returns>true if they refer to the same certificate</returns>
            public static bool AreSame(PSCertificateDetails first, PSCertificateDetails second)
            {
                if (first == null && second == null)
                {
                    return true;
                }

                if (first == null || second == null)
                {
                    return false;
                }

                if (string.Equals(first.IssuerName, second.IssuerName, StringComparison.OrdinalIgnoreCase) == false ||
                    string.Equals(first.Subject, second.Subject, StringComparison.OrdinalIgnoreCase) == false ||
                    string.Equals(first.IssuerThumbprint, second.IssuerThumbprint, StringComparison.OrdinalIgnoreCase) == false)
                {
                    return false;
                }

                return true;
            }

            /// <summary>
            /// Indicates whether the current object is equal to another object of the same type.
            /// </summary>
            /// <param name="other">Other RbacUserInfo instance</param>
            /// <returns>True if the current object is equal to the other parameter; otherwise, False.</returns>
            public bool Equals(RbacUserInfo other)
            {
                if (other == null)
                {
                    return false;
                }

                if (string.Equals(this.Name, other.Name, StringComparison.OrdinalIgnoreCase) == false ||
                    string.Equals(this.AuthenticationType, other.AuthenticationType, StringComparison.OrdinalIgnoreCase) == false ||
                    AreSame(this.CertificateDetails, other.CertificateDetails) == false)
                {
                    return false;
                }

                return true;
            }

            /// <summary>
            /// Indicates whether the current object is equal to another object of the object type.
            /// </summary>
            /// <param name="other">Other object instance</param>
            /// <returns>true, if both instace are same else false</returns>
            public override bool Equals(object other)
            {
                return this.Equals(other as RbacUserInfo);
            }

            /// <summary>
            /// Gets hash code for the object
            /// </summary>
            /// <returns>Hash code for the object</returns>
            public override int GetHashCode()
            {
                return base.GetHashCode();
            }
        }
    }
}