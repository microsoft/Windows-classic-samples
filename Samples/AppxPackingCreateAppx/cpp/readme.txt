
Appx Package Creation Sample
============================

     This sample demonstrates how to use Appx packaging APIs to produce an Appx package from a collection of files on disk.

     The package created using data files supplied with this sample is a valid Windows Modern Application.  It can be installed in Windows 8 or above if it is signed with a valid developer test certificate.


Prerequisites
=============

     This sample requires Windows 8 or higher, and an installation of Visual Studio 2012 Ultimate.


Sample Language Implementations
===============================

     This sample is available in the following language implementations:

     C++


Files
=====

     CreateAppx.cpp - main entry point for the sample application

     CreateAppx.h - main header file

     CreateAppx.vcxproj - build configuration for this sample

     CreateAppx.sln - Visual Studio 2012 Solution file for this sample

     Data\AppxManifest.xml - A sample Appx package manifest for the package to be produced

     Data\AppTile.png
     Data\Default.html
     Data\Error.html
     Data\images\smiley.jpg - Sample data (payload) files for the package to be produced


To build the sample using the command prompt:
=============================================

     1. Open the Command Prompt window and navigate to the directory containing CreateAppx.vcxproj

     2. Type: msbuild CreateAppx.vcxproj


To build the sample using Visual Studio 2012 Ultimate (preferred method):
=========================================================================

     1. Open File Explorer and navigate to the directory containing CreateAppx.vcxproj.

     2. Double-click the icon for the CreateAppx.vcxproj file to open the file in Visual Studio.

     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================

     1. Open a command prompt window and navigate to the directory containing the Data folder for this sample.

     2. Type the command:

             <path>\CreateAppx.exe

        where <path> is the full or relative path to where CreateAppx.exe is built.  If CreateAppx.exe is built using Visual Studio, then <path> is usually "Debug" or "Release" depending on the configuration used.

     3. When the application exits successfully, an Appx package named "HelloWorld.appx" should be created.
