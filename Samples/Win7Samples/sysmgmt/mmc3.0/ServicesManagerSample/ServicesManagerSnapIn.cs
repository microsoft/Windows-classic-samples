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
//  Shows a complete example putting many things together.
//
//=======================================================================================
//
//  Description:
//  Demonstrates how to utilize localizable resources. Uses an MmcListView to administer 
//  services on a machine. Uses a property page to allow specifying the startup method for Services. 
//  Also has a property page for describing the service. Defines SelectionData Start, 
//  Stop, Pause, Resume actions to manage the selected service.
//
//=======================================================================================

using System;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Configuration.Install;
using System.ComponentModel;
using System.Diagnostics;
using System.Collections;
using System.Management;
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
    /// SnapIn class - Provides the main entry point for the creation of a snap-in.
    /// SnapInSettings Attribute - Used to define the registration information for a SnapIn. 
    /// SnapInAboutAttribute - Used to define the localized "About" information for a snap-in. (See SnapInAbout help for more detail)
    /// </summary>
    [SnapInSettings("497428ED-91C2-A6DA-6A87-3A7344F8BCC8", 
        DisplayName = "- Services Manager Sample", 
        Description = "Sample (Complete) that manages services")]
	[SnapInAbout("ResourceDLL.dll",
		ApplicationBaseRelative = true,
		DisplayNameId = 101, 
		DescriptionId = 102,
		VendorId = 103,
		VersionId = 104,
		IconId = 110,
		LargeFolderBitmapId = 111,
		SmallFolderBitmapId = 112,
		SmallFolderSelectedBitmapId = 112,
        FolderBitmapsColorMask = 0x00ff00 )]
	public class ServicesManagerSnapIn : SnapIn
	{
		public ServicesManagerSnapIn()
		{
			// Load the images for the snap-in.
            // Both the LargeImages and SmallImages collections 
            // should be populated with matching sets of images.
			
            Color transparentColor = Color.FromArgb(0, 255, 0);

            Bitmap bitmap = new Bitmap(typeof(ServicesManagerSnapIn).Assembly.GetManifestResourceStream("Microsoft.ManagementConsole.Samples.Images16.bmp"));
            this.SmallImages.TransparentColor = transparentColor;
            this.SmallImages.AddStrip(bitmap);

            bitmap = new Bitmap(typeof(ServicesManagerSnapIn).Assembly.GetManifestResourceStream("Microsoft.ManagementConsole.Samples.Images32.bmp"));
            this.LargeImages.TransparentColor = transparentColor;
            this.LargeImages.AddStrip(bitmap);

            // Create the root node
			this.RootNode = new ScopeNode();
            this.RootNode.DisplayName = "- Services Manager Sample";

            // Create an MmcListView for the root node.
			MmcListViewDescription lvd = new MmcListViewDescription();
			lvd.DisplayName = "Services";
			lvd.ViewType = typeof(ServicesListView);
			lvd.Options = MmcListViewOptions.SingleSelect;
			
			RootNode.ViewDescriptions.Add(lvd);
			RootNode.ViewDescriptions.DefaultIndex = 0;
		}
	}

} // namespace
