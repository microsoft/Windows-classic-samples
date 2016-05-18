//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  QueryUsers VC Sample: Searching for Users in Active Directory
//
//--------------------------------------------------------------------------

Description
===========
The QueryUsers sample queries an Active Directory domain partition for user
objects that match a specified filter.

The sample uses the IDirectorySearch interface to perform the search.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============

  *  QueryUsers.cpp
  *  QueryUsers.sln
  *  QueryUsers.vcproj
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution QueryUsers.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built the
      sample.
  2.  Type the command

        QueryUsers [/V] [<querystring>]

      where
        /V specifies to return all properties for the found users.
        <querystring> is the query criteria in LDAP query format.

If you don't specify /V, the query returns only the  DN of the items found.
If you don't specify <querystring>, the query returns all users.

Example Output
==============
Typical output from the sample follows.  Some of the output is deleted to
conserve space.

Entering the command
  QueryUsers
produces output similar to the following for the Fabrikam.Com domain.

Finding all user objects...

Administrator
  DN: CN=Administrator,CN=Users,DC=fabrikam,DC=com

Guest
  DN: CN=Guest,CN=Users,DC=fabrikam,DC=com

A Helper
  DN: CN=A Helper,CN=Users,DC=fabrikam,DC=com

Test User
  DN: CN=Test User,CN=Users,DC=fabrikam,DC=com

Entering the command
  QueryUsers (sn=User)
produces output similar to the following for the Fabrikam.Com domain.

Finding user objects based on query: (sn=User)...

Test User
  DN: CN=Test User,CN=Users,DC=fabrikam,DC=com

How the Sample Works
====================
The sample defines the following functions.

  wmain
    Interprets the commmand-line parameters, initializes COM, uses the IADs
    interface to get the Naming Context and bind to the domain controller,
    calls the FindUsers function, and uninitializes.
  FindUsers
    Uses the IDirectorySearch interface to set search preferences, execute
    the search, and retrieve the results.

See Also
========
IADs interface
IDirectorySearch interface
Searching with IDirectorySearch

