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
//  Show standard verbs like cut, copy, paste, rename
//
//=======================================================================================
//
//  Description:
//  Uses a MmcListView to hold a list of Users. Enables and handles the standard verbs 
//  Delete and Refresh.
//
//=======================================================================================

using System;
using System.ComponentModel;
using System.Security.Permissions;
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
    /// StandardVerbsSnapIn class - List with some standards verbs enabled
    /// </summary>
    [SnapInSettings("{EA3C0147-9238-408e-B99C-2A96A072CD96}",
       DisplayName = "- Standard Verbs Sample", 
        Description = "Shows MmcListView with Standard Verbs.")]
	public class StandardVerbsSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
        public StandardVerbsSnapIn()
		{
            // Create the root node
            this.RootNode = new ScopeNode();
            this.RootNode.DisplayName = "StandardVerbs Samples";

            // Create a message view for the root node.
            MmcListViewDescription lvd = new MmcListViewDescription();
            lvd.DisplayName = "Users (MmcListView)";
            lvd.ViewType = typeof(StandardVerbsListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;

            // Attach the view to the root node
            this.RootNode.ViewDescriptions.Add(lvd);
            this.RootNode.ViewDescriptions.DefaultIndex = 0;
        }
	}

} //namespace
