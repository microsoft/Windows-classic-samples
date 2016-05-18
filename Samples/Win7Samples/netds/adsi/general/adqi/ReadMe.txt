//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  ADQI VC Sample: ADSI Query Interface
//
//--------------------------------------------------------------------------

Description
===========
The ADQI sample lists all supported ADSI interfaces for a given ADsPath.
For selected supported interfaces, the sample allows you to run some of the
object methods.

Sample Files
============
  *  ADQI.Cpp
  *  ADQI.sln
  *  ADQI.vcproj
  *  ADQI.h
  *  ADQI.Rc
  *  ADQIDlg.Cpp
  *  ADQIDlg.h
  *  ADs.Cpp
  *  ADs.h
  *  adsContainer.Cpp
  *  adsContainer.h
  *  ADsLargeInteger.Cpp
  *  ADsLargeInteger.h
  *  ADsOpenDSObject.Cpp
  *  ADsOpenDSObject.h
  *  ADsPropertyList.Cpp
  *  ADsPropertyList.h
  *  DirectoryObject.Cpp
  *  DirectoryObject.h
  *  DirectorySearch.Cpp
  *  DirectorySearch.h
  *  Helper.Cpp
  *  Helper.h
  *  makefile
  *  res\ADQI.ico
  *  res\ADQI.Rc2
  *  Resource.h
  *  Security.Cpp
  *  Security.h
  *  StdAfx.Cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution ADQI.sln.
  2.  From the Build menu, select Set Active Configuration
  3.  In the resulting Set Active Project Configuration, select
      Win32 Release, and then select OK.
  4.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "ADQI.exe".
  3.  The Active Directory Service Interfaces dialog appears in which
      you enter an ADsPath to Active Directory.  After you enter an
      ADsPath, select OK.
  4.  A list of interfaces that selected provider supports appears in the
      Supported Interfaces frame.  Double click one of the supported
      interfaces.
  5.  A dialog, which depends on the selected interface, appears that
      provides details about the selected interface.  You can experiment
      with different interfaces to find out about attributes, queries,
      and so forth that relate to the selected interface.

You can also run the sample by selecting Execute ADQI.exe from
the Build menu.

How the Sample Works
====================
The sample uses the selected ADsPath to bind to Active Directory and then
it gets a pointer to an IUnknown interface for the Active Directory object
of the sample.  With this, it enumerates all known ADSI interfaces and adds
the supported ones to the supported interfaces list.

When you select a supported interface, the sample displays an interface-
dependent dialog.  Only the following interfaces are handled by the present
version of the sample.
  *  IADs
  *  IADsContainer
  *  IADsLargeInteger
  *  IADsPropertyList
  *  IADsOpenDSObject
  *  IADsSecurityDescriptor
  *  IDirectoryObject
  *  IDirectorySearch

As an exercise, you could add code for an unhandled interface.

See Also
========
ADSI Service Providers
IADs interface
IADsContainer interface
IADsLargeInteger interface
IADsPropertyList interface
IADsOpenDSObject interface
IADsSecurityDescriptor interface
IDirectoryObject interface
IDirectorySearch interface
