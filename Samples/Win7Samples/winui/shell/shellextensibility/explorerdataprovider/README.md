---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
urlFragment: explorerdataprovider
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: "Implements a shell namespace extension, including context menus and custom tasks in the browser."
---

Explorer Data Provider Sample
================================
This sample demonstrates how to implement a shell namespace extension, including context menu
behavior and custom tasks in the browser.

Sample Language Implementations
===============================
C++

Files
=============================================
* Category.cpp
* Category.h
* ContextMenu.cpp
* Dll.cpp
* ExplorerDataProvider.cpp
* ExplorerDataProvider.def
* ExplorerDataProvider.propdesc
* ExplorerDataProvider.rc
* ExplorerDataProvider.sln
* ExplorerDataProvider.vcproj
* fvcommands.cpp
* fvcommands.h
* Guid.h
* resource.h
* Utils.cpp
* Utils.h


To build the sample using the command prompt:
=============================================
1. Open the Command Prompt window and navigate to the ExplorerDataProvider directory.
2. Type msbuild ExplorerDataProvider.sln.


To build the sample using Visual Studio (preferred method):
===========================================================
1. Open Windows Explorer and navigate to the ExplorerDataProvider directory.
2. Double-click the icon for the ExplorerDataProvider.sln file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The DLL will be built in the default \Debug or \Release directory.


To run the sample:
=================
1. Navigate to the directory that contains the new .DLL and .propdesc file using the command prompt.
2. Type "regsvr32.exe ExplorerDataProvider.dll" at the command line.
   1. If you run this command from an elevated command prompt, the self-registration will also register
      the .propdesc file automatically.  If it is run from a non-elevated command prompt then the
      namespace extension will still work but without custom property functionality.
3. Open the My Computer folder and browse the new namespace extension present there.
