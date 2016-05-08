//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  RootDSE VC Sample: Retrieving the Distinguished Name
//                     of a Domain Partition
//
//--------------------------------------------------------------------------

Description
===========
The RootDSE sample binds to rootDSE of an Active Directory server using the
IADs interface and retrieves the defaultNamingContext property, which contains
the distinguished name of the domain partition, as well as other naming
contexts for the domain.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============

  *  RootDSE.cpp
  *  RootDSE.sln
  *  RootDSE.vcproj
  *  makefile
  *  readme.txt
  *  stdafx.cpp
  *  stdafx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution RootDSE.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built the
      sample.
  2.  Type the command "RootDSE.exe".

Example Output
==============
Typical output from the sample follows for the Fabrikam.Com domain.

Default Naming Context:DC=fabrikam,DC=com

Root Domain Naming Context:DC=fabrikam,DC=com

Configuration Naming Context :CN=Configuration,DC=fabrikam,DC=com

Schema Naming context :CN=Schema,CN=Configuration,DC=fabrikam,DC=com

How the Sample Works
====================
The sample uses the ADsGetObject function to get a pointer to an IADs
interface to the root of the directory information tree of the directory
server (rootDSE) and then uses that interface to print the following
properties.
  *  defaultNamingContext
  *  rootDomainNamingContext
  *  configurationNamingContext
  *  schemaNamingContext

See Also
========
IADs interface
Serverless Binding and RootDSE

