Windows Media Player Plug-in Wizard

Installing the Plug-in Wizard
You must install the Windows Media Player plug-in wizard to use it. Use the following steps to install the wizard.


1. Locate the folder where you installed the Windows SDK. Expand the folder to view its subfolders, and navigate to Samples\Multimedia\WMP\wizards\wmpwiz.



2. Locate the following three files:
     wmpwiz.vsz
     wmpwiz.vsdir
     wmpwiz.ico



3. Using Notepad, edit the wmpwiz.vsz file.
     
Locate the following line:
Wizard=VsWizard.<VsWizardEngine version goes here>

Change <VsWizardEngine version goes here> to one of the following values, depending on which version of Visual Studio you have installed.

Value                    Visual Studio Version
------------------       -----------------------
VsWizardEngine.7.1       Visual Studio .NET 2003
VsWizardEngine.8.0       Visual Studio 2005
VsWizardEngine.9.0       Visual Studio 2008

Locate the following line:
Param="ABSOLUTE_PATH = <path to wmpwiz directory goes here>"

Change <path to wmpwiz directory goes here> to the path where the wizard files are located.

For example, suppose you have Visual Studio 2008, and your wizard files are here:
C:\Program Files\Microsoft SDKs\Windows\v7.0\Samples\Multimedia\WMP\wizards\wmpwiz

Then your wmpwiz.vsz file would look like this:

VSWIZARD 7.0
Wizard=VsWizard.VsWizardEngine.9.0

Param="WIZARD_NAME = Windows Media Player Plug-in Wizard"
Param="ABSOLUTE_PATH = C:\Program Files\Microsoft SDKs\Windows\v7.0\Samples\Multimedia\WMP\wizards\wmpwiz"
Param="FALLBACK_LCID = 1033"



4. Locate the folder where you installed Visual Studio. Expand the folder to view its subfolders, and locate a folder named vcprojects.


5. Copy the three files listed in step 2 to the vcprojects folder. The wizard is now installed.


Copyright (c) Microsoft Corporation. All rights reserved.