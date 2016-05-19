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
//  Shows how to create an ExtensionNamespace for the snap-in created in 
//  the 'Extensible SnapIn Sample'.
//
//=======================================================================================
//
//  Description:
//  Uses the ExtendsNodeType attribute and the Guid of the ‘Extensible SnapIn Sample’ root node 
//  to create a NameSpaceExtension. Uses the shared data features to get some data from 
//  the primary and use it in the Extension node’s DisplayName.
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
    /// SnapInAttribute - Used to define the registration information for a snap-in. 
    /// ExtendsNodeType attribute - Extends the type of node referenced with the guid.
    /// (see the ExtensibleSnapIn Sample)
    /// NamespaceExtension - Basic snapin that extends other snapins
    /// </summary>
    [SnapInSettings("0F3F3735-573D-9804-99E4-AB2A69BA5FD4", 
        DisplayName = "- Extension to Extensible SnapIn", 
        Description = "Sample showing how to extend a node.")]
    [ExtendsNodeType("53F21A59-11E5-4ee3-8ED2-3D4337FC854A")]
    public class ExtensionToSnapIn : NamespaceExtension
    {
        /// <summary>
        /// Reference id of sharedDataItem this extension uses
        /// </summary>
        public const string Data1ClipboardFormatId = "Data1";

        /// <summary>
        /// Constructor
        /// </summary>
        public ExtensionToSnapIn ()
        {
            // In order to receive published data from the primary snap-in, MMC needs to know which
            // data items the extension is interested in. 
            PrimaryNode.SharedData.Add(new SharedDataItem(Data1ClipboardFormatId));
        }

        /// <summary>
        /// Intialize the extension to add new nodes attached to the Primary, etc.
        /// </summary>
        protected override void OnInitialize()
        {
            string sampleData1FromPrimary = Encoding.Unicode.GetString(PrimaryNode.SharedData.GetItem(Data1ClipboardFormatId).GetData());

            ScopeNode scopeNode = new ScopeNode();
            scopeNode.DisplayName = "Extension - Shared: (" + sampleData1FromPrimary + ")";
            PrimaryNode.Children.Add(scopeNode);
        }
    }

} // namespace
