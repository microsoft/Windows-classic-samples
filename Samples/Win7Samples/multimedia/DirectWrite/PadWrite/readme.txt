PadWrite

Demonstrates
============
How to use DirectWrite’s layout API for features like ranged formatting and
hit-testing. It supports basic editing to allow interactive modification of
the displayed text and attributes. The sample shows how to:

- Assign layout-wide attributes like text alignment, reading direction, and 
  trimming.
- Assign ranged attributes such font weight, font size, and font family.
- Use hit-testing to map mouse coordinates to text positions and draw
  selection ranges.
- Apply a transform while retaining WYSIWIG document layout.
- Load an inline image for use in layout via WIC.
- Use drawing effects and inline images with either a D2D render target or
  GDI DIB.

Languages
=========
This sample is available in the following language implementations:
     C++

Files
=====
    PadWrite.cpp: Main application entry point and window.
    TextEditor.cpp: Custom control the displays a text layout.
    EditableLayout.cpp: Layout adapter to assist with editing.
    RenderTarget.cpp: Render target for rendering the layout.
    DrawingEffect.h: Simple color applied to range.
    InlineImage.cpp: WIC image used as an inline object in layout.
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
    2.  Type msbuild PadWrite.sln.

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
    2.  Type PadWrite.exe at the command line, or double-click the icon for
        PadWrite.exe to launch it from Windows Explorer.
    3.  Use the keys to move the caret or mouse to select a region. Choose a
        different font, load an image, zoom in/out, or change options like
        line wrapping and trimming.
