//-----------------------------------------------------------------------
// <copyright file="RbacSystem.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.RoleBasedPlugins
{
    using System;
    using System.Collections.Generic;
    using System.Management.Automation.Remoting;
    using System.Security.Principal;

    /// <summary>
    /// Role Based management system implementation
    /// </summary>
    internal class RbacSystem
    {
        /// <summary> static instance of RbacSystem </summary>
        private static RbacSystem current = null;

        /// <summary> Lock to modify the static instance of RbacSystem </summary>
        private static object syncObject = new object();

        /// <summary>
        /// Prevents a default instance of the RbacSystem class from being created.
        /// </summary>
        private RbacSystem()
        {
            this.Users = new List<RbacUser>();
            this.Groups = new List<RbacGroup>();

            this.Populate(Utils.GetRbacFilePath());
        }

        /// <summary> Gets singleton instance of the RbacSystem class</summary>
        public static RbacSystem Current
        {
            get
            {
                if (RbacSystem.current == null)
                {
                    lock (RbacSystem.syncObject)
                    {
                        if (RbacSystem.current == null)
                        {
                            RbacSystem.current = new RbacSystem();
                        }
                    }
                }

                return RbacSystem.current;
            }
        }

        /// <summary> Gets collection of users registered in the system </summary>
        public List<RbacUser> Users { get; private set; }

        /// <summary> Gets collection of groups present in the system </summary>
        public List<RbacGroup> Groups { get; private set; }

        /// <summary>
        /// Authorizes a user 
        /// For authorized users, it returns the WindowsIdentity in which context commands need to be executed
        /// </summary>
        /// <param name="userInfo">User information</param>
        /// <param name="quota">User quota value</param>
        /// <returns>WindowsIdentiy in which context commands need to be executed</returns>
        public WindowsIdentity AuthorizeUser(RbacUser.RbacUserInfo userInfo, out Microsoft.Management.Odata.UserQuota quota)
        {
            RbacUser user = this.FindUser(userInfo);
            quota = new Microsoft.Management.Odata.UserQuota(user.Quota.MaxConcurrentRequests, user.Quota.MaxRequestsPerTimeSlot, user.Quota.Timeslot);

            return user.Group.GetWindowsIdentity(userInfo.WindowsIdentity);
        }

        /// <summary>
        /// Gets management system execution state membershipId for a user
        /// </summary>
        /// <param name="userInfo">User information</param>
        /// <returns>Managment system execution state membershipId</returns>
        public string GetMembershipId(RbacUser.RbacUserInfo userInfo)
        {
            return this.FindUser(userInfo).GetMembershipId();
        }

        /// <summary>
        /// Gets collection of cmdlets for a user
        /// </summary>
        /// <param name="userInfo">User information</param>
        /// <returns>Collection of cmdlet names </returns>
        public List<string> GetCmdlets(PSPrincipal userInfo)
        {
            RbacGroup group = this.FindGroup(userInfo);
            return new List<string>(group.Cmdlets);
        }

        /// <summary>
        /// Gets collection of scripts for a user
        /// </summary>
        /// <param name="userInfo">User information</param>
        /// <returns>Collection of scripts</returns>
        public List<string> GetScripts(PSPrincipal userInfo)
        {
            RbacGroup group = this.FindGroup(userInfo);
            return new List<string>(group.Scripts);
        }

        /// <summary>
        /// Gets collection of modules for a user
        /// </summary>
        /// <param name="userInfo">User information </param>
        /// <returns>Collection of module names</returns>
        public List<string> GetModules(PSPrincipal userInfo)
        {
            RbacGroup group = this.FindGroup(userInfo);
            return new List<string>(group.Modules);
        }

        /// <summary>
        /// Populates the RbacSystem from an RBAC configuration file
        /// </summary>
        /// <param name="configPath">full path to the config file</param>
        private void Populate(string configPath)
        {
            this.Reset();

            XmlConfiguration rbacConfiguration = XmlConfiguration.Create(configPath);

            foreach (XmlGroup group in rbacConfiguration.Groups)
            {
                WindowsIdentity identity = null;

                try
                {
                    if (group.UserName == null || group.Password == null)
                    {
                        if (group.UserName != null || group.Password != null)
                        {
                            if (group.UserName == null)
                            {
                                throw new ArgumentException("User name is null for group " + group.Name);
                            }

                            if (group.Password == null)
                            {
                                throw new ArgumentException("Password is null for group " + group.Name);
                            }
                        }
                    }
                    else
                    {
                        if (group.DomainName == null)
                        {
                            group.DomainName = Environment.MachineName;
                        }

                        identity = WindowsIdentityHelper.GetWindowsIdentity(group.UserName, group.Password, group.DomainName);
                    }
                }
                catch (Exception)
                {
                    // Not able to get the impersonated WindowsIdentity
                    // use the current WindowsIdentity
                    identity = WindowsIdentity.GetCurrent();
                }

                this.Groups.Add(new RbacGroup(group));
            }

            foreach (XmlUser userConfig in rbacConfiguration.Users)
            {
                RbacUser user = new RbacUser(new RbacUser.RbacUserInfo(userConfig.Name, userConfig.AuthenticationType, userConfig.DomainName), userConfig.Quota);
                RbacGroup group = this.Groups.Find(item => item.Name == userConfig.GroupName);
                if (group == null)
                {
                    throw new ArgumentException("Group not found = " + userConfig.GroupName);
                }

                user.Group = group;

                this.Users.Add(user);
            }
        }

        /// <summary>
        /// restores all collections to default/empty values
        /// </summary>
        private void Reset()
        {
            this.Users.Clear();
            this.Groups.Clear();
        }

        /// <summary>
        /// Finds group for a PSPrincipal
        /// </summary>
        /// <param name="principal">PSPrincipal instance</param>
        /// <returns>Group associated with the identity</returns>
        private RbacGroup FindGroup(PSPrincipal principal)
        {
            if (principal == null)
            {
                throw new ArgumentNullException("principal");
            }

            if (principal.Identity == null)
            {
                throw new ArgumentException("Null identity passed");
            }

            if (principal.Identity.IsAuthenticated == false)
            {
                throw new UnauthorizedAccessException();
            }

            PSIdentity powerShellIdentity = principal.Identity;

            GenericIdentity identity = new GenericIdentity(powerShellIdentity.Name, powerShellIdentity.AuthenticationType);

            RbacUser.RbacUserInfo userInfo = new RbacUser.RbacUserInfo(identity, powerShellIdentity.CertificateDetails);
            RbacUser user = this.Users.Find(item => item.UserInfo.Equals(userInfo));
            if (user == null)
            {
                throw new ArgumentException("User not found: name=" + userInfo.Name + ", authentication=" + userInfo.AuthenticationType);
            }

            RbacGroup group = this.Groups.Find(item => item.Name == user.Group.Name);
            if (group == null)
            {
                throw new ArgumentException("group not found = " + user.Group.Name);
            }

            return group;
        }

        /// <summary>
        /// Finds a user in the RbacSytem
        /// </summary>
        /// <param name="userInfo">User information</param>
        /// <returns>User from RbacSystem which was searched</returns>
        private RbacUser FindUser(RbacUser.RbacUserInfo userInfo)
        {
            RbacUser user = this.Users.Find(item => item.UserInfo.Equals(userInfo));
            if (user == null)
            {
                throw new ArgumentException("User not found. Name = " + userInfo.Name + " Authentication Type = " + userInfo.AuthenticationType);
            }

            return user;
        }
    }
}