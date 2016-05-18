Speech List Box

Demonstrates
============
Speech List Box is an elementary application showcasing speech recognition and
dynamic grammars. The sample consists of two projects: SpeechListBox, which is
the speech enabled ListBox control, and SpeechListBoxApplication, which uses
this control.

SpeechListBoxApplication opens with no words in the dynamic grammar. You can add
words or phrases by typing the text in the edit box and clicking Add. After
adding at least one item, individual items may be highlighted by saying "select"
followed by the text of the item. For example, if one of the items a list of
cities were "Seattle," saying "Select Seattle" will choose the Seattle item.
 
Sample Language Implementations
===============================
This sample is available in C#.

Files
=====
SpeechListBox\SpeechListBox.cs      Implementation of SpeechListBox control.

SpeechListBox\SpeechListBox.resx    .NET Managed resource file.

SpeechListBox\AssemblyInfo.cs       General information about SpeechListBox
                                    assembly.
                                    
SpeechListBox\small.ico             Icon resource.
                                    
SpeechListBox\SpeechListBox.csproj  Visual C# project file.

SpeechListBoxApplication\SpeechListBoxApplication.cs
                                    Implementation of SpeechListBoxApplication.

SpeechListBoxApplication\SpeechListBoxApplication.resx
                                    .NET Managed resource file.

SpeechListBoxApplication\AssemblyInfo.cs
                                    General information about
                                    SpeechListBoxApplication assembly.
                                    
SpeechListBoxApplication\small.ico  Icon resource.
                                    
SpeechListBoxApplication\SpeechListBoxApplication.csproj
                                    Visual C# project file.


ListBox.sln                         Microsoft Visual Studio solution file.

Readme.txt                          This file.

To build the sample using the command prompt:
=============================================
    1. Open the Command Prompt window and navigate to the directory.
    2. Type "msbuild".

To build the sample using Visual Studio 2005 or Visual Studio 2008:
==================================================================
    1. Open Windows Explorer and navigate to the directory.
    2. Double-click the icon for the ListBox.sln (solution) file
       to open the file in Visual Studio.
    3. In the Build menu, select Build Solution. The sample will be built in the
       "bin\Debug" or "bin\Release" directory.
    
To run the sample:
=================
    1. Navigate to the directory that contains the executable, using the command
       prompt or Windows Explorer.
    2. Type SpeechListBoxApplication.exe at the command line, or double-click
       the icon for SpeechListBoxApplication.exe to launch it from Windows
       Explorer.
