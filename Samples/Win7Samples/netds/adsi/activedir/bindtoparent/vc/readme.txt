//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  BindtoParent VC Sample: Binding to an Object's Parent
//
//--------------------------------------------------------------------------

Description
===========
The BindtoParent sample uses ADSI to bind to the local domain partition in
Active Directory.  It then searches for a specified user object.  When it
finds the object, the sample binds to the parent of that object, which is the
container of the user object in the Active Directory hierarchy.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  BindtoParent.Cpp
  *  BindtoParent.sln
  *  BindtoParent.vcproj
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the workspace BindtoParent.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command

        BindtoParent.exe "<username>"

      where <username> is the Common Name of an existing user.

Alternatively, you can enter just the command "BindtoParent.exe" at the
command prompt, and the sample will prompt you for the Common Name.

Example Output
==============
The following output results from entering the Common Name of the existing
user "Guest" in the Fabrikam.Com domain.

This program finds a user in the current Windows 2000 domain
and displays its parent container's ADsPath and binds to that container.
Enter Common Name of the user to find:Guest

Finding user: Guest...
Found User Guest.
ADsPath: LDAP://CN=Guest,CN=Users,DC=fabrikam,DC=com
Successfully bound to parent container
ADsPath: LDAP://CN=Users,DC=fabrikam,DC=com
DN: CN=Users,DC=fabrikam,DC=com
Class: container

How the Sample Works
====================
The sample defines the following functions.

  wmain
    Determines the user name, initializes COM, calls the other functions,
    and uninitializes.
  FindUserByName
    Uses the IDirectorySearch interface to find the selected user.
  GetParentObject
    Uses the IADs interface to get the parent object of the selected user.

See Also
========
IADs interface
IDirectorySearch interface
Searching with IDirectorySearch

