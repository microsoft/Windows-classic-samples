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
//  Shows how to add to the help system by pointing at a .chm file.
// 
//=======================================================================================;
//
//  Description:
//  Uses the SnapInHelp attribute to cause an external .chm help file to be added to 
//  MMC's help system. Also fills the HelpTopic property on the root node to jump 
//  right to a specific help page url (in this case the ScopeNode definition help page).
//
//  SPECIAL NOTE: You should create your Help file with the Binary TOC option set to Yes 
//  otherwise your help file Table of Contents will not show up in MMC.
//
//=======================================================================================;

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
    /// SnapInHelpTopic - Adds the referenced help file to MMC's help system. 
    /// SnapInLinkedHelpTopic - Adds any Additional help file to MMC's help system. 
    /// SnapIn class - Provides the main entry point for the creation of a snap-in. 
    /// HelpSnapIn - Provides a specific help topic for its rootnode
	/// </summary>
    [SnapInSettings("{395A15EB-9A9A-4471-86A3-09778C57B3FD}", 
		 DisplayName = "- Help Sample", 
		 Description = "Show Help Sample SnapIn")]
    [SnapInHelpTopicAttribute(@"HelpSampleReference.chm", ApplicationBaseRelative = true)]
    [SnapInLinkedHelpTopicAttribute(@"HelpSampleReference.chm", ApplicationBaseRelative = true)]
    public class HelpSnapIn : SnapIn
	{
        /// <summary>
        /// Constructot
        /// </summary>
		public HelpSnapIn() 
		{
			// update scope pane with a node in the tree
			this.RootNode = new ScopeNode();
			this.RootNode.DisplayName = "Help Sample - (Help action shows ScopeNode Help)";

            // refer to the 'URL' of the help page you want. 
            // (TIP: Right-click in the right-hand pane for a given help topic to find it's URL.) 
            this.RootNode.HelpTopic = @"HelpSampleReference.chm::/Topic2.htm";
        }

	} // class
 } // namespace
