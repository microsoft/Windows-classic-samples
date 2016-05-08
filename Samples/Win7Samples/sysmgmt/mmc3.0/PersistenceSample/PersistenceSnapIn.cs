//=======================================================================================
//
//  This source code is only intended as a supplement to existing Microsoft documentation. 
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//=======================================================================================
//
//  Purpose:
//  Shows how to have MMC load and store data for a snap-in
//
//=======================================================================================
//
//  Description:
//  Uses the snapin LoadCustomData and SaveCustomData methods to store and retrieve 
//  the root node’s DisplayName regardless of how it’s been renamed since. 
//
//=======================================================================================

using System;
using System.ComponentModel;
using System.Configuration;
using System.Security.Permissions;
using System.Text;
using Microsoft.ManagementConsole;

[assembly: PermissionSetAttribute(SecurityAction.RequestMinimum, Unrestricted = true)]

namespace Microsoft.ManagementConsole.Samples
{
    /// <summary>
    /// RunInstaller attribute - Allows the .Net framework InstallUtil.exe to install the assembly.
    /// SnapInInstaller class - Installs snap-in for MMC.
    /// </summary>
    [RunInstaller(true)]
    public class InstallUtilSupport : SnapInInstaller
    {
    }

    /// <summary>
    /// SnapInSettings attribute - Used to set the registration information for the snap-in.
    /// SnapIn class - Provides the main entry point for the creation of a snap-in. 
    /// PersistenceSnapIn class - Uses MMC to load and save data 
    /// </summary>
    [SnapInSettings("{D97B71CA-5F46-4584-A89B-D24FF6D6190B}",
       DisplayName = "- Persistence SnapIn",
       Description = "Sample - Renames marks as dirty and saves changes")]
    public class PersistenceSnapIn : SnapIn
    {
        private const string defaultDisplayName = "Rename Me and I Save Changes";

        /// <summary>
        /// Constructor
        /// </summary>
        public PersistenceSnapIn()
        {
            this.RootNode = new PersistentScopeNode();
        }
        
        /// <summary>
        /// Snap-in has data? then load 
        /// </summary>
        /// <param name="status">asynchronous status for updating the console</param>
        /// <param name="persistenceData">binary data stored in console file</param>
        protected override void OnLoadCustomData(AsyncStatus status, byte[] persistenceData)
        {
            // saved name? then set snap-in to the name
            if (string.IsNullOrEmpty(Encoding.Unicode.GetString(persistenceData)))
            {
                this.RootNode.DisplayName = defaultDisplayName;
            }
            else
            {
                this.RootNode.DisplayName = Encoding.Unicode.GetString(persistenceData);
            }
        }

        /// <summary>
        /// if snapin 'ismodified', then save data
        /// </summary>
        /// <param name="status">status for updating the console</param>
        /// <returns>true for success</returns>
        protected override byte[] OnSaveCustomData(SyncStatus status)
        {
            return Encoding.Unicode.GetBytes(this.RootNode.DisplayName);
        }

        /// <summary>
        /// ScopeNode class - Basic icon and name for an item in the scope pane
        /// PersistentScopeNode class - Node that when renamed marks its snapin as 
        /// modified and needing saving 
        /// </summary>
        public class PersistentScopeNode : ScopeNode
        {
            /// <summary>
            /// Constructor
            /// </summary>
            public PersistentScopeNode()
            { 
                this.DisplayName = defaultDisplayName;
                this.EnabledStandardVerbs = StandardVerbs.Rename;
            }

            /// <summary>
            /// Handles Rename. Marks snapin as modified so that it saves changes
            /// </summary>
            /// <param name="newText">text the displayname is changing to</param>
            /// <param name="status">status for updating the console</param>
            protected override void  OnRename(string newText, SyncStatus status)
            {
                this.DisplayName = newText;
                this.SnapIn.IsModified = true;
            }
        }

    } //class
} // namespace
