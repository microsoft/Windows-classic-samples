Sample Name
Xps Load Modify Save

Demonstrates
How to load an xps file from a stream, modify and save it.

Languages

Sample Language Implementations
===============================
     This sample is available in the following language implementations:
     C++

Files
main.cpp: The main file that has the essentials of creating the xps object model in memory, 
          browsing it, modifying and subsequently saving.
resource.h: Resource Definitions
sample1.xps: The sample xps file used for demonstrating above functionality. 
             NOTE: This file is built into the sample as an embedded resource. If you modify this file, you will have to rebuild the sample. 
XpsLoadModifySave.rc : Resource file which includes sample1.xps

 
Prerequisites
You must be running Windows 7. The sample does not run on previous version of windows.

Building the Sample

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild XpsLoadModifySave.sln


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.

Running the Sample
This section is optional. Depending on the sample you may choose to include these as steps.  For example:

To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type XpsLoadModifySave.exe at the command line, or double-click the icon for XpsLoadModifySave.exe to launch it from Windows Explorer.

The sample loads the xps file file (which is attached as a resource to the executable), modifies it and saves it as SDKSample_XpsLoadModifySave.xps on the user's desktop
