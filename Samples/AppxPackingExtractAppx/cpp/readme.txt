
Appx Package Extraction Sample
==============================

     This sample demonstrates how to use Appx packaging APIs to read an Appx package and extract its contents to disk.


Prerequisites
=============

     This sample requires Windows 8 or higher, and an installation of Visual Studio 2012 Ultimate.


Sample Language Implementations
===============================

     This sample is available in the following language implementations:

     C++


Files
=====

     ExtractAppx.cpp - main entry point for the sample application

     ExtractAppx.h - main header file

     ExtractAppx.vcxproj - build configuration for this sample

     ExtractAppx.sln - Visual Studio 2012 Solution file for this sample

     Data\HelloWorld.appx - A sample Appx package that can be used as input for this sample


To build the sample using the command prompt:
=============================================

     1. Open the Command Prompt window and navigate to the directory containing ExtractAppx.vcxproj

     2. Type: msbuild ExtractAppx.vcxproj


To build the sample using Visual Studio 2012 Ultimate (preferred method):
=========================================================================

     1. Open File Explorer and navigate to the directory containing ExtractAppx.vcxproj.

     2. Double-click the icon for the ExtractAppx.vcxproj file to open the file in Visual Studio.

     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================

     1. Open a command prompt window and navigate to the directory where ExtractAppx.exe is built.

     2. Type the command:

             ExtractAppx.exe inputFile outputPath

        inputFile: the full or relative path to an Appx package that can be read

        outputPath: the full or relative path where extracted package contents can be written.  This path should be 150 characters or less to avoid hitting limitations of this sample.

        Example:   ExtractAppx.exe "data\HelloWorld.appx" "outputFolder"

     3. When the application exits successfully, the output folder should contain all payload and footprint files extracted from the input package.
