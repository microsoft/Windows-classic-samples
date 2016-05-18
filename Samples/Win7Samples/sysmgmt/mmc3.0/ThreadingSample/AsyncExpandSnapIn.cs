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
//  Shows how use a separate thread to expand nodes and update the status and 
//  progress bar.
//
//=======================================================================================
//
//  Description:
//  Defines a new type of ScopeNode that adds 10 children to itself.  It does this by 
//  firing off a new Thread for the adding process. This adding routine then uses 
//  SnapIn.Invoke to have a delegate actually add the node.
//
//=======================================================================================

using System;
using System.Configuration.Install;
using System.ComponentModel;
using System.Security.Permissions;
using System.Threading;
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
    /// AsyncExpandSnapIn class - snapin with nodes that expand on separate threads.
    /// </summary>
    [SnapInSettings("{BC2A078C-27DF-405b-BEF0-A5A87B83A557}",
        DisplayName = "- Threading Sample",
        Description = "Shows asynchronous expanding with progress.")]
    public class AsyncExpandSnapIn : SnapIn
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public AsyncExpandSnapIn()
        {
            this.RootNode = new AsyncExpandNode();
            this.RootNode.DisplayName = "Threading Sample - Asynchronous expansion on separate thread";
        }
    }

    /// <summary>
    /// ScopeNode class - basic icon and name for item in scope pane
    /// AsyncExpandNode class - node which handles its expansion on a separate thread
    /// </summary>
    public class AsyncExpandNode : ScopeNode
    {
        private AsyncStatus expandStatus = null;

        /// <summary>
        /// Constructor
        /// </summary>
        public AsyncExpandNode()
        {
        }

        /// <summary>
        /// Launch new thread to handle expand tasks
        /// </summary>
        /// <param name="status">asynchronous status for updating the console</param>
        protected override void OnExpand(AsyncStatus status)
        {
            // hang onto status
            expandStatus = status;

            // mark as 
            expandStatus.EnableManualCompletion();

            Thread thread = new Thread(new ThreadStart(Expand));
            thread.Start();
        }

        /// <summary>
        /// Add child nodes during expansion 
        /// </summary>
        private void Expand()
        {
            int foundChildren = 10;

            // report progress
            expandStatus.ReportProgress(0, foundChildren, "Loading Sample children...");

            // find results
            for (int child = 1; child < foundChildren; child++)
            {
                // The Thread.Sleep statement below is for demo purposes only. When doing your 
                // development you should not block your snap-in thread to service an 
                // async request like OnExpand unless you only use the scope tree and 
                // list view and never show any WinForms views or WinForms property pages.  
                // The reason is that your WinForms views and property pages will be blocked, 
                // and in certain cases that can also block MMC.  

                // sleep for a second
                Thread.Sleep(1000);

                // New scope nodes must be created on the snapin's thread
                DelegateWithNode delegateWithNode = new DelegateWithNode(AddChildNode);
                this.SnapIn.BeginInvoke(delegateWithNode, new object[] {this});

                // update the progress
                expandStatus.ReportProgress(child, foundChildren, "Loading Sample children...");
            }

            // update progress
            expandStatus.Complete("Loading Sample complete.", true);
        }

        private delegate void DelegateWithNode(AsyncExpandNode parentNode);

        /// <summary>
        /// Adding node on the snap-in thread although the Expand is in its own thread
        /// </summary>
        /// <param name="parentNode">node to add under</param>
        private void AddChildNode(AsyncExpandNode parentNode)
        {
            // add the child 
            ScopeNode childNode = new AsyncExpandNode();
            childNode.DisplayName = "Added " + System.DateTime.Now.ToLongTimeString();
            this.Children.Add(childNode);
        }

    }
} // namespace