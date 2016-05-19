RenderTest

Demonstrates
============
An overview tour of DirectWrite's layout/rendering API's and an up-close view
of concepts like subpixel rendering and text transformation. The sample shows
how to:

- Create a text format from current font family, font size, weight, stretch,
  and style from the standard Win32 font selection dialog.
- Create a layout object from the current string, text format, measuring
  mode, etc.
- Set the current transform and show a magnified view.
- Draw the text layout object to a GDI DIB or D2D render target.
- Respect multi-monitor settings set by the ClearType Tuner.

Languages
=========
This sample is available in the following language implementations:
    C++

Files
=====
    RenderTest.cpp: Main application entry point and window.
    IRenderer.h: Abstract rendering target interface.
    D2DRenderer.cpp: D2D specific renderer.
    DWriteRenderer.cpp: DWrite specific renderer.
    TextHelpers.cpp: Helper conversion functions.
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
    1.  Open the Command Prompt window and navigate to the  directory.
    2.  Type msbuild RenderTest.sln.

To build the sample using Visual Studio 2008 (preferred method):
----------------------------------------------------------------
    1.  Open Windows Explorer and navigate to the directory.
    2.  Double-click the icon for the .sln (solution) file to open the file in
        Visual Studio.
    3.  In the Build menu, select Build Solution. The application will be built
        in the default \Debug_[x64/Win32] or \Release_[x64/Win32] directory.

Running the Sample
==================
    1.  Navigate to the directory that contains the new executable,
        using the command prompt or Windows Explorer.
    2.  Type RenderTest.exe at the command line, or double-click the icon for
        RenderTest.exe to launch it from Windows Explorer.
    3.  You can then nudge the text left/right, rotate it, change the font,
        and select different measuring modes.
