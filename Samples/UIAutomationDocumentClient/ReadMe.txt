================================
UIA Document Content Client Sample
Microsoft UI Automation
================================
This sample is a simple command line tool to examine controls that implement
TextPattern2 and the new Caret and Document features, like annotation.

===============================
Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

=====
Files
=====
UiaDocumentClient.cpp       source file, contains the whole client
UiaDocumentClient.sln       VS solution for the sample
UiaDocumentClient.vcxproj   VS project for the sample
ReadMe.txt                  This readme

========
Building
========

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild UiaDocumentClient.sln


To build the sample using Visual Studio 2011 (preferred method):
================================================
     1. Open File Explorer and navigate to the  directory.
     2. Double-click the icon for the UiaDocumentClient.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


=======
Running
=======
To run the sample:
     1. Navigate to the directory that contains the new executable and dll, using the command prompt.
     2. Type UiaDocumentClient.exe at the command line.

To run from Visual Studio, press F5 or click Debug->Start Debugging.

========
Comments
========
From the command line you can run it with no arguments (in which case it will target the window under the mouse after 3 seconds),
or you can give it a number, the HWND handle of a window you want to target (gotten from Spy++ or Inspect or other tools).

Once the program starts running it should list available commands you can type. You can also type help
at any time to see a list of possible commands.