//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Binding VC Sample: Binding with Current and Alternate Credentials
//
//--------------------------------------------------------------------------

Description
===========
The Binding sample demonstrates how to bind to a domain using the credentials
of the currently logged-on user and with credentials of a specified user.

This sample uses the WinNT: provider and is suitable for Windows NT(R) 4.0
networks as well as Windows 2000 and later networks running Active Directory.

Sample Files
============
  *  Binding.cpp
  *  Binding.sln
  *  Binding.vcproj
  *  makefile
  *  StdAfx.cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Binding.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Binding.exe".

You can also run the sample by selecting Execute Binding.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, there is no output.

How the Sample Works
====================
The sample uses the WinNT ADsPath to perform the binding and the
IADs interface to bind with the credentials of the specified
user.

See Also
========
IADs interface
WinNT ADsPath
WinNT Binding String (ADsPath)

