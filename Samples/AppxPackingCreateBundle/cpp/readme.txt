
Appx Bundle Creation Sample
===========================

     This sample demonstrates how to use Appx Bundle APIs to produce an Appx Bundle from a collection of packages on disk.

     The bundle created using data packages supplied with this sample is a valid Windows Modern Application Bundle.  It can be installed in Windows 8.1 or above if it is signed with a valid developer test certificate.


Prerequisites
=============

     This sample requires Windows 8.1 or higher, and an installation of Visual Studio 2012 Ultimate.


Sample Language Implementations
===============================

     This sample is available in the following language implementations:

     C++


Files
=====

     CreateBundle.cpp - main entry point for the sample application

     CreateBundle.h - main header file

     CreateBundle.vcxproj - build configuration for this sample

     CreateBundle.sln - Visual Studio 2012 Solution file for this sample

     Data\MainAppPackage.appx
     Data\ResourcePackage.lang-de.appx - Sample data (payload) packages for the bundle to be produced


To build the sample using the command prompt:
=============================================

     1. Open the Command Prompt window and navigate to the directory containing CreateBundle.vcxproj

     2. Type: msbuild CreateBundle.vcxproj


To build the sample using Visual Studio 2012 Ultimate (preferred method):
=========================================================================

     1. Open File Explorer and navigate to the directory containing CreateBundle.vcxproj.

     2. Double-click the icon for the CreateBundle.vcxproj file to open the file in Visual Studio.

     3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


To run the sample:
==================

     1. Open a command prompt window and navigate to the directory containing the Data folder for this sample.

     2. Type the command:

             <path>\CreateBundle.exe

        where <path> is the full or relative path to where CreateBundle.exe is built.  If CreateBundle.exe is built using Visual Studio, then <path> is usually "Debug" or "Release" depending on the configuration used.

     3. When the application exits successfully, an Appx Bundle named "sample.appxbundle" should be created.
