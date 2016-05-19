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
//  Show MmcListView with multi-selection.
//
//=======================================================================================
//
//  Description:
//  Uses the MmcListView to display list of users. Adds ShowSelection actions.
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
    /// SelectionListviewSnapIn class - Shows MmcListView with actions on selections
    /// </summary>
    [SnapInSettings("{CE255EF6-9E3D-42c8-B725-95CCC761B9D9}",
       DisplayName = "- Selection (MmcListView) Sample", 
        Description = "Shows MmcListView with multi-selection.")]
	public class SelectionListviewSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
		public SelectionListviewSnapIn()
		{
            // Create the root node
            this.RootNode = new ScopeNode();
            this.RootNode.DisplayName = "Selection (MmcListView) Sample";

            // Create a message view for the root node.
            MmcListViewDescription lvd = new MmcListViewDescription();
            lvd.DisplayName = "Users (MmcListView)";
            lvd.ViewType = typeof(SelectionListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;

            // Attach the view to the root node
            this.RootNode.ViewDescriptions.Add(lvd);
            this.RootNode.ViewDescriptions.DefaultIndex = 0;

            this.RootNode.Children.Add(new ScopeNode());
            this.RootNode.Children.Add(new ScopeNode());
            this.RootNode.Children.Add(new ScopeNode());
            this.RootNode.Children.Add(new ScopeNode());
        }
	}


} //namespace
