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
//  Shows a message view
//
//=======================================================================================
//
//  Description:
//  Uses the MessageView to show a simple message in the result pane.
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
    /// MessageViewSnapIn class - Defines a message view attached to the rootnode
	/// </summary>
    [SnapInSettings("{6C00C867-B6F2-4428-8625-9AAF6A9AE6F4}", 
		 DisplayName = "- MessageView Sample", 
		 Description = "MessageView Hello World SnapIn")]
	public class MessageViewSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
		public MessageViewSnapIn() 
		{
			// update scope pane with a node in the tree
			this.RootNode = new ScopeNode();
			this.RootNode.DisplayName = "MessageView Sample";

            // set up the update result pane when scope node selected
            MessageViewDescription mvd = new MessageViewDescription();
            mvd.DisplayName = "Hello World";
            mvd.BodyText = "This is a MessageView. You can attach it to any scope node.";
            mvd.IconId = MessageViewIcon.Information;

            // attach the view and set it as the default to show
            this.RootNode.ViewDescriptions.Add(mvd);
            this.RootNode.ViewDescriptions.DefaultIndex = 0;
		}
	} // class
 } // namespace
