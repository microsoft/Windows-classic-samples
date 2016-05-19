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
//  Shows how to create an ExtensionPropertySheet for the Computer Management snap-in .
//
//=======================================================================================
//
//  Description:
//  Uses the ExtendsNodeType attribute and the Guid of the ‘Computer Management’ 
//  snap-in root node to create a ExtensionPropertySheet. Uses the shared data 
//  features to get some data from the primary and use it in the Extension sheet. 
//
//=======================================================================================

using System;
using System.Text;
using System.Configuration.Install;
using System.ComponentModel;
using System.Security.Permissions;
using System.Windows.Forms;
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
    /// Property sheet extension for the root node of Computer Management snap-in.
    /// </summary>
    [SnapInSettings("{1A64DD4E-1C06-42c2-A0C7-45233E7E67DA}",
           DisplayName = "- Extension to Property Sheet Sample",
            Description = "Adds a Property Page to Computer Management")]
    [ExtendsNodeType("{476E6446-AAFF-11D0-B944-00C04FD8D5B0}")]
    public class ExtensionToPropertySheet : PropertySheetExtension
    {
        private SharedDataItem sharedDataItem;

        /// <summary>
        /// Extension class to extend the Computer Management snap-in property sheet
        /// </summary>
        public ExtensionToPropertySheet()
        {
            // In order to receive published data from the primary snap-in, MMC needs to know which
            // data items the extension is interested in. 
            sharedDataItem = new SharedDataItem(@"MMC_SNAPIN_MACHINE_NAME");
            this.SharedData.Add(sharedDataItem);
        }

        /// <summary>
        /// Initialization notification.
        /// </summary>
        protected override void OnInitialize()
        {
        }

        /// <summary>
        /// Virtual method that is called to get the extension pages.  
        /// </summary>
        /// <param name="propertyPageCollection">Page collection.</param>
        protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
        {
            // add extension page.
            MachinePropertyPage machinePropertyPage = new MachinePropertyPage(sharedDataItem);
            propertyPageCollection.Add(machinePropertyPage);
        }

    } //class
} // namespace















