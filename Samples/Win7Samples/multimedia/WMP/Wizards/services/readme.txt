Windows Media Player Online Stores Wizard

Before using the Online Stores wizard, you must install the Windows SDK, which includes the Windows Media Player SDK. You must also install Visual Studio. If you intend to use the wizard to create a type 1 plug-in, you must install Visual Studio 2008 or Visual Studio 2005. If you intend to use the wizard to create a type 2 plug-in, you can install Visual Studio 2008, Visual Studio 2005, or Visual Studio .NET 2003.

-----------------------------------
Installing the Online Stores Wizard
-----------------------------------

1. Locate the folder where you installed the Windows SDK. Expand the folder to view its subfolders, and navigate to Samples\Multimedia\WMP\wizards\services.



2. Locate the following three files:
     wmpservices.vsz
     wmpservices.vsdir
     wmpservices.ico



3. Using Notepad, edit the wmpservices.vsz file.
     
Locate the following line:
Wizard=VsWizard.<VsWizardEngine version goes here>

Change <VsWizardEngine version goes here> to one of the following values, depending on which version of Visual Studio you have installed.

Value                    Visual Studio Version
------------------       -----------------------
VsWizardEngine.7.1       Visual Studio .NET 2003
VsWizardEngine.8.0       Visual Studio 2005
VsWizardEngine.9.0       Visual Studio 2008

Locate the following line:
Param="ABSOLUTE_PATH = <path to wmpservices directory goes here>"

Change <path to wmpservices directory goes here> to the path where the wizard files are located.

For example, suppose you have Visual Studio 2008, and your wizard files are here:
C:\Program Files\Microsoft SDKs\Windows\v7.0\Samples\Multimedia\WMP\wizards\services.

Then your wmpservices.vsz file would look like this:

VSWIZARD 7.0
Wizard=VsWizard.VsWizardEngine.9.0

Param="WIZARD_NAME = wmpservices"
Param="ABSOLUTE_PATH = C:\Program Files\Microsoft SDKs\Windows\v7.0\Samples\Multimedia\WMP\wizards\services"
Param="FALLBACK_LCID = 1033"



4. Locate the folder where you installed Visual Studio. Expand the folder to view its subfolders, and locate a folder named vcprojects.


5. Copy the three files listed in step 2 to the vcprojects folder. The wizard is now installed.



------------------------------------
Using the Sample Type 1 Online Store
------------------------------------

For a type 1 plug-in, the wizard displays a dialog box that has two input boxes: Friendly Name and Content Distributor. The default value for Content Distributor is MSSampleType1Store. If you keep the default value for Content Distributor, your plug-in will be registered to work with Microsoft's sample type 1 online store.

To see the sample type 1 store on the Online Stores menu in Windows Media Player, create the following entry in the registry.

HKEY_CURRENT_USER\Software\Microsoft\MediaPlayer\Services
   TestParameter     REG_SZ     2573


Copyright (c) Microsoft Corporation. All rights reserved.