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
//  Shows how to extend MMC's standard 'Computer Management' snap-in
// 
//=======================================================================================;
//
//  Description:
//  Uses the ExtendsNodeType attribute and the Guid of the Computer Management 'System Tools' 
//  RootNode to create a NameSpaceExtension.  Then uses the shared data features to get the 
//  MMC_SNAPIN_MACHINE_NAME from the Computer Management snap-in and use it in the 
//  Extension node’s DisplayName as it adds it under 'System Tools'.
//
//=======================================================================================;

using System;
using System.Text;
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
    /// NamespaceExtension class - Provides the main entry point for the creation of a Namespace Extension.
    /// SnapInAttribute - Used to define the registration information for a snap-in or Extension. 
    /// ExtendsNodeTypeAttribute - Specifies which node types it can extend. It is an optional attribute. 
    /// It requires GUID of the Primary node. In this case it is the 'System Tools' node under the
    /// Computer Management snap-in.
    /// </summary>
    [SnapInSettings("{18BB66A7-80A7-4456-9830-0DAA85B3F270}",
        DisplayName = "- Extension to Computer Management", 
        Description = "Sample - Extends the Computer Management snap-in with an 'Extension' node")]
    [ExtendsNodeType("{476E6448-AAFF-11D0-B944-00C04FD8D5B0}")]
    public class ExtensionToComputerManagement : NamespaceExtension
    {
        /// <summary>
        /// Clipboard 'id' of the machine name exposed by the Computer Management snap-in
        /// </summary>
        private const string machineNameClipboardFormatId = "MMC_SNAPIN_MACHINE_NAME";
        
        /// <summary>
        /// Store the extension's node
        /// </summary>
        private ScopeNode extensionRootNode;

        /// <summary>
        /// Constructor. Sets up view to the shared data the extension is interested in
        /// </summary>
        public ExtensionToComputerManagement()
        {
            // In order to receive published data from the primary snap-in, MMC needs to know which
            // data items the extension is interested in. 
            // In this case, the primary snap-in publishes a data item called MMC_SNAPIN_MACHINE_NAME.
            PrimaryNode.SharedData.Add(new SharedDataItem(machineNameClipboardFormatId));
        }

        /// <summary>
        /// Use shared data
        /// </summary>
        protected override void OnInitialize()
        {
            // create extension node
            extensionRootNode = new ScopeNode();
            PrimaryNode.Children.Add(extensionRootNode);

            // get shared data
            SetDisplayName(extensionRootNode, PrimaryNode.SharedData.GetItem(machineNameClipboardFormatId));
        }

        /// <summary>
        /// The DisplayName of the extension node will be the computer name 
        /// published by the primary.  Note: this defaults to an empty string for localhost.
        /// </summary>
        void SetDisplayName(ScopeNode scopeNode, SharedDataItem sharedDataItem)
        {
            // get buffer containing the machine name
            string machineName = Encoding.Unicode.GetString(sharedDataItem.GetData());
            
            // find first null terminated string in buffer. 
            if (machineName.IndexOf('\0') <= 0)
            {
                // either not found in buffer or first entry in buffer
                machineName = String.Empty;
            }
            else
            {
                machineName = machineName.Substring(0, machineName.IndexOf('\0'));
            }

            // if empty then localhost
            if (machineName == string.Empty)
            {
                scopeNode.DisplayName = "Sample Extension to (localhost)";
            }
            else
            {
                scopeNode.DisplayName = "Sample Extension to (" + machineName + ")";
            }
        }

        /// <summary>
        /// Respond to changes in shared information from the primary
        /// </summary>
        /// <param name="sharedDataItem">shared data</param>
        protected override void OnSharedDataChanged(SharedDataItem sharedDataItem)
        {
            // if the machine name has changed, update the extension node name
            if (sharedDataItem.ClipboardFormatId == machineNameClipboardFormatId)
            {
                SetDisplayName(extensionRootNode, sharedDataItem);
            }
        }

    } // class
} // namespace

