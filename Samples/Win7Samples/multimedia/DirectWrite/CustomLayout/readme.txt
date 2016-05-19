CustomLayout

Demonstrates
============
How to utilize DirectWrite's lower text analysis and font API layers to create a
custom layout, useful when unique features are needed that aren't supported by
DWrite's general layout, such as arbitrarily shaped text flow. These are the
same text analysis and font functions used by DirectWrite's own layout. The
sample shows how to:

- Call the text analyzer to record results for bidi, line breaking, and script
  itemization.
- Shape text into glyphs using the script itemizer and shaping.
- Measure and fit text using returned glyph advances and clusters.
- Break lines using line breakpoints information of hard breaks, word breaks,
  and whitespace.
- Visually reorder text using the bidirectional analysis results (for
  right-to-left languages).
- Basic justification utilizing whitespace and glyph advance information.

Languages
=========
This sample is available in the following language implementations:
    C++

Files
=====
    CustomLayout.cpp: Main application entry point and window.
    FlowLayout.cpp: Custom layout supporting arbitrarily shaped text flow.
    FlowSource.cpp: Source used by the layout to read shape information from.
    FlowSink.cpp: Sink used by the layout to push finalized glyphs to.
    TextAnalysis.cpp: Class to call the analyzer and hold textual results.
    Common.h: Common definitions and system files.
    resource.h: Menu command identifiers.

Prerequisites
=============
Windows 7 with DirectWrite.
Windows Software Development Kit (SDK) for Windows 7.

Building the Sample
===================

To build the sample using the command prompt:
---------------------------------------------
    1.  Open the Command Prompt window and navigate to the directory.
    2.  Type msbuild CustomLayout.sln.

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
    2.  Type CustomLayout.exe at the command line, or double-click the icon for
        CustomLayout.exe to launch it from Windows Explorer.
    3.  You can now resize the window to watch the text flow in action, change
        the displayed text, switch shapes, or toggle number substitution
        (only noticeable when Arabic text is displayed).
