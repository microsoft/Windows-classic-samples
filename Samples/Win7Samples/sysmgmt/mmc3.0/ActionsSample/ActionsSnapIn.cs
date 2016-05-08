//=======================================================================================;
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
//=======================================================================================;
//
//  Purpose:
//  This Sample is designed to show the various places 'custom' actions can be added to 
//  the snap-in: ScopeNodes, Views, ViewModes, ResultNode Selections. 
//
//=======================================================================================;
//
//  Description:
//  Loads a list of users into a MmcListView.  Adds actions as follows: ScopeNode has 
//  actions Add Child Node and Add to Root Node. MmcListView has actions Refresh and 
//  SortByName. The mode of the MmcListView has actions View as Large Icons, View As 
//  List, View As Small Icons and View as Report. The SelectionData of result nodes 
//  has the action ShowSelection.
//
//=======================================================================================;

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
    /// ActionsSnapIn - Snap-in for this example to show off Actions
    /// </summary>
    [SnapInSettings("{3DCE9B26-01F8-42f1-A5EA-2CBD5A48EB68}",
       DisplayName = "- Actions Sample", 
       Description = "Shows MmcListView with Actions at scope, list, view mode and selection level.")]
	public class ActionsSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor for snapin
        /// </summary>
        public ActionsSnapIn()
		{
            // Create the root node
            this.RootNode = new ActionScopeNode();
            this.RootNode.DisplayName = "Actions Sample";
            this.RootNode.ImageIndex = 0;

            // Create a message view for the root node.
            MmcListViewDescription lvd = new MmcListViewDescription();
            lvd.DisplayName = "Users (MmcListView)";
            lvd.ViewType = typeof(ActionListView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes | MmcListViewOptions.AllowUserInitiatedModeChanges;

            // Attach the view to the root node
            this.RootNode.ViewDescriptions.Add(lvd);
            this.RootNode.ViewDescriptions.DefaultIndex = 0;
        }

        /// <summary>
        /// ScopeNode - Basic name and icon for the ScopeTree
        /// ActionScopeNode - Version of ScopeNode that has built in actions
        /// </summary>
        public class ActionScopeNode : ScopeNode
        {
            /// <summary>
            /// Constructor for node
            /// </summary>
            public ActionScopeNode()
            {
                // add actions
                this.ActionsPaneItems.Add(new Action("Add Child", "Adds a new scope node under this node", 0, "AddChild"));
                this.ActionsPaneItems.Add(new Action("Add to Root", "Adds a new scope node to the root", 0, "AddToRoot"));
            }

            /// <summary>
            /// Handle node actions
            /// </summary>
            /// <param name="action">action that was triggered</param>
            /// <param name="status">asynchronous status for updating the console</param>
            protected override void OnAction(Action action, AsyncStatus status)
            {
                switch ((string)action.Tag)
                {
                    case "AddChild":
                        {
                            ActionScopeNode actionScopeNode = new ActionScopeNode();
                            actionScopeNode.DisplayName = "Added " + System.DateTime.Now.ToLongTimeString();
                            actionScopeNode.ImageIndex = 0;
                            this.Children.Add(actionScopeNode);
                            break;
                        }

                    case "AddToRoot":
                        {
                            ActionScopeNode actionScopeNode = new ActionScopeNode();
                            actionScopeNode.DisplayName = "Added " + System.DateTime.Now.ToLongTimeString();
                            actionScopeNode.ImageIndex = 0;
                            ((ActionsSnapIn)this.SnapIn).RootNode.Children.Add(actionScopeNode);
                            break;
                        }
                }
            }        
        }

	} // class
} //namespace
