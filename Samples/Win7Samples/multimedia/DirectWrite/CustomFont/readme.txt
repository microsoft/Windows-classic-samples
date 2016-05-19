CustomFont

Demonstrates
============
How to render fonts that are embedded as resources. This application implements
a custom font file loader to provide access to the embedded font data. This
allows DirectWrite to recognize the family name of the fonts even though these
fonts are not installed on the system. The sample shows how to:

- Implement a custom font file loader, which provides access to font data using
  an integer resource ID as the font file key.
- Implement a custom font collection loader, which enumerates font files using
  an array of integer resource IDs and the font collection key.
- Create a custom font collection comprising two fonts - Pericles and Kootenay -
  from the Sample OpenType Font Pack.
- Format and render some text in these fonts in the application window.

Languages
=========
This sample is available in the following language implementations:
    C++

Files
=====
    CustomFont.cpp: Main application entry point and window.
    Layout.cpp: Macro layout that draws smaller layouts as formatted paragraphs.
    ResourceFontContext.cpp: Registers and creates the custom font collection.
    ResourceFontCollectionLoader.cpp: IDWriteFontCollectionLoader implementation.
    ResourceFontFileEnumerator.cpp: IDWriteFontFileEnumerator implementation
    ResourceFontFileLoader.cpp: IDWriteFontFileLoader implementation.
    ResourceFontFileStream.cpp: IDWriteFontFileStream implementation.
    Common.h: Common definitions and system files.
    resource.h: Menu command identifiers.

Prerequisites
=============
Windows 7 with DirectWrite and D2D.
Windows Software Development Kit (SDK) for Windows 7.

Building the Sample
===================
Include instructions as numbered steps. For example:

To build the sample using the command prompt:
---------------------------------------------
    1.  Open the Command Prompt window and navigate to the directory.
    2.  Type msbuild CustomFont.sln.

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
    2.  Type CustomFont.exe at the command line, or double-click the icon for
        CustomFont.exe to launch it from Windows Explorer.
    3.  The window will then draw the custom fonts.
        Close the window when done.
