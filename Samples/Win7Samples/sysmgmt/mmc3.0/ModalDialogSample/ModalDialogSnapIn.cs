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
//  Shows a ‘connect to’ modal dialog.
//
//=======================================================================================
//
//  Description:
//  Uses the snapin Console.ShowDialog to show a simple dialog during a ‘Connect To’ 
//  custom action.
//
//=======================================================================================

using System;
using System.Configuration.Install;
using System.ComponentModel;
using System.Windows.Forms;
using System.Security.Permissions;
using Microsoft.ManagementConsole;
using Microsoft.ManagementConsole.Advanced;

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
    /// ShowDialogSnapIn - Shows a root node that shows a modal dialog during a SyncAction.
	/// </summary>
    [SnapInSettings("{8FF0C6A1-5162-4010-816F-A0628904CE81}", 
		 DisplayName = "- Modal Dialog Sample", 
		 Description = "Shows modal dialog during SyncAction")]
	public class ModalDialogSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
        public ModalDialogSnapIn() 
		{
			// update scope pane with a node in the tree
			this.RootNode = new ShowDialogNode();
			this.RootNode.DisplayName = "Modal Dialog Sample - Connect To Action...";
        }
    }

    /// <summary>
    /// ScopeNode class - Basic icon and name item for the Scope Pane
    /// Node that shows a modal dialog on actions
    /// </summary>
    public class ShowDialogNode : ScopeNode
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public ShowDialogNode()
        {
            ActionsPaneItems.Add(new SyncAction("Connect To...", "Connect to a different computer.", 0, "ConnectTo"));
            ActionsPaneItems.Add(new ActionSeparator());
            ActionsPaneItems.Add(new SyncAction("Show Common Dialog", "Shows a common dialog (Color Picker).", 0, "CommonDialog"));
            ActionsPaneItems.Add(new SyncAction("Show MessageBox", "Shows a message box.", 0, "MessageBox"));
            ActionsPaneItems.Add(new SyncAction("Show User Defined Form", "Shows a user defined form.", 0, "UserDefinedForm"));
            ActionsPaneItems.Add(new SyncAction("Show UDF with Wait Cursor", "Shows a user defined form if waiting for more than 5 seconds.", 0, "UserDefinedFormWithWaitCursor"));
        }

        /// <summary>
        /// Handle triggered action
        /// </summary>
        /// <param name="action">triggered action</param>
        /// <param name="status">synchronous status to update console</param>
        protected override void  OnSyncAction(SyncAction action, SyncStatus status)
        {
            switch ((string)action.Tag)
            {
                case "ConnectTo":
                    {
                        ConnectDialog connectDialog = new ConnectDialog();
                        connectDialog.ConnectToServerName.Text = String.Empty;

                        if (this.SnapIn.Console.ShowDialog(connectDialog) == DialogResult.OK)
                        {
                            this.DisplayName = "Connected (" + connectDialog.ConnectToServerName.Text + ")";
                        }
                        break;
                    }
                case "CommonDialog":
                    {
                        ColorDialog colorDialog = new ColorDialog();
                        colorDialog.AllowFullOpen = false;

                        if (this.SnapIn.Console.ShowDialog(colorDialog) == DialogResult.OK)
                        {
                            this.DisplayName = "CommonDialog - Selected a Color";
                        }
                        break;
                    }
                case "MessageBox":
                    {
                        MessageBoxParameters messageBoxParameters = new MessageBoxParameters();

                        messageBoxParameters.Caption = "Sample MessageBox...";
                        messageBoxParameters.Buttons = MessageBoxButtons.OKCancel;
                        messageBoxParameters.Text = "Select Ok or Cancel";

                        if (this.SnapIn.Console.ShowDialog(messageBoxParameters) == DialogResult.OK)
                        {
                            this.DisplayName = "MessageBox - Selected Ok";
                        }
                        break;
                    }
                case "UserDefinedForm":
                    {
                        UserDefinedForm userDefinedForm = new UserDefinedForm();

                        if (this.SnapIn.Console.ShowDialog(userDefinedForm) == DialogResult.OK)
                        {
                            this.DisplayName = "User Defined Form - Ok";
                        }
                        break;
                    }
                case "UserDefinedFormWithWaitCursor":
                    {
                        WaitCursor waitCursor = new WaitCursor();
                        waitCursor.Timeout = new System.TimeSpan(0,0,5);

                        UserDefinedFormForWaiting userDefinedFormForWaiting = new UserDefinedFormForWaiting();

                        if (this.SnapIn.Console.ShowDialog(userDefinedFormForWaiting, waitCursor) == DialogResult.OK)
                        {
                            this.DisplayName = "User Defined Form with Wait Cursor - Ok";
                        }
                        break;
                    }
            }
        }
	}

} // namespace
