//-----------------------------------------------------------------------
// <copyright file="RbacGroup.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.RoleBasedPlugins
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Security.Principal;

    /// <summary>
    /// Class represents a group in RBAC
    /// </summary>
    internal class RbacGroup
    {
        /// <summary>Default key guid</summary>
        private Guid keyGuid = Guid.NewGuid();

        /// <summary>
        /// Initializes a new instance of the RbacGroup class
        /// </summary>
        /// <param name="group">Group information</param>
        public RbacGroup(XmlGroup group)
            : this(group != null ? group.Name : string.Empty, group)
        {
        }

        /// <summary>
        /// Initializes a new instance of the RbacGroup class.
        /// </summary>
        /// <param name="groupName">Group name</param>
        /// <param name="group">Group configuration</param>
        public RbacGroup(string groupName, XmlGroup group)
        {
            if (string.IsNullOrEmpty(groupName))
            {
                throw new ArgumentException("groupName is passed as empty or null");
            }

            this.Name = groupName;
            
            if (group != null)
            {
                if (group.MapIncomingUser == true)
                {
                    this.MapIncomingUser = group.MapIncomingUser;

                    if (string.IsNullOrEmpty(group.UserName) == false || string.IsNullOrEmpty(group.Password) == false || string.IsNullOrEmpty(group.DomainName) == false)
                    {
                        throw new ArgumentException("Group " + groupName + " has defined incoming user to true and defined credential for mapped user. They are exclusive and only one can be defined.");
                    }
                }
                else
                {
                    this.UserName = group.UserName;
                    this.Password = group.Password;
                    this.DomainName = group.DomainName;
                }
            }

            if (group != null && group.Cmdlets != null)
            {
                this.Cmdlets = new List<string>(group.Cmdlets);
            }
            else
            {
                this.Cmdlets = new List<string>();
            }

            this.Scripts = new List<string>();

            if (group != null && group.Scripts != null)
            {
                foreach (string script in group.Scripts)
                {
                    this.Scripts.Add(script);
                }
            }

            this.Modules = new List<string>();

            if (group != null && group.Modules != null)
            {
                foreach (string module in group.Modules)
                {
                    this.Modules.Add(Path.Combine(Utils.GetBasePath(), module));
                }
            }
        }

        /// <summary> Gets name of the group </summary>
        public string Name { get; private set; }

        /// <summary> Gets collection of Commands supported in the group </summary>
        public List<string> Cmdlets { get; private set; }

        /// <summary> Gets collection of Scripts supported in the group </summary>
        public List<string> Scripts { get; private set; }

        /// <summary> Gets collection of Modules supported in the group </summary>
        public List<string> Modules { get; private set; }

        /// <summary> Gets a value indicating whether to use network client identity for executing a cmdlet </summary>
        public bool MapIncomingUser { get; private set; }

        /// <summary> Gets user name </summary>
        public string UserName { get; private set; }

        /// <summary> Gets password </summary>
        public string Password { get; private set; }

        /// <summary> Gets domain name </summary>
        public string DomainName { get; private set; }

        /// <summary>
        /// Gets the membershipId for the group
        /// </summary>
        /// <returns>Membership id of the group</returns>
        public string GetMembershipId()
        {
            return this.Name + this.keyGuid.ToString();
        }

        /// <summary>
        /// Gets Windows Identity associated with this group
        /// </summary>
        /// <param name="incomingIdentity">Incoming identity</param>
        /// <returns>Windows Identity associated with this group</returns>
        public WindowsIdentity GetWindowsIdentity(WindowsIdentity incomingIdentity)
        {
            WindowsIdentity identity = null;

            if (this.MapIncomingUser == true)
            {
                if (incomingIdentity == null)
                {
                    throw new ArgumentException("Current user is mapped to group " + this.Name + " which is expected to return context of the incoming user. But context of the incoming user passed is null.");
                }

                return incomingIdentity;
            }

            if (this.UserName == null || this.Password == null)
            {
                if (this.UserName == null && this.Password == null)
                {
                    identity = WindowsIdentityHelper.GetCurrentWindowsIdentity();
                }
                else
                {
                    if (this.UserName == null)
                    {
                        throw new ArgumentException("User name is null for group " + this.Name);
                    }

                    if (this.Password == null)
                    {
                        throw new ArgumentException("Password is null for group " + this.Name);
                    }
                }
            }
            else
            {
                identity = WindowsIdentityHelper.GetWindowsIdentity(this.UserName, this.Password, this.DomainName);
            }

            return identity;
        }
    }
}