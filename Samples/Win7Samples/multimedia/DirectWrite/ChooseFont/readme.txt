ChooseFont

Demonstrates
============
How to enumerate font families and faces using DirectWrite, for a scenario where
the application displays a font chooser. The sample shows how to:

- Enumerate font families and family font faces, populating a listbox.
- Render the selected font to a DIB.
- Return a TextFormat matching the user's selections from the dialog.

Languages
=========
This sample is available in the following language implementations:
    C++

Files
=====
    ChooseFont.cpp: Main application entry point and window.
    FontEnumeration.cpp: Support routines to enumerate and retain information.
    GdiTextRenderer.cpp: Renderer callback implementation onto a DIB.
    resource.h: Menu command identifiers.

Prerequisites
=============
Windows 7 with DirectWrite.
Windows Software Development Kit (SDK) for Windows 7.

Building the Sample
===================
Include instructions as numbered steps. For example:

To build the sample using the command prompt:
---------------------------------------------
    1.  Open the Command Prompt window and navigate to the directory.
    2.  Type msbuild ChooseFont.sln.

To build the sample using Visual Studio 2008 (preferred method):
----------------------------------------------------------------
    1.  Open Windows Explorer and navigate to the  directory.
    2.  Double-click the icon for the .sln (solution) file to open the file in
        Visual Studio.
    3.  In the Build menu, select Build Solution. The application will be built
        in the default \Debug_[x64/Win32] or \Release_[x64/Win32] directory.

Running the Sample
==================
    1.  Navigate to the directory that contains the new executable,
        using the command prompt or Windows Explorer.
    2.  Type ChooseFont.exe at the command line, or double-click the icon for
        ChooseFont.exe to launch it from Windows Explorer.
    3.  Select different font sizes (fractional sizes supported) to see the
        sample text change or select different families to see the supported
        face variants.
