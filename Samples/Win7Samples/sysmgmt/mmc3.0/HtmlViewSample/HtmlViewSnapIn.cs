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
//  Shows an html view in the Result Pane
// 
//=======================================================================================;
//
//  Description:
//  Uses the HtmlView to show the site www.microsoft.com in the result pane.
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
    /// SnapIn class - Provides the main entry point for the creation of a snap-in. 
    /// HtmlViewSnapIn class - SnapIn to show a web page 
	/// </summary>
    [SnapInSettings("{FA14D0D0-B368-4405-B1A3-C33637CCA46D}", 
		 DisplayName = "- HtmlView Sample", 
		 Description = "HtmlView (http://www.microsoft.com) SnapIn")]
	public class HtmlViewSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
        public HtmlViewSnapIn()
        {
            // update scope pane with a node in the tree
            this.RootNode = new ScopeNode();
            this.RootNode.DisplayName = "HtmlView Sample";

            // set up the update result pane when scope node selected
            HtmlViewDescription hvd = new HtmlViewDescription();
            hvd.DisplayName = "HtmlView (http://www.microsoft.com)";
            hvd.Url = new Uri("http://www.microsoft.com");

            // attach the view and set it as the default to show
            this.RootNode.ViewDescriptions.Add(hvd);
            this.RootNode.ViewDescriptions.DefaultIndex = 0;
        }

	} // class
 } // namespace
