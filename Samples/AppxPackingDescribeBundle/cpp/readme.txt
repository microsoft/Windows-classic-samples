
Appx Bundle Description Sample
==============================

     This sample demonstrates how to use Appx Bundle Manifest APIs to get a description of the Appx Bundle's identity and payload packages.


Prerequisites
=============

     This sample requires Windows 8.1 or higher, and an installation of Visual Studio 2012 Ultimate.


Sample Language Implementations
===============================

     This sample is available in the following language implementations:

     C++


Files
=====

     DescribeBundle.cpp - main entry point for the sample application

     DescribeBundle.h - main header file

     DescribeBundle.vcxproj - build configuration for this sample

     DescribeBundle.sln - Visual Studio 2012 Solution file for this sample

     Data\sample.appxbundle - A sample Appx Bundle containing the manifest to be read


To build the sample using the command prompt:
=============================================

     1. Open the Command Prompt window and navigate to the directory containing DescribeBundle.vcxproj

     2. Type: msbuild DescribeBundle.vcxproj


To build the sample using Visual Studio 2012 Ultimate (preferred method):
=========================================================================

     1. Open File Explorer and navigate to the directory containing DescribeBundle.vcxproj.

     2. Double-click the icon for the DescribeBundle.vcxproj file to open the file in Visual Studio.

     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================

     1. Open a command prompt window and navigate to the directory where DescribeBundle.exe is built.

     2. Type the command:

             DescribeBundle.exe inputFile

        inputFile: path to an Appx Bundle that can be read, for example "Data\sample.appxbundle"
