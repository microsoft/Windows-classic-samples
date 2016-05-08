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
//  Shows how to do dragging and dropping within the MmcListView 
//
//=======================================================================================
//
//  Description:
//  Loads a list of Users into the MmcListView and enables the standard verbs Copy and 
//  Paste. Dropped on nodes get their DisplayName changed to show they were dropped on.
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
    /// DragDropSnapIn class - Shows mmc list view with dragdrop behavior
    /// </summary>
    [SnapInSettings("{A364BFB2-B140-48ba-938B-6E3F891FBCD1}",
       DisplayName = "- Drag Drop Sample", 
        Description = "Shows MmcListView with Drag Drop.")]
	public class DragDropSnapIn : SnapIn
	{
        /// <summary>
        /// Constructot
        /// </summary>
        public DragDropSnapIn()
		{
            // Create the root node
            this.RootNode = new ScopeNode();
            this.RootNode.DisplayName = "DragDrop Sample";
            this.RootNode.ImageIndex = 0;

            // Create a message view for the root node.
            MmcListViewDescription lvd = new MmcListViewDescription();
            lvd.DisplayName = "Users (MmcListView)";
            lvd.ViewType = typeof(DragDropListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;

            // Attach the view to the root node
            this.RootNode.ViewDescriptions.Add(lvd);
            this.RootNode.ViewDescriptions.DefaultIndex = 0;
        }
	}

} //namespace
