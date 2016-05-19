This house is more than one Simple Dictation

Demonstrates
============
As the name implies it is a simple application that only displays the text of
speech recognition. To use Simple Dictation, begin speaking into the microphone
after the application launches. The results of the speech recognition display in
the text box. There are no other controls for Simple Dictation. Dictation cannot
be turned off through the application and different grammars may not be used. 
Limited text normalization is supported, i.e. saying "five dollars" will
result in "$5.00". Other substitutions, such as "new line" or "paragraph" are not
supported. 

Speak at a normal rate and volume for best results. Do not pause unnecessarily
or excessively between words. Speech recognition yields the best results from
natural speech patterns.
 
Sample Language Implementations
===============================
This sample is available in C++.

Files
=====
SimpleDictation.h       Contains the declaration of the CSimpleDict class.

SimpleDictation.cpp     Contains the entry point for the SimpleDictation
                        application and definitions of methods of CSimpleDict
                        class. 

stdafx.h                Contains the standard system include files and project
                        specific include files that are used frequently, but are
                        changed infrequently.

stdafx.cpp              Generates the precompiled header.

resource.h              Microsoft Developer Studio generated include file. Used
                        by sipmledict.rc.

sipmledict.rc           Resource scripts.
version.rc2

sipmledict.ico          Icon files.
small.ico

SimpleDictation.sln     Microsoft Visual Studio solution file.

SimpleDictation.vcproj  Visual C++ project file.

Readme.txt              This file.

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the SimpleDictation.sln (solution) file to 
       open the file in Visual Studio.
    3. In the Build menu, select Build Solution. The application will be built
       in the "Debug" or "Release" directory for 32-bit platforms, "x64\Debug"
       or "x64\Release" directory for 64-bit platforms.
    
To run the sample:
=================
    1. Navigate to the directory that contains the new executable, using the
       command prompt or Windows Explorer.
    2. Type SimpleDictation.exe at the command line, or double-click the icon for
       SimpleDictation.exe to launch it from Windows Explorer.
