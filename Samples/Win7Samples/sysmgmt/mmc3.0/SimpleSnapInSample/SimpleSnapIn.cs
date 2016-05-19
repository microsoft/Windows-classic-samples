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
//  Show minimal up-and-running snap-in
//
//=======================================================================================
//
//  Description:
//  Creates the minimum for a Hello World snap-in
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
    /// SimpleSnapIn class - Simplest class available. ex. Hello World  
	/// </summary>
    [SnapInSettings("{CFAA3895-4B02-4431-A168-A6416013C3DD}", 
		 DisplayName = "- Simple SnapIn Sample", 
		 Description = "Simple Hello World SnapIn")]
	public class SimpleSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
		public SimpleSnapIn() 
		{
			// update scope pane with a node in the tree
			this.RootNode = new ScopeNode();
			this.RootNode.DisplayName = "Hello World";
		}
	} // class
 } // namespace
