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
//  Show wizard at add snap-in to console time.
// 
//=======================================================================================;
//
//  Description:
//  Uses the SnapIn ShowInitializationWizard method to show a dialog that allows the 
//  user to enter a name which is then used in setting the root node DisplayName.
//
//=======================================================================================;

using System;
using System.ComponentModel;
using System.Configuration;
using System.Security.Permissions;
using System.Text;
using System.Windows.Forms;
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
	/// SnapInSettings attribute - Used to set the registration information for the SnapIn.
    /// SnapIn class - Provides the main entry point for the creation of a snap-in. 
    /// InitializationWizardSnapIn class - Shows a dialog during the add snapin to console process.
    /// </summary>
    [SnapInSettings("{2DACA4C5-ADB7-4f98-ADB7-C965D81EC9B3}",
       DisplayName = "- Initilization Wizard SnapIn",
       Description = "Sample - Shows Wizard during Add to Console")]
    public class InitializationWizardSnapIn : SnapIn
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public InitializationWizardSnapIn()
        {
            this.RootNode = new ScopeNode();
            this.RootNode.DisplayName = "Unknown";

            this.IsModified = true;         // tells mmc to save custom data
        }

        /// <summary>
        /// Shows the Initialization Wizard when the snapin is added to console 
        /// Returning 'False' will cause MMC to cancel the loading of the snap-in 
        /// </summary>
        /// <returns>true to continue loading the snap-in. false cancels snap-in loading.</returns>
        protected override bool OnShowInitializationWizard()
        {
            // show modal dialog to get snapin name
            InitializationWizard initializationWizard = new InitializationWizard();
            bool result = (initializationWizard.ShowDialog() == DialogResult.OK);

            // got name? 
            if (result)
            {
                this.RootNode.DisplayName = initializationWizard.SelectedSnapInName;
            }

            return result;
        }

        /// <summary>
        /// Load in any saved data
        /// </summary>
        /// <param name="status">asynchronous status for updating the console</param>
        /// <param name="persistenceData">binary data stored in the console file</param>
        protected override void OnLoadCustomData(AsyncStatus status, byte[] persistenceData)
        {
            // saved name? then set snap-in to the name
            if (string.IsNullOrEmpty(Encoding.Unicode.GetString(persistenceData)))
            {
                this.RootNode.DisplayName = "Unknown";
            }
            else
            {
                this.RootNode.DisplayName = Encoding.Unicode.GetString(persistenceData);
            }
        }

        /// <summary>
        /// If snapIn 'IsModified', then save data
        /// </summary>
        /// <param name="status">status for updating the console</param>
        /// <returns>binary data to be stored in the console file</returns>
        protected override byte[] OnSaveCustomData(SyncStatus status)
        {
            return Encoding.Unicode.GetBytes(this.RootNode.DisplayName);
        }

    } //class
} // namespace
