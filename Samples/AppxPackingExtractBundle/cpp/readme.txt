
Appx Bundle Extraction Sample
=============================

     This sample demonstrates how to use Appx Bundle APIs to read an Appx Bundle and extract its contents to disk.


Prerequisites
=============

     This sample requires Windows 8.1 or higher, and an installation of Visual Studio 2012 Ultimate.


Sample Language Implementations
===============================

     This sample is available in the following language implementations:

     C++


Files
=====

     ExtractBundle.cpp - main entry point for the sample application

     ExtractBundle.h - main header file

     ExtractBundle.vcxproj - build configuration for this sample

     ExtractBundle.sln - Visual Studio 2012 Solution file for this sample

     Data\sample.appxbundle - A sample Appx Bundle that can be used as input for this sample


To build the sample using the command prompt:
=============================================

     1. Open the Command Prompt window and navigate to the directory containing ExtractBundle.vcxproj

     2. Type: msbuild ExtractBundle.vcxproj


To build the sample using Visual Studio 2012 Ultimate (preferred method):
=========================================================================

     1. Open File Explorer and navigate to the directory containing ExtractBundle.vcxproj.

     2. Double-click the icon for the ExtractBundle.vcxproj file to open the file in Visual Studio.

     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================

     1. Open a command prompt window and navigate to the directory where ExtractBundle.exe is built.

     2. Type the command:

             ExtractBundle.exe inputFile outputPath

        inputFile: the full or relative path to an Appx Bundle that can be read

        outputPath: the full or relative path where contents extracted from the bundle can be written.  This path should be 150 characters or less to avoid hitting limitations of this sample.

        Example:   ExtractBundle.exe "Data\sample.appxbundle" "outputFolder"

     3. When the application exits successfully, the output folder should contain all payload packages and footprint files extracted from the input bundle.
