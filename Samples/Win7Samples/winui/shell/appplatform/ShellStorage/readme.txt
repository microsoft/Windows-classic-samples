Shell Storage Sample
================================
Demonstrates how to use the IStorage and IStream APIs to create files and folders in shell containers like libraries or any container picked from either the folder picker dialog or the SHBrowseForFolder dialog. Also demonstrates how to save directly to to the shell item returned from the file dialog using these APIs.


Sample Language Implementations
===============================
C++


Files:
=============================================
ShellStorage.h
ShellStorage.cpp
ShellStorage.sln
ShellStorage.vcproj


To build the sample using the Visual Studio command prompt:
===========================================================
     1. Open the Command Prompt window and navigate to the ShellStorage directory.
     2. Type msbuild.exe ShellStorage.sln.



To build the sample using Visual Studio/Visual C++ Express Edition (preferred method):
======================================================================================
     1. Open Windows Explorer and navigate to the ShellStorage directory.
     2. Double-click the icon for the ShellStorage.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Note: If you are compiling 64-bit using VC++ Express Edition, you will need to use x64 cross-compiler supplied with the Windows SDK.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type ShellStorage.exe at the command line, or double-click the icon for ShellStorage.exe to launch it from Windows Explorer.
     
     Note: This tool creates a file (ShellStorageSample.txt) and a folder (ShellStorageSample) in the selected location.