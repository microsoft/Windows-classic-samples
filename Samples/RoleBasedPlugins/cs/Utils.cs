//-----------------------------------------------------------------------
// <copyright file="Utils.cs" company="Microsoft Corporation">
//     Copyright (C) 2011 Microsoft Corporation
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.RoleBasedPlugins
{
    using System.IO;
    using System.Web;

    /// <summary>
    /// Helper class with some utility functions
    /// </summary>
    internal static class Utils
    {
        /// <summary>
        /// Get base path
        /// </summary>
        /// <returns>Gets the base path to find the RBAC configuration file</returns>
        public static string GetBasePath()
        {
            string path = null;
            if (HttpContext.Current != null)
            {
                path = HttpContext.Current.Server.MapPath(".");
            }
            else
            {
                path = Directory.GetCurrentDirectory();
            }

            string basePath = System.Configuration.ConfigurationManager.AppSettings["BasePath"];
            if (string.IsNullOrEmpty(basePath) == false)
            {
                return Path.Combine(path, basePath);
            }
            else
            {
                return path;
            }
        }

        /// <summary>
        /// Gets RBAC configuration path
        /// </summary>
        /// <returns>Rbac configuratio file path</returns>
        public static string GetRbacFilePath()
        {
            string rbacFileName = System.Configuration.ConfigurationManager.AppSettings["RbacFileName"];
            if (string.IsNullOrEmpty(rbacFileName))
            {
                rbacFileName = "RbacConfiguration.xml";
            }

            string basePath = System.Configuration.ConfigurationManager.AppSettings["BasePath"];
            if (string.IsNullOrEmpty(basePath) == false)
            {
                rbacFileName = Path.Combine(basePath, rbacFileName);
            }

            string path = null;
            if (HttpContext.Current != null)
            {
                path = HttpContext.Current.Server.MapPath(".");
            }
            else
            {
                path = Directory.GetCurrentDirectory();
            }

            return Path.Combine(path, rbacFileName);
        }
    }
}