//-----------------------------------------------------------------------
// <copyright file="RbacQuota.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.RoleBasedPlugins
{
    /// <summary>
    /// RBAC quota module class
    /// </summary>
    internal class RbacQuota
    {
        /// <summary> Default Maximum concurrent requests </summary>
        private const int DefaultMaxConcurrrentRequests = 10;

        /// <summary> Default Maximum requests per second</summary>
        private const int DefaultMaxRequestPerSec = 10;

        /// <summary>
        /// Initializes a new instance of the RbacQuota class
        /// </summary>
        /// <param name="quota">RBAC configuration quota value</param>
        public RbacQuota(XmlQuota quota)
        {
            if (quota == null)
            {
                this.MaxConcurrentRequests = DefaultMaxConcurrrentRequests;
                this.MaxRequestsPerTimeSlot = DefaultMaxRequestPerSec;
                this.Timeslot = 1;
            }
            else
            {
                this.MaxConcurrentRequests = quota.MaxConcurrentRequests;
                this.MaxRequestsPerTimeSlot = quota.MaxRequestsPerTimeslot;
                this.Timeslot = quota.Timeslot;
            }
        }

        /// <summary> Gets or sets maximum concurrent requests </summary>
        public int MaxConcurrentRequests { get; set; }

        /// <summary> Gets or sets maximum requests per time slot</summary>
        public int MaxRequestsPerTimeSlot { get; set; }

        /// <summary> Gets or sets Time slot</summary>
        public int Timeslot { get; set; }
    }
}