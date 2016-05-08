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
    /// OnScopeChangeSnapIn class - Defines a node with views that get notifications of scope node change.
    /// </summary>
    [SnapInSettings("{B4312FC1-E3F6-47ea-A0D2-3E07AAC56F88}",
       DisplayName = "- On Scope Node Change Sample", 
        Description = "Shows how Views can get notifications when a scope node changes.")]
	public class OnScopeChangeSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
		public OnScopeChangeSnapIn()
		{
            // Create the root node
            this.RootNode = new NotifyingScopeNode();
        }
    } // snap in class

    /// <summary>
    /// ScopeNode - Item in the Scope pane
    /// NotifyingNode - Notifies views of its status
    /// </summary>
    public class NotifyingScopeNode : ScopeNode
    {
        MessageViewDescription normalMessageViewDescription;
        MessageViewDescription errorMessageViewDescription;
        Action changeToNormalAction;
        Action changeToErrorAction; 

        /// <summary>
        /// Constructor
        /// </summary>
        public NotifyingScopeNode()
        {
            // define views that can be used
            normalMessageViewDescription = new MessageViewDescription();
            normalMessageViewDescription.DisplayName = "Normal Message View";
            normalMessageViewDescription.ViewType = typeof(NormalMessageView);

            errorMessageViewDescription = new MessageViewDescription();
            errorMessageViewDescription.DisplayName = "Simulated Error Message View";
            errorMessageViewDescription.ViewType = typeof(ErrorMessageView);
            
            // define actions that can be used
            changeToNormalAction = new Action("Show Normal", "Switches the view to the normal view", -1, "ShowNormal");
            changeToErrorAction = new Action("Simulate an Error", "Switches the view to an error handling view", -1, "ShowError");


            // set the node name
            this.DisplayName = "On Scope Node Change Sample";

            // set starting view description
            this.ViewDescriptions.Add(normalMessageViewDescription);
            this.ViewDescriptions.DefaultIndex = 0;

            // add starting action to show an 'error'
            this.ActionsPaneItems.Clear();
            this.ActionsPaneItems.Add(changeToErrorAction);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="action"></param>
        /// <param name="status"></param>
        protected override void OnAction(Action action, AsyncStatus status)
        {
            // remove existing view description and action
            this.ViewDescriptions.Clear();
            this.ActionsPaneItems.Clear();

            // new view description and action
            switch ((string)action.Tag)
            {
                case "ShowError":
                    {
                        this.ViewDescriptions.Add(errorMessageViewDescription);
                        this.ActionsPaneItems.Add(changeToNormalAction);
                        break;
                    }

                case "ShowNormal":
                    {
                        this.ViewDescriptions.Add(normalMessageViewDescription);
                        this.ActionsPaneItems.Add(changeToErrorAction);
                        break;
                    }
            }

            // notify any listening views that a change happened
            RaiseOnChange((string)action.Tag);
        } 

        #region Event related methods  ------------------------------------------

        /// <summary>
        /// Declare the delegate type for the changed event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        public delegate void ChangedDelegate(object sender, ChangedEventArgs e);

        /// <summary>
        /// internal store of delegates needed when using event 'lock' syntax 
        /// </summary>
        private ChangedDelegate changedDelegate;

        /// <summary>
        /// Changed event to notify that the scope node has changed
        /// </summary>
        public event ChangedDelegate Changed
        {
            add
            {
                lock (this)
                    changedDelegate += value;
            }
            remove
            {
                lock (this)
                    changedDelegate -= value;
            }
        }

        /// <summary>
        /// Event arguments 
        /// </summary>
        public class ChangedEventArgs : EventArgs
        {
            private string status = "";

            /// <summary>
            /// New status changed to
            /// </summary>
            public string Status
            {
                get
                {
                    return status;
                }
                set
                {
                    status = value;
                }
            }
        }

        /// <summary>
        /// Raises the changed event using the private delegate
        /// </summary>
        private void RaiseOnChange(string status)
        {
            // safely invoke an event
            ChangedDelegate raiseChangedDelegate = changedDelegate;
            if (raiseChangedDelegate != null)
            {
                ChangedEventArgs changedEventArgs = new ChangedEventArgs();
                changedEventArgs.Status = status;

                raiseChangedDelegate(this, changedEventArgs);
            }
        }

        #endregion

    } // scope node class 

} //namespace
