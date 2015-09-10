================================
UIA Document Content Provider Sample
Microsoft UI Automation
================================
This sample is a simple (trivial) text viewer, that implements the UI Automation text pattern,
showcasing the new Document Content and Caret features, including Annotation support.

===============================
Sample Language Implementations
===============================
This sample is available in the following language implementations:
     C++

=====
Files
=====
AnnotatedTextControl.cpp                    Implementation of the simple Text Viewer control
AnnotatedTextControl.h                      Header for the simple Text Viewer control
AnnotationProvider.cpp                      UI Automation implementation for the Annotations
AnnotationProvider.h                        Header for the UIA provider for the Annotations
FrameProvider.cpp                           UI Automation implementation for the root window
FrameProvider.h                             Header for the UIA provider for the root window
TextAreaProvider.cpp                        UI Automation implementation for the Text Area
TextAreaProvider.h                          Header for the UIA provider for the Text Area
UiaDocumentProvider.cpp                     main file, makes application window
UiaDocumentProvider.sln                     VS solution for the sample
UiaDocumentProvider.vcxproj                 VS project for the sample
UiaDocumentProviderSample.partial.manifest  Manifest for the host application (enables common control v6)
ReadMe.txt				    This readme

========
Building
========

To build the sample using the command prompt:
=============================================
     1. Open the Command Prompt window and navigate to the  directory.
     2. Type msbuild UiaDocumentProvider.sln


To build the sample using Visual Studio 2011 (preferred method):
================================================
     1. Open File Explorer and navigate to the  directory.
     2. Double-click the icon for the UiaDocumentProvider.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


=======
Running
=======
To run the sample:
     1. Navigate to the directory that contains the new executable and dll, using the command prompt or File Explorer.
     2. Type UiaDocumentProvider.exe at the command line, or double-click the icon for UiaDocumentProvider.exe 
	to launch it from File Explorer.

To run from Visual Studio, press F5 or click Debug->Start Debugging.

========
Comments
========
The sample will show a simple document, with formatting and comments. It is fixed in this sample, the cursor is merely for browsing.
To see the real work the sample does attach a Text Pattern client, such as Inspect or the UiaDocumentClient sample.