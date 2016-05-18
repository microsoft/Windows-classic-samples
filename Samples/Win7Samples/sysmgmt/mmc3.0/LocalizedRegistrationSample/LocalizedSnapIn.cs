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
//  Show how to localize the snap-in registration
//
//=======================================================================================
//
//  Description:
//  Has two projects. The first defines a new resource.dll. The second uses the 
//  SnapInAbout attribute to define resources in the external dll to be used to 
//  register the snapIn.
//
//=======================================================================================

using System;
using System.ComponentModel;
using System.Configuration.Install;
using System.Resources;
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
	/// SnapInAbout attribute - Used to set the localized about information for the snap-in.
    /// SnapIn class - Provides the main entry point for the creation of a snap-in. 
    /// LocalizedRegistrationSnapIn class - Registered using information from the resource dll
	/// </summary>
    [SnapInSettings("{097AD391-496C-474c-9A11-D705EF97C2BC}", 
		 DisplayName = "- Localized Registration", 
		 Description = "Sample Registered using external resource")]
    [SnapInAbout("Resource-en.dll",
        ApplicationBaseRelative = true,
        DisplayNameId = 101,
        DescriptionId = 102,
        VendorId = 103,
        VersionId = 104,
        IconId = 110,
        LargeFolderBitmapId = 111,
        SmallFolderBitmapId = 112,
        SmallFolderSelectedBitmapId = 112,
        FolderBitmapsColorMask = 0x00ff00)]
	public class LocalizedSnapIn : SnapIn
	{
        /// <summary>
        /// Constructor
        /// </summary>
        public LocalizedSnapIn() 
		{
			this.RootNode = new ScopeNode();
            this.RootNode.DisplayName = "Registration info came from a separate.dll.";
		}
	} 

} // namespace
