========================================================================
       Windows Application : Enhanced Storage Managed Sample
========================================================================
Demonstrates working with Enhanced Storage API using managed code and C#

Sample Language Implementations:
===============================
     This sample is available in the following language implementations:
     C#

Files:
===============================
EhStorManaged\Program.cs - contains main code that demonstrates enumeration of devices in system.
EhStorManaged\PortableDeviceManagerImp.cs - sample wrapper for the Portable Device Manager.
microsoft.storage.EhStorage.dll\EhStorAPIHelper.cs - Sample wrappers for Enhanced Storage API.
microsoft.storage.EhStorage.dll\EhStorAPIInterop.cs - Enhanced Storage API interop.
microsoft.storage.EhStorage.dll\PortableDeviceAPIInterop.cs - Portable Device Manager interop.
 
Prerequisites:
===============================
For Windows Vista Enhanced Storage support should be installed first.
.NET framework 2.0 or higher required.

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory.
     2. Type msbuild EhStorManaged.sln.

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type EhStorManaged at the command line, or double-click the icon for EhStorManaged.exe to launch it from Windows Explorer.


Comments:
=================
Portable device API interop has been implemented for the needs of Enhanced Storage.
Some functions and interfaces may not be present or present in limited form.
