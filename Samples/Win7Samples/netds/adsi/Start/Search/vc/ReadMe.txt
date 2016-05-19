//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Search VC Sample: Searching Active Directory
//
//--------------------------------------------------------------------------

Description
===========
The Search sample uses the IDirectorySearch interface to search Active
Directory for objects that match a specified filter.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  makefile
  *  Search.cpp
  *  Search.sln
  *  Search.vcproj
  *  StdAfx.cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Search.sln.
  2.  Open the source file Search.Cpp.
  3.  Replace the domain name with an existing one, such as "DC=Fabrikam,
      DC=Com" in the following line.
        hr = ADsGetObject(L"LDAP://DC=testDom1,DC=testDom2,  \
                          DC=microsoft,DC=com",  \
                          IID_IDirectorySearch,  \
                          (void**) &pSearch );
  4.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Search.exe".

You can also run the sample by selecting Execute Search.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, it prints output similar to the following
in a command window.

HelpServicesGroup
Administrators
Users
Guests
Print Operators
Backup Operators
Replicator
Remote Desktop Users
Network Configuration Operators
Domain Computers
Domain Controllers
Schema Admins
Enterprise Admins
Cert Publishers
Domain Admins
Domain Users
Domain Guests
Group Policy Creator Owners
RAS and IAS Servers
Server Operators
Account Operators
Pre-Windows 2000 Compatible Access
Incoming Forest Trust Builders
DnsAdmins
DnsUpdateProxy
DHCP Users
DHCP Administrators
Win2K Group Name
My New Group

How the Sample Works
====================
The sample uses the LDAP: ADsPath to perform the bindings and the
IDirectorySearch interface to perform the search with the LDAP
syntax filter "(objectCategory=Group).

See Also
========
IDirectorySearch interface
LDAP ADsPath
LDAP Binding String (ADsPath)

