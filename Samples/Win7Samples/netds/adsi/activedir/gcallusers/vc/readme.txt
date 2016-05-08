//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  GCallUsers VC Sample: Searching the Global Catalog
//
//--------------------------------------------------------------------------

Description
===========
The GCallUsers sample searches the global catalog to enumerate users in
an Active Directory forest.

The sample uses the IDirectorySearch interface to perform the search.

This sample uses the GC: provider and is suitable for Windows 2000 and later
networks running Active Directory.

Sample Files
============
  *  GCallUsers.cpp
  *  GCallUsers.sln
  *  GCallUsers.vcproj
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution GCallUsers.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "GCallUsers.exe".

You can also run this sample by selecting the Execute GCallUsers.exe
command in the Build menu.

Example Output
==============
Executing the command "GCallUsers.exe" produces output similar to the
following.

This program finds all users in the forest
by searching the global catalog.
------------------------------
cn: Administrator
distinguishedName: CN=Administrator,CN=Users,DC=fabrikam,DC=com
------------------------------
cn: Guest
distinguishedName: CN=Guest,CN=Users,DC=fabrikam,DC=com
------------------------------
cn: Test User
distinguishedName: CN=Test User,CN=Users,DC=fabrikam,DC=com
------------------------------

How the Sample Works
====================
The sample defines the following functions.

  main
    Initializes, calls the FindAllUsersInGC function, uninitializes.
  FindAllUsersInGC
    Uses the IADs, IADsContainer, and IDirectorySearch interfaces to
    bind to the global catalog, construct the search filter, execute
    the search, and print out the selected attributes for found users.

See Also
========
IADs interface
IADsContainer interface
IDirectorySearch interface
Searching with IDirectorySearch

