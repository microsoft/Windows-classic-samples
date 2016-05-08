Preview Handler Sample (.recipe previewer)
===============================
This sample demonstrates how to write a handler for displaying a preview inside the
Windows Explorer preview pane or other preview handler hosts.

Sample Language Implementations
===============================
C++

Files
===============================
RecipePreviewHandler.cpp
PreviewHandlerSDKSample.def
PreviewHandlerSDKSample.vcproj
PreviewHandlerSDKSample.sln

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type: msbuild PreviewHandlerSDKSample.sln

To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The DLL will be built in the default \Debug or \Release directory.

To run the sample:
=================
     0. Run "regsvr32.exe PreviewHandlerSDKSample.dll" to register the handler.
     1. Navigate Windows Explorer to the directory that contains the sample files.
     2. Select the example .recipe file.
     3. Show the Preview Pane if it is not already showing.
        Win7: Click the preview pane button.
        Vista: Click Organize -> Layout -> Preview Pane
        
64-bit Note
===========
THIS SAMPLE AS-IS MUST BE BUILT 64-BIT FOR 64-BIT TARGETS
You can also make the 32-bit output work on a 64-bit version of Windows by changing the AppId
on the class registration to the WOW64 surrogate host (which will work for x86 and x64 versions of Windows).
That AppId value is: {534A1E02-D58F-44f0-B58B-36CBED287C7C}


Removing the Sample
==================
Run "regsvr32.dll /u PreviewHandlerSDKSample.dll"