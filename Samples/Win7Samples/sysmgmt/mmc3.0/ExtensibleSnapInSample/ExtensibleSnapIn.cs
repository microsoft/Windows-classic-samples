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
//  Shows how to create a Primary snap-in in that can be extended.
//
//=======================================================================================
//
//  Description:
//  Uses PublishesNodeType and NodeType attributes on the snap-in and its root node type 
//  respectively to create a snap-in that can be extended.  (see also ‘Extension Sample’)
//
//=======================================================================================

using System.Text;
using System.ComponentModel;
using System.Configuration;
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
    /// SnapInSettings attribute - Used to define the registration information for a snap-in. 
    /// PublishesNodeType attrbiute - Used to define the types of nodes that a snap-in allows to 
    /// be extended. 
    /// SnapIn class - Provides the main entry point for the creation of a snap-in.
    /// ExtensibleSnapIn - A snapin that can share data with its extensions
    /// </summary>
    [SnapInSettings("2F444B72-9A4C-46e9-9407-335A24AFA825", 
        DisplayName = "- Extensible SnapIn", 
        Description = "Sample showing an Extensible SnapIn. See also ExtensionToSnapIn sample")]
    [PublishesNodeType("53F21A59-11E5-4ee3-8ED2-3D4337FC854A", 
        Description = "- Extensible Scope Node")]
    public class ExtensibleSnapIn : SnapIn
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public ExtensibleSnapIn()
        {
        }

        /// <summary>
        /// 
        /// </summary>
        protected override void OnInitialize()
        {
            this.RootNode = new ExtensibleNode();
        }
    }

    /// <summary>
    /// NodeType attribute - Attribute that is used at runtime to specify 
    ///     that instances of this class are of this NodeType.
    /// ScopeNode class - The basic unit managed in the Scope Pane.
    /// ExtensibleNode - Node that can share data with its extensions
    /// </summary>
    [NodeType("53F21A59-11E5-4ee3-8ED2-3D4337FC854A", 
        Description = "Extensible Scope Node")]
    public class ExtensibleNode : ScopeNode
    {
        /// <summary>
        /// Clipboard format for shared data item
        /// </summary>
        public const string Data1ClipboardFormatId = "Data1";
        
        /// <summary>
        /// Constructor
        /// </summary>
        public ExtensibleNode()
        {
            this.DisplayName = "Extensible ScopeNode";

            // add data item viewable by extensions
            WritableSharedDataItem writableSharedDataItem = new WritableSharedDataItem(Data1ClipboardFormatId, true);
            // writableSharedDataItem.SetData(Encoding.Unicode.GetBytes("Some Sample Data1"));
            this.SharedData.Add(writableSharedDataItem);
        }

        


        /// <summary>
        /// Handles call back shared data items
        /// </summary>
        /// <param name="sharedDataItem">shared data</param>
        /// <returns>the item data in binary form</returns>
        protected byte[] OnGetSharedData(WritableSharedDataItem sharedDataItem)
        {
            byte[] sharedDataBytes = null;

            if (sharedDataItem.ClipboardFormatId == Data1ClipboardFormatId)
            {
                sharedDataBytes = Encoding.Unicode.GetBytes("Some Sample Data " + System.DateTime.Now.ToShortTimeString());
            }
            return sharedDataBytes;
        }
    }

} // namespace
