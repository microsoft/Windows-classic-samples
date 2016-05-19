//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  WkGuid VC Sample: Using Well-Known GUIDs
//
//--------------------------------------------------------------------------

Description
===========
The WkGuid sample uses the well-known GUID of the Users container to bind
to the Users container in the default naming context.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  UsersContainer.sln
  *  UsersContainer.vcproj
  *  WkGuid.cpp
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution UsersContainer.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "UsersContainer.exe".

You can also run the sample by selecting Execute UsersContainer.exe from
the Build menu.

Example Output
==============
Running the sample produces the following output when run in the Fabrikam.Com
domain.

This program finds the User's container in the current Window 2000 domain
It uses the WKGUID binding string format.

ADsPath of Users Container (using WKGUID binding format):
LDAP://<WKGUID=a9d1ca15768811d1aded00c04fd8d5cd,DC=fabrikam,DC=com>

Name of Users Container (using WKGUID binding format):
<WKGUID=a9d1ca15768811d1aded00c04fd8d5cd,DC=fabrikam,DC=com>

Name of Users Container: CN=Users,DC=fabrikam,DC=com

How the Sample Works
====================
The sample defines two functions.

  wmain
    Initializes, calls the GetWKDomainObject function, and prints
    out the ADsPath, name in WKGUID format, and common name of the
    Users container.
  GetWKDomainObject
    Gets the specified well-know object for the current domain.

You can also use well-known GUIDs to bind to other well-known containers,
such as the Computers container.  The complete list includes:

  *  GUID_USERS_CONTAINER_W
  *  GUID_COMPUTRS_CONTAINER_W
  *  GUID_SYSTEMS_CONTAINER_W
  *  GUID_DOMAIN_CONTROLLERS_CONTAINER_W
  *  GUID_INFRASTRUCTURE_CONTAINER_W
  *  GUID_DELETED_OBJECTS_CONTAINER_W
  *  GUID_LOSTANDFOUND_CONTAINER_W

See Also
========
IADs interface
Binding to Well-Known Objects Using WKGUID

