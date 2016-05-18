
FlowText sample

Summary
=============================================
This sample demonstrates the use of DWrite API to perform text layout and XPS Object Model to serialize to XPS document.


Sample Language implementation
=============================================
The sample uses C++ programming language.


Sample Files
=============================================
Readme.txt - this file
FlowText.sln - Visual C++ solution
FlowText.vcproj - project file
common.h - common header for all source (CPP) files
LayoutToCanvasBuilder.h/cpp - definition and implementation of IDWriteTextLayout interface
FlowTextMain.cpp - sample main functions


To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the directory.
     2. Type msbuild FlowText.sln.


To build the sample using Visual Studio 2008 (preferred method):
================================================
     1. Open Windows Explorer and navigate to the  directory.
     2. Double-click the icon for the FlowText.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
=================
     1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
     2. Type FlowText.exe at the command line, or double-click the icon for FlowText.exe to launch it from Windows Explorer.
     3. Find generated SDKSample_FlowText_Output.xps file on desktop and double-click it to open in Win7 XPS Viewer.
