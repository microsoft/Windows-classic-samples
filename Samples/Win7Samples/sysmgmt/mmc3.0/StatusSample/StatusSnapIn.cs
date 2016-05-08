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
//  Shows how to use the status and update the progress bar.
//
//=======================================================================================
//
//  Description:
//  Defines a new type of ScopeNode that adds 10 children to itself OnExpand one every 
//  couple of seconds. While adding, it updates the progress bar in Mmc and sets the 
//  status bar message.
//
//=======================================================================================

using System;
using System.Threading;
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
    /// StatusSnapIn class - Adds nodes while updating console progress and status bar
    /// </summary>
    [SnapInSettings("{1BD9BDB9-BF00-4d82-A12D-6CEC86C98072}", 
        DisplayName = "- Status Sample",
        Description = "Shows how to use Status to update the Progress window.")]
    public class StatusSnapIn : SnapIn
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public StatusSnapIn()
        {
            this.RootNode = new StatusNode();
        }
    }

    /// <summary>
    /// Node type to update progress bar as children are added upon expand
    /// </summary>
    public class StatusNode : ScopeNode
    { 
        /// <summary>
        /// Constructor
        /// </summary>
        public StatusNode()
        {
            this.DisplayName = "Status and Progress Sample";
        }

        /// <summary>
        /// Node plus sign clicked to expand the node. Load in children.
        /// </summary>
        /// <param name="status">asynchronous status for updating the console</param>
        protected override void OnExpand(AsyncStatus status)
        {
            int foundChildren = 10;

            // report progress
            status.ReportProgress(0, foundChildren, "Loading Sample children...");

            // find results
            for (int child = 1; child < foundChildren; child++)
            {
                // The Thread.Sleep statement below is for demo purposes only. When doing your 
                // development you should not block your snap-in thread to service an 
                // async request like OnExpand unless you only use the scope tree and 
                // list view and never show any WinForms views or WinForms property pages.  
                // The reason is that your WinForms views and property pages will be blocked, 
                // and in certain cases that can also block MMC.  

                // Sleep for a second.
                Thread.Sleep(1000);

                // add the child 
                ScopeNode childNode = new StatusNode();
                childNode.DisplayName = "Added " + System.DateTime.Now.ToLongTimeString();
                this.Children.Add(childNode);

                // update the progress
                status.ReportProgress(child, foundChildren, "Loading Sample children...");
            }

            // update progress
            status.Complete("Loading Sample complete.", true);
        }
    }

} // namespace