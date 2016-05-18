//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Write VC Sample: Setting Properties Using ADSI
//
//--------------------------------------------------------------------------

Description
===========
The Write sample shows how to set single-valued and multi-valued attributes
of an Active Directory object.  The sample sets attributes using both the
IADs and IDirectoryObject interfaces.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  makefile
  *  StdAfx.cpp
  *  StdAfx.h
  *  Write.cpp
  *  Write.sln
  *  Write.vcproj

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

You must run this sample in a Windows domain where you have permission to
write to Active Directory.

To build this sample
  1.  Open the solution Write.sln.
  2.  Open the source file Write.cpp.
  3.  Replace the domain name, organizational unit, and user name with
      appropriate values in the following line.
        LPWSTR pszADsPath = L"LDAP://CN=Jane Johnson,OU=testOU,  \
                            DC=testDom1,DC=testDom2,DC=microsoft,DC=com";
  4.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Write.exe".

You can also run the sample by selecting Execute Write.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, there is no output.

The sample sets and commits the properites of the specified user.  You can
verify the result by opening and examining the Active Directory Users and
Computers snap-in of the Microsoft Management Console.

How the Sample Works
====================
The sample uses the LDAP ADsPath to perform bindings and the IADs and
IDirectoryObject interfaces to write the attributes.

See Also
========
IADs interface
IDirectoryObject interface
LDAP ADsPath
LDAP Binding String (ADsPath)


