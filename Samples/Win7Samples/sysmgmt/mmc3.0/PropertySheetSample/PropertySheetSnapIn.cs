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
//  Shows how to add property pages.
//
//=======================================================================================
//
//  Description:
//  Uses a MmcListView to load a list of Users. Adds a property page to the property 
//  sheet for the selected ResultNode.
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
    /// PropertySheetSnapIn class - Shows property page for selected list item 
	/// </summary>
    [SnapInSettings("{2D3BD1F0-1404-4fcf-BBE1-0F45E908E923}", 
		 DisplayName = "- Property Sheet Sample", 
		 Description = "User List with Property Sheet SnapIn")]
	public class PropertySheetSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
		public PropertySheetSnapIn() 
		{
			// snap-in node
			ScopeNode scopeNode = new SampleScopeNode();
			scopeNode.DisplayName = "Property Sheet Sample";
			this.RootNode = scopeNode;
            this.RootNode.EnabledStandardVerbs = StandardVerbs.Properties;

			// snap-in result list view
			MmcListViewDescription mmcListViewDescription = new MmcListViewDescription();
			mmcListViewDescription.DisplayName = "User List with Properties";
			mmcListViewDescription.ViewType = typeof(UserListView);
			mmcListViewDescription.Options = MmcListViewOptions.SingleSelect;
			scopeNode.ViewDescriptions.Add(mmcListViewDescription);
			scopeNode.ViewDescriptions.DefaultIndex = 0;
		}

    }

    /// <summary>
    /// 
    /// </summary>
    public class SampleScopeNode : ScopeNode
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public SampleScopeNode()
        { 
        }

        /// <summary>
        /// OnAddPropertyPages is used to get the property pages to show. 
        /// (triggered by Properties verbs)
        /// </summary>
        /// <param name="propertyPageCollection">property pages</param>
        protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
        {
            propertyPageCollection.Add(new ScopePropertyPage(this));
        }
    }

    

} // namespace
	