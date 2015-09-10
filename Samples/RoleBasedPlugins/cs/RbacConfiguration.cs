//-----------------------------------------------------------------------
// <copyright file="RbacConfiguration.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.RoleBasedPlugins
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Xml;
    using System.Xml.Schema;
    using System.Xml.Serialization;

    /// <summary>
    /// Keeps Configuration for the RbacSystem
    /// It reads the RacSystem configuration for configuratin file and creates RbacConfiguration
    /// </summary>
    [Serializable]
    [XmlRoot("RbacConfiguration")]    
    public class XmlConfiguration
    {
        /// <summary>
        /// Initializes a new instance of the XmlConfiguration class
        /// </summary>
        public XmlConfiguration()
        {
            this.Users = new List<XmlUser>();
            this.Groups = new List<XmlGroup>();
        }
        
        /// <summary> Gets collection of groups </summary>
        [XmlArray("Groups")]
        [XmlArrayItem("Group", typeof(XmlGroup))]
        public List<XmlGroup> Groups { get; private set; }

        /// <summary> Gets collection of users </summary>
        [XmlArray("Users")]
        [XmlArrayItem("User", typeof(XmlUser))]
        public List<XmlUser> Users { get; private set; }

        /// <summary>
        /// Creates RbacConfiguration from Rbac configuration file
        /// </summary>
        /// <param name="configPath">full path to the config file</param>
        /// <returns>RbacConfiguration created from the configuration file</returns>
        public static XmlConfiguration Create(string configPath)
        {
            string configData = File.ReadAllText(configPath);

            try
            {
                XmlReader xsd = XmlReader.Create(new StringReader(Resources.rbac));

                XmlReaderSettings settings = new XmlReaderSettings();

                settings.IgnoreComments = true;
                settings.IgnoreProcessingInstructions = true;
                settings.IgnoreWhitespace = true;
                settings.XmlResolver = null;
                settings.ValidationType = ValidationType.Schema;

                settings.ValidationEventHandler += delegate(object sender, ValidationEventArgs args)
                {
                    throw new ArgumentException("Rbac configuration file is incorrect", args.Exception);
                };

                XmlSerializer serializer = new XmlSerializer(typeof(XmlConfiguration));

                using (XmlReader reader = XmlReader.Create(new StringReader(configData), settings))
                {
                    return serializer.Deserialize(reader) as XmlConfiguration;
                }
            }
            catch (XmlException)
            {
                throw;
            }
        }
    }

    /// <summary>
    /// Represents Group in the RbacConfiguration
    /// </summary>
    [Serializable]
    public class XmlGroup
    {
        /// <summary>
        /// Initializes a new instance of the XmlGroup class
        /// </summary>
        public XmlGroup()
        {
            this.Cmdlets = new List<string>();
            this.Scripts = new List<string>();
            this.Modules = new List<string>();
        }

        /// <summary> Gets or sets name of the group </summary>
        [XmlAttribute("Name")]
        public string Name { get; set; }

        /// <summary> Gets or sets user name of the user in which context commands are executed for this group </summary>
        [XmlAttribute("UserName")]
        public string UserName { get; set; }

        /// <summary> Gets or sets password of the user in which context commands are executed for this group </summary>
        [XmlAttribute("Password")]
        public string Password { get; set; }

        /// <summary> Gets or sets domain of the user in which context commands are executed for this group </summary>
        [XmlAttribute("DomainName")]
        public string DomainName { get; set; }

        /// <summary> Gets or sets a value indicating whether to map incoming user to the user context returned from custom authorization </summary>
        [XmlAttribute("MapIncomingUser")]
        public bool MapIncomingUser { get; set; }

        /// <summary> Gets collection of cmdlets in the group </summary>
        [XmlArray("Cmdlets")]
        [XmlArrayItem("Cmdlet", typeof(string))]
        public List<string> Cmdlets { get; private set; }

        /// <summary> Gets collection of cmdlets in the group </summary>
        [XmlArray("Scripts")]
        [XmlArrayItem("Script", typeof(string))]
        public List<string> Scripts { get; private set; }

        /// <summary> Gets collection of cmdlets in the group </summary>
        [XmlArray("Modules")]
        [XmlArrayItem("Module", typeof(string))]
        public List<string> Modules { get; private set; }
    }

    /// <summary>
    /// Represents User in the RbacConfiguration
    /// </summary>
    [Serializable]
    public class XmlUser
    {
        /// <summary> Gets or sets name of the user </summary>
        [XmlAttribute("Name")]
        public string Name { get; set; }

        /// <summary> Gets or sets authentication type used for the user </summary>
        [XmlAttribute("AuthenticationType")]
        public string AuthenticationType { get; set; }

        /// <summary> Gets or sets domain name of the user. If this is null/empty, domain is local machine </summary>
        [XmlAttribute("DomainName")]
        public string DomainName { get; set; }

        /// <summary> Gets or sets group in which the user has membership </summary>
        [XmlAttribute("GroupName")]
        public string GroupName { get; set; }

        /// <summary> Gets or sets quota for the user </summary>
        [XmlElement("Quota", typeof(XmlQuota))]
        public XmlQuota Quota { get; set; }
    }

    /// <summary>
    /// Represents quota for a user
    /// </summary>
    [Serializable]
    public class XmlQuota
    {
        /// <summary> Gets or sets maximum concurrent requests </summary>
        [XmlAttribute("MaxConcurrentRequests")]
        public int MaxConcurrentRequests { get; set; }

        /// <summary> Gets or sets maximum requests per time slot</summary>
        [XmlAttribute("MaxRequestsPerTimeslot")]
        public int MaxRequestsPerTimeslot { get; set; }

        /// <summary> Gets or sets time slot</summary>
        [XmlAttribute("Timeslot")]
        public int Timeslot { get; set; }
    }
}