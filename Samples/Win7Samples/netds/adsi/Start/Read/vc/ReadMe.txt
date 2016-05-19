//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Read VC Sample: Reading Attributes Using ADSI
//
//--------------------------------------------------------------------------

Description
===========
The Read sample reads various single-valued and multi-valued properties of
an Active Directory object.

This sample uses the WinNT: and LDAP: providers and is suitable for Windows
2000 and later networks running Active Directory.

Sample Files
============
  *  makefile
  *  Read.cpp
  *  Read.sln
  *  Read.vcproj
  *  StdAfx.cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Read.sln.
  2.  Open the source file Read.Cpp.
  3.  Replace the domain name "INDEPENDENCE" with an appropriate domain name,
      such as "FABRIKAM", in the following line.
        hr = ADsGetObject(L"WinNT://INDEPENDENCE/Administrator,user",  \
                          IID_IADs, (void**) &pUsr );
  4.  Replace the domain name "INDEPENDENCE" and the computer name "ANDYHAR11"
      with appropriate ones, such as "FABRIKAM/FABRIKAMDC, in the following line.
        hr = ADsGetObject(L"WinNT://INDEPENDENCE/ANDYHAR11/Browser,service",  \
                          IID_IADs, (void**) &pSvc );
  5.  Replace the domain name "DC=testDom1,DC=testDom2,DC=microsoft,DC=com"
      with an appropriate one, such as "DC=Fabrikam,DC=Com", in the following line.
        hr = ADsGetObject(L"LDAP://CN=Administrator,CN=Users,  \
                            DC=testDom1,DC=testDom2,DC=microsoft,DC=com",  \
                            IID_IDirectoryObject,  \
                            (void**) &pDirObject );
  6.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Read.exe".

You can also run the sample by selecting Execute Read.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, it prints the output similar to the
following in a command window.

FullName: Jeff Smith
Getting service dependencies using IADs :
LanmanWorkstation
Getting the objectClass multivalue attribute using IDirectoryObject :
  top
  person
  organizationalPerson
  user

How the Sample Works
====================
The sample uses the WinNT ADsPath and the LDAP ADsPath to perform bindings
and the IADs and IDirectoryObject interfaces to read the attributes.

See Also
========
IADs interface
IDirectoryObject interface
LDAP ADsPath
LDAP Binding String (ADsPath)
WinNT ADsPath
WinNT Binding String (ADsPath)

