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
//  Show FormView multi-selection while publishing selection context
//
//=======================================================================================
//
//  Description:
//  Uses a FormView containing a form with a WinForms.ListView. Adds ShowSelected action. 
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
    /// SelectionFormViewSnapIn class - SnapIn that uses a Form in the result pane
    /// </summary>
    [SnapInSettings("{9627F1F3-A6D2-4cf8-90A2-10F85A7A4EE7}",
       DisplayName = "- Selection (FormView) Sample", 
        Description = "Shows FormView with multi-selection.")]
	public class SelectionFormViewSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
		public SelectionFormViewSnapIn()
		{
            // Create the root node
            this.RootNode = new ScopeNode();
            this.RootNode.DisplayName = "Selection (FormView) Sample";

            // Create a form view for the root node.
            FormViewDescription fvd = new FormViewDescription();
            fvd.DisplayName = "Users (FormView)";
            fvd.ViewType = typeof(SelectionFormView); 
            fvd.ControlType = typeof(SelectionControl);

            // Attach the view to the root node
            this.RootNode.ViewDescriptions.Add(fvd);
            this.RootNode.ViewDescriptions.DefaultIndex = 0;
        }
	}

} //namespace
