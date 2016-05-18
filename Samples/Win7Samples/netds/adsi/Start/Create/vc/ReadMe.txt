//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Create VC Sample: Creating a Local User Account
//
//--------------------------------------------------------------------------

Description
===========
The Create sample creates a new local user account.  It includes a
function that uses the IADsContainer interface with the WinNT provider
to create a local user account.  It also has an optional funciton that
uses the IDirectoryObject interface with the LDAP provider to create a
domain user account in Active Directory.

This sample uses the WinNT: provider and is suitable for Windows NT(R) 4.0
networks as well as Windows 2000 and later networks running Active Directory.
If you use the optional LDAP: provider subroutine, it is suitable for Windows
2000 and later networks running Active Directory.

Sample Files
============
  *  Create.cpp
  *  Create.sln
  *  Create.vcproj
  *  makefile
  *  StdAfx.Cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Create.sln.
  2.  Open the source file Create.Cpp.
  3.  Replace the domain name "seyitb-dev" with an appropriate domain name,
      such as "FABRIKAM", in the following line.
        hr = ADsGetObject( L"WinNT://seyitb-dev",  \
                           IID_IADsContainer, (void**) &pCont );
  4.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Create.exe".

You can also run the sample by selecting Execute Create.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, it prints the following output to
a command window.

User created successfully
Result: 0x0

The sample creates a user "AliceW" with the password "MysEcret1".  You
can verify the result by opening and examining the Active Directory Users
and Computers snap-in of the Microsoft Management Console.

How the Sample Works
====================
The sample uses the WinNT ADsPath to perform the bindings and the
IADsContainer and IADsUser interfaces to create the user and set the
password in the CreateUserIADsContainer function.

The optional CreateUserIDirectoryObject function uses the LDAP: provider
and the IDirectoryObject interface to perform a similar task.

See Also
========
Creating a User
IADsContainer interface
IADsUser interface
IDirectoryObject interface
LDAP ADsPath
LDAP Binding String (ADsPath)
WinNT ADsPath
WinNT Binding String (ADsPath)

