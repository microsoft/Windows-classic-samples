// <copyright file="HostUtilities.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

namespace Microsoft.Samples.PowerShell.Host
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Management.Automation;
    using System.Reflection;
    using Microsoft.Win32;

    /// <summary>
    /// Helper functions for setting $profile.
    /// </summary>
    public class HostUtilities
    {
        /// <summary>
        /// Root registry key for Windows PowerShell.
        /// </summary>
        internal const string PowerShellRootKeyPath = "Software\\Microsoft\\PowerShell";
        
        /// <summary>
        /// Engine registry key.
        /// </summary>
        internal const string PowerShellEngineKey = "PowerShellEngine";
        
        /// <summary>
        /// Registry key used to determine the directory where Windows  
        /// PowerShell is installed. 
        /// </summary>
        internal const string PowerShellEngineApplicationBase = "ApplicationBase";
        
        /// <summary>
        /// Windows PowerShell version registry key.
        /// </summary>
        internal const string RegistryVersionKey = "1";

        /// <summary>
        /// Gets a PSObject whose base object is currentUserCurrentHost 
        /// and with notes for the other 4 parameters.
        /// </summary>
        /// <param name="allUsersAllHosts">The profile file name for all 
        /// users and all hosts.</param>
        /// <param name="allUsersCurrentHost">The profile file name for 
        /// all users and current host.</param>
        /// <param name="currentUserAllHosts">The profile file name for 
        /// cuurrent user and all hosts.</param>
        /// <param name="currentUserCurrentHost">The profile name for 
        /// current user and current host.</param>
        /// <returns>A PSObject whose base object is currentUserCurrentHost 
        /// and with notes for the other four parameters.</returns>
        internal static PSObject GetDollarProfile(
                                                  string allUsersAllHosts, 
                                                  string allUsersCurrentHost, 
                                                  string currentUserAllHosts, 
                                                  string currentUserCurrentHost)
        {
            PSObject returnValue = new PSObject(currentUserCurrentHost);
            returnValue.Properties.Add(new PSNoteProperty("AllUsersAllHosts", allUsersAllHosts));
            returnValue.Properties.Add(new PSNoteProperty("AllUsersCurrentHost", allUsersCurrentHost));
            returnValue.Properties.Add(new PSNoteProperty("CurrentUserAllHosts", currentUserAllHosts));
            returnValue.Properties.Add(new PSNoteProperty("CurrentUserCurrentHost", currentUserCurrentHost));
            return returnValue;
        }

        /// <summary>
        /// Gets an array of commands that can be run sequentially to set 
        /// $profile and run the profile commands.
        /// </summary>
        /// <param name="shellId">Identifies the host or shell used in 
        /// the profile file names.</param>
        /// <returns></returns>
        public static PSCommand[] GetProfileCommands(string shellId)
        {
            return HostUtilities.GetProfileCommands(shellId, false);
        }

        /// <summary>
        /// Gets an array of commands that can be run sequentially to set 
        /// $profile and run the profile commands.
        /// </summary>
        /// <param name="shellId">Identifies the host or shell used in 
        /// the profile file names.</param>
        /// <param name="useTestProfile">Used from test not to overwrite 
        /// the profile file names from development boxes.</param>
        /// <returns>The profile commands.</returns>
        internal static PSCommand[] GetProfileCommands(string shellId, bool useTestProfile)
        {
            List<PSCommand> commands = new List<PSCommand>();
            string allUsersAllHosts = HostUtilities.GetFullProfileFileName(null, false, useTestProfile);
            string allUsersCurrentHost = HostUtilities.GetFullProfileFileName(shellId, false, useTestProfile);
            string currentUserAllHosts = HostUtilities.GetFullProfileFileName(null, true, useTestProfile);
            string currentUserCurrentHost = HostUtilities.GetFullProfileFileName(shellId, true, useTestProfile);
            PSObject dollarProfile = HostUtilities.GetDollarProfile(allUsersAllHosts, allUsersCurrentHost, currentUserAllHosts, currentUserCurrentHost);
            PSCommand command = new PSCommand();
            command.AddCommand("set-variable");
            command.AddParameter("Name", "profile");
            command.AddParameter("Value", dollarProfile);
            command.AddParameter("Option", ScopedItemOptions.None);
            commands.Add(command);

            string[] profilePaths = new string[] { allUsersAllHosts, allUsersCurrentHost, currentUserAllHosts, currentUserCurrentHost };
            foreach (string profilePath in profilePaths)
            {
                if (!System.IO.File.Exists(profilePath))
                {
                    continue;
                }

                command = new PSCommand();
                command.AddCommand(profilePath, false);
                commands.Add(command);
            }

            return commands.ToArray();
        }

        /// <summary>
        /// Used to get all profile file names for the current or all hosts and for the current or all users.
        /// </summary>
        /// <param name="shellId">null for all hosts, not null for the specified host</param>
        /// <param name="forCurrentUser">false for all users, true for the current user.</param>
        /// <returns>The profile file name matching the parameters.</returns>
        internal static string GetFullProfileFileName(string shellId, bool forCurrentUser)
        {
            return HostUtilities.GetFullProfileFileName(shellId, forCurrentUser, false);
        }

        /// <summary>
        /// Used to get all profile file names for the current or all hosts and for the current or all users.
        /// </summary>
        /// <param name="shellId">null for all hosts, not null for the specified host</param>
        /// <param name="forCurrentUser">false for all users, true for the current user.</param>
        /// <param name="useTestProfile">used from test not to overwrite the profile file names from development boxes</param>
        /// <returns>The profile file name matching the parameters.</returns>
        internal static string GetFullProfileFileName(string shellId, bool forCurrentUser, bool useTestProfile)
        {
            string basePath = null;

            if (forCurrentUser)
            {
                basePath = Environment.GetFolderPath(Environment.SpecialFolder.Personal);
                basePath = System.IO.Path.Combine(basePath, "WindowsPowerShell");
            }
            else
            {
                basePath = GetAllUsersFolderPath(shellId);
                if (string.IsNullOrEmpty(basePath))
                {
                    return string.Empty;
                }
            }

            string profileName = useTestProfile ? "profile_test.ps1" : "profile.ps1";

            if (!string.IsNullOrEmpty(shellId))
            {
                profileName = shellId + "_" + profileName;
            }

            string fullPath = basePath = System.IO.Path.Combine(basePath, profileName);

            return fullPath;
        }

        // <summary>
        /// Gets the registry key used to install Windows PowerShell.
        /// </summary>
        /// <param name="shellId">The parameter is not used.</param>
        /// <returns>The registry key.</returns>
        internal static string GetApplicationBase(string shellId)
        {
            string engineKeyPath = PowerShellRootKeyPath + "\\" +
                RegistryVersionKey + "\\" + PowerShellEngineKey;

            using (RegistryKey engineKey = Registry.LocalMachine.OpenSubKey(engineKeyPath))
            {
                if (engineKey != null)
                {
                    return engineKey.GetValue(PowerShellEngineApplicationBase) as string;
                }
            }

            // The default keys aren't installed, so try and use the entry assembly to
            // get the application base. This works for managed apps like minishells...
            Assembly assem = Assembly.GetEntryAssembly();
            if (assem != null)
            {
                // For minishells, we just return the executable path. 
                return Path.GetDirectoryName(assem.Location);
            }

            // For unmanaged host apps, look for the SMA dll, if it's not GAC'ed then
            // use it's location as the application base...
            assem = Assembly.GetAssembly(typeof(System.Management.Automation.PSObject));
            if (assem != null)
            {
                // For for other hosts. 
                return Path.GetDirectoryName(assem.Location);
            }

            // otherwise, just give up...
            return string.Empty;
        }

        /// <summary>
        /// Used internally in GetFullProfileFileName to get the base path for all users profiles.
        /// </summary>
        /// <param name="shellId">The shellId to use.</param>
        /// <returns>the base path for all users profiles.</returns>
        private static string GetAllUsersFolderPath(string shellId)
        {
            string folderPath = string.Empty;
            try
            {
                folderPath = GetApplicationBase(shellId);
            }
            catch (System.Security.SecurityException)
            {
            }

            return folderPath;
        }
    }
}
