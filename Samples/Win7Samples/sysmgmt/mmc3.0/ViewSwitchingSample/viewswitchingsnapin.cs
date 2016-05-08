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
//  Show a scope node with views that know how to display each other.
//
//=======================================================================================
//
//  Description:
//  Uses the ViewDefinitions and View.SelectScopeNode to switch between views.
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
    /// ScopeViewSnapIn class - Defines a node with views that can show each other in the view pane.
    /// </summary>
    [SnapInSettings("{1DFE2262-085B-455a-8610-B6255B8D7BE7}",
       DisplayName = "- Views Switching Sample", 
        Description = "Shows how Views can switch between each other.")]
	public class ViewSwitchingSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
		public ViewSwitchingSnapIn()
		{
            // Create the root node
            ViewSwitchingScopeNode viewSwitchingScopeNode = new ViewSwitchingScopeNode();
            this.RootNode = viewSwitchingScopeNode;

            // attach a view and set as the default 
            viewSwitchingScopeNode.AddViewDescription("Normal Message View", typeof(NormalMessageView));
            viewSwitchingScopeNode.ViewDescriptions.DefaultIndex = 0;
        }
    }

    /// <summary>
    /// ScopeNode - Item in the Scope pane
    /// ViewSwitchingNode - Switches between views
    /// </summary>
    public class ViewSwitchingScopeNode : ScopeNode
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public ViewSwitchingScopeNode()
        {
            // set the node name
            this.DisplayName = "View Switching Sample";
        }

        /// <summary>
        /// Adds a view description and set it as the default
        /// </summary>
        /// <param name="displayName"></param>
        /// <param name="viewType"></param>
        public void AddViewDescription(string displayName, Type viewType)
        {
            // add the view description (display name is required)
            MessageViewDescription mvd = new MessageViewDescription();
            mvd.DisplayName = displayName;
            mvd.ViewType = viewType;
            this.ViewDescriptions.Add(mvd);
        }

        /// <summary>
        /// Removes a message view description 
        /// </summary>
        /// <param name="removeIndex"></param>
        public void RemoveViewDescriptionAt(int removeIndex)
        {
            // remove the description
            this.ViewDescriptions.RemoveAt(removeIndex);
        }

        /// <summary>
        /// Swaps a new view for the existing. 
        /// (NOTE: expects that only a single view is already defined)
        /// </summary>
        /// <param name="currentView"></param>
        /// <param name="newViewDisplayName"></param>
        /// <param name="newViewType"></param>
        public void SwapView(View currentView, string newViewDisplayName, Type newViewType)
        {
            // remove the existing view description 
            this.RemoveViewDescriptionAt(0);

            // add the new view description
            this.AddViewDescription(newViewDisplayName, newViewType);

            // set the new view as the default
            this.ViewDescriptions.DefaultIndex = 0;

            // trigger MMC to show the new default view 
            currentView.SelectScopeNode(this);
        }
    }

} //namespace
