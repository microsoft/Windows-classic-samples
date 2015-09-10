//-----------------------------------------------------------------------
// <copyright file="CustomAuthorization.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.RoleBasedPlugins
{
    using System;
    using System.Security.Principal;
    using Microsoft.Management.Odata;

    /// <summary>
    /// Custom Authorization implementation
    /// </summary>
    public class CustomAuthorization : Microsoft.Management.Odata.CustomAuthorization 
    {
        /// <summary>
        /// Authorizes a user
        /// </summary>
        /// <param name="senderInfo">User information</param>
        /// <param name="quota">Returns user quota</param>
        /// <returns>WindowsIdentity, if the user is authorized else throws an exception</returns>
        public override WindowsIdentity AuthorizeUser(SenderInfo senderInfo, out UserQuota quota)
        {
            if ((senderInfo == null) || (senderInfo.Principal == null) || (senderInfo.Principal.Identity == null))
            {
                throw new ArgumentNullException("senderInfo");
            }

            if (senderInfo.Principal.Identity.IsAuthenticated == false)
            {
                throw new ArgumentException("User is not authenticated");
            }

            RbacUser.RbacUserInfo userInfo = null;
            if (senderInfo.Principal.WindowsIdentity != null)
            {
                userInfo = new RbacUser.RbacUserInfo(senderInfo.Principal.WindowsIdentity);
            }
            else
            {
                userInfo = new RbacUser.RbacUserInfo(senderInfo.Principal.Identity);
            }

            return RbacSystem.Current.AuthorizeUser(userInfo, out quota);
        }

        /// <summary>
        /// Gets membership id
        /// </summary>
        /// <param name="senderInfo">Sender information</param>
        /// <returns>Collection of management system execution state keys</returns>
        public override string GetMembershipId(SenderInfo senderInfo)
        {
            if ((senderInfo == null) || (senderInfo.Principal == null) || (senderInfo.Principal.Identity == null))
            {
                throw new ArgumentNullException("senderInfo");
            }

            return RbacSystem.Current.GetMembershipId(new RbacUser.RbacUserInfo(senderInfo.Principal.Identity));
        }
    }
}