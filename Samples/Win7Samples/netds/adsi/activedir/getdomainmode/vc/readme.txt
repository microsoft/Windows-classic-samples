//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  GetDomainmode VC Sample: Determining the Operation Mode of a Domain
//
//--------------------------------------------------------------------------

Description
===========
The GetDomainmode sample binds to an Active Directory domain partition
and reads the ntMixedDomain property to determine the operation mode
of the domain.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  GetDomainmode.cpp
  *  GetDomainmode.sln
  *  GetDomainmode.vcproj
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution GetDomainmode.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "GetDomainmode.exe".

You can also run this sample by selecting the Execute GetDomainmode.exe
command in the Build menu.

Example Output
==============
Successful execution of the sample produces one of the following.

This program checks whether the current domain is in mixed or native mode.
Current domain fabrikam is in mixed mode

-Or-

This program checks whether the current domain is in mixed or native mode.
Current domain fabrikam is in native mode

If an error occurs, an error message appears instead.

How the Sample Works
====================
The sample defines the following functions.

  main
    Initializes using the IADs interface, calls the GetDomainMode function,
    uninitializes.
  GetDomainMode
    Uses the IADs interfaces to get the ntMixedDomain attribute.

See Also
========
IADs interface
Detecting the Operation Mode of a Domain

