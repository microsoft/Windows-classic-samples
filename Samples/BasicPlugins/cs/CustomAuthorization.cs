//-----------------------------------------------------------------------
// <copyright file="CustomAuthorization.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.BasicPlugins
{
    using System.Configuration;
    using System.Globalization;
    using System.Security.Principal;
    using Microsoft.Management.Odata;

    /// <summary>
    /// Basic CustomAuthorization implementation
    /// Management OData service uses Microsoft.Management.Odata.CustomAuthorization interface to authorize a user.
    /// This is a pass-through implementation which means it authorizes all users.
    /// It gives same quota values for all users. The quota values can be overridden by following application settings:
    /// MaxConcurrentRequests: Overrides maximum number of concurrent requests for a user
    /// MaxRequestsPerTimeslot: Overrides maximum number of requests in a time slot
    /// TimeslotSize: Override size of time slot (in seconds)
    /// </summary>
    public class CustomAuthorization : Microsoft.Management.Odata.CustomAuthorization 
    {
        /// <summary>
        /// Default value of max concurrent requests
        /// </summary>
        private const int DefaultMaxConcurrentRequests = 10;

        /// <summary>
        /// Default value of max request per time slot
        /// </summary>
        private const int DefaultMaxRequestsPerTimeslot = 4;

        /// <summary>
        /// Default time slot size
        /// </summary>
        private const int DefaultTimeslotSize = 1;

        /// <summary>
        /// Default managemnet system state key
        /// </summary>
        private const string DefaultManagementSystemStateId = "E7D438A1-C0BA-49D6-952E-EF7C45CB737D";

        /// <summary>
        /// Authorize a user. 
        /// </summary>
        /// <param name="senderInfo">Sender information</param>
        /// <param name="userQuota">User quota value</param>
        /// <returns>User context in which to execute PowerShell cmdlet</returns>
        public override WindowsIdentity AuthorizeUser(SenderInfo senderInfo, out UserQuota userQuota)
        {
            var maxConcurrentRequests = ConfigurationManager.AppSettings["MaxConcurrentRequests"];
            var maxRequestsPerTimeslot = ConfigurationManager.AppSettings["MaxRequestsPerTimeslot"];
            var timeslotSize = ConfigurationManager.AppSettings["TimeslotSize"];

            userQuota = new UserQuota(
                maxConcurrentRequests != null ? int.Parse(maxConcurrentRequests, CultureInfo.CurrentUICulture) : DefaultMaxConcurrentRequests,
                maxRequestsPerTimeslot != null ? int.Parse(maxRequestsPerTimeslot, CultureInfo.CurrentUICulture) : DefaultMaxRequestsPerTimeslot,
                timeslotSize != null ? int.Parse(timeslotSize, CultureInfo.CurrentUICulture) : DefaultTimeslotSize);

            return WindowsIdentity.GetCurrent();
        }

        /// <summary>
        /// Gets membership id
        /// </summary>
        /// <param name="senderInfo">Sender information</param>
        /// <returns>Always returns same membership id for all users which means all users are in same group</returns>
        public override string GetMembershipId(SenderInfo senderInfo)
        {
            return DefaultManagementSystemStateId;
        }
    }
}