THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999  Microsoft Corporation.  All Rights Reserved.

SDKDiff Sample
-------------------------------

Abstract:

 SDKDiff allows the users to compare two files or two directories 
 against each other, and display the differences found between the 
 files or directories on the screen. 

 The differences are displayed textually and graphically.  Graphically,
 data that exists in the first file but does not exist in the second file 
 is represented by a red line, whereas data that does not exist in the first
 file but exists in the second file is represented by yellow line.
 The identical parts of the files are displayed in black line.  The 2 lines, one
 for each file, made up of different colors based on blocks of identical or 
 different blocks of data in the files.

 Both files are displayed simultaneously.  They are virtually mapped to one file, 
 where the lines that only appear in the first file are highlighted in red, and
 the lines that only appear in the left file are highlighted in yellow.
  
Supported OS:
 Windows 2000, Windows XP

Building:
 Build the sample using the latest Platform SDK via the MAKEFILE included. 
 For building in Visual Studio, use the sdkdiff.vcproj for Visual Studio.NET and
 sdkdiff.dsp for Visual Studio 6.0.

Usage:
 The sample can be run directly on the command-line by typing 'sdkdiff'.  
 This starts the sdkdiff program, and the files or directories to be compared can
 be chosen via the menus in the program: File->Compare Files or File->Compare Directories.
 
Special Note for 64-bit Build Environments:

This sample builds a binary sample and an associated help file. In some 64-bit 
environments, NMAKE is unable to find the help compiler to build the help file 
and will fail. As a result, help-related functions in this sample will be unable 
to find the help file. To resolve this problem, build the sample in a 32-bit 
environment and then copy the help file to the output directory of the 64-bit 
build environment. Alternatively, add the path of the HCRTF.EXE tool (located 
in the installed Visual Studio distribution) to the "PATH" environment variable.

