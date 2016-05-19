BootConfigurationData Sample

Introduction
==============================

BootConfigurationData sample is an application that demonstrates usages of the BCD APIs to manipulate the BCD store.


Sample Language Implementations
===============================

This sample is available in the following language implementations:
     C#


Files
===============================
    .\readme.txt (this file)
    .\BcdSampleApp.sln
    .\BcdSampleApp\BcdSampleApp.csproj
    .\BcdSampleApp\MainForm.cs
    .\BcdSampleApp\MainForm.Designer.cs
    .\BcdSampleApp\MainForm.resx
    .\BcdSampleApp\Program.cs
    .\BcdSampleApp\properties\AssemblyInfo.cs
    .\BcdSampleApp\properties\Resources.Designer.cs
    .\BcdSampleApp\properties\Resources.resx
    .\BcdSampleApp\properties\Settings.Designer.cs
    .\BcdSampleApp\properties\Settings.settings
    .\BcdSampleLib\BcdSampleLib.csproj
    .\BcdSampleLib\BcdStore.API.cs
    .\BcdSampleLib\Constants.cs
    .\BcdSampleLib\ROOT.WMI.BcdObject.cs
    .\BcdSampleLib\ROOT.WMI.BcdObjectListElement.cs
    .\BcdSampleLib\ROOT.WMI.BcdStore.cs
    .\BcdSampleLib\ROOT.WMI.BcdStringElement.cs
    .\BcdSampleLib\Utils.cs
    .\BcdSampleLib\properties\AssemblyInfo.cs


Building the Sample
===============================

BootConfigurationData sample a Visual Studio 2008 solution that contains two projects, BcdSampleApp and BcdSampleLib.
BcdSampleApp is a windows forms sample app that acts as a test app for BcdSampleLib, which contains usages of the APIs. 
You are free to use the BcdSampleLib alone.

To build the sample using the command prompt:
=============================================
     1. Open an SDK CMD Shell window and navigate to the directory.
     2. Type msbuild [Solution Filename].


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


Running the Sample
===============================
     1. Right-click on BcdSampleApp.exe.
     2. Select "Run as administrator".

Note that BcdSampleApp.exe must be run elevated (step 2 above), because accessing BCD requires admin privilege.  Likewise, if you execute
the app from within VS2008, you must elevate VS2008.

Alternatively if you want automatic LUA behavior by double clicking the BcdSampleApp.exe (or your app that you created that uses BcdSampleLib),
then you will need to add a file like the following below in the same directory as the exe, with the name myApp.exe.manifest (if your app is
named myApp.exe).

The contents of this file should be the following xml (this is entire file contents):

BcdSampleApp.exe.manifest contents: (use same name as your exe + ".manifest", and put in same folder as your EXE).
----------------------------------------------------------

<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">   
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level="requireAdministrator" uiAccess="false"></requestedExecutionLevel>
        </requestedPrivileges>
       </security>
  </trustInfo>
</assembly>

A better solution would be to incorporate this manifest directly into the BcdSampleApp.exe as a post install step from Visual Studio 2008,
perhaps using the mt.exe utility (other utilities exist to sign manifest).  A standard usage for the mt.exe utility might be:

mt -manifest BcdSampleApp.exe.manifest -outputresource:BcdSampleApp.exe;#1

This updates BcdSampleApp.exe's internal manifest to now be the same as the BcdSampleApp.exe.manifest file.  If this approach doesn't work
you will have to use the manifest as an external file and keep it in same folder as your exe app.

Microsoft SDK publishing rules do not usually allow exe's to be published, and your Build Output Path may be either bin/Debug, bin/Release or a 
custom location.  Please copy the included (or your created) manifest file to your correct Build Output Path, and on deployment, make sure you 
deploy the manifest file along with your final exe application.  Remember the manifest must be in same folder as the exe, unless you
are able to embed the manifest file using mt.exe.

If you don't see any .manifest files, or lose your manifest file, you can create a file with the name and content as described above.

Right clicking and choosing run as administrator should always work, with or without manifest files, but double clicking will typically fail 
ungracefully without a manifest file (or embedded manifest in exe).


Comments
===============================

Additional documentation on the new Bcd classes can be searched for on msdn.







