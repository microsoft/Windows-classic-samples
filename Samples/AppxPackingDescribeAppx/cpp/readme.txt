
Appx Package Description Sample
===============================

     This sample demonstrates how to use Appx Manifest and Blockmap APIs to get a description of the Appx package's payload files and manifest attributes.


Prerequisites
=============

     This sample requires Windows 8.1 or higher, and an installation of Visual Studio 2012 Ultimate.


Sample Language Implementations
===============================

     This sample is available in the following language implementations:

     C++


Files
=====

     DescribeAppx.cpp - main entry point for the sample application

     DescribeAppx.h - main header file

     DescribeAppx.vcxproj - build configuration for this sample

     DescribeAppx.sln - Visual Studio 2012 Solution file for this sample

     Data\SamplePackage.appx - A sample Appx package containing manifest and block map to be read


To build the sample using the command prompt:
=============================================

     1. Open the Command Prompt window and navigate to the directory containing DescribeAppx.vcxproj

     2. Type: msbuild DescribeAppx.vcxproj


To build the sample using Visual Studio 2012 Ultimate (preferred method):
=========================================================================

     1. Open File Explorer and navigate to the directory containing DescribeAppx.vcxproj.

     2. Double-click the icon for the DescribeAppx.vcxproj file to open the file in Visual Studio.

     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================

     1. Open a command prompt window and navigate to the directory where DescribeAppx.exe is built.

     2. Type the command:

             DescribeAppx.exe inputFile

        inputFile: path to an Appx package that can be read, for example "data\SamplePackage.appx"
