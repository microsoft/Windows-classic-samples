//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Child VC Sample: Binding to a Child in an Active Directory Container
//
//--------------------------------------------------------------------------

Description
===========
The Child sample demonstrates binding to a child object from a container
using LDAP binding.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  Child.cpp
  *  Child.sln
  *  Child.vcproj
  *  makefile
  *  StdAfx.cpp
  *  StdAfx.h
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Child.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Child.exe".

You can also run the sample by selecting Execute Child.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, there is no output.

How the Sample Works
====================
The sample uses the LDAP: ADsPath to perform the bindings and the
IADsContainer interface to perform the binding.

See Also
========
IADsContainer interface
LDAP ADsPath
LDAP Binding String (ADsPath)

