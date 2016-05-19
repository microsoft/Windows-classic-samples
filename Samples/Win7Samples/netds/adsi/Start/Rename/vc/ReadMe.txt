//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Rename VC Sample: Renaming an Object Using ADSI
//
//--------------------------------------------------------------------------

Description
===========
The Rename sample renames an Active Directory Object.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  makefile
  *  Rename.cpp
  *  Rename.sln
  *  Rename.vcproj
  *  StdAfx.Cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

You must run this sample in a Windows domain where you have permission to
write to Active Directory

To build this sample
  1.  Open the solution Rename.sln.
  2.  Open the source file Rename.Cpp.
  3.  Replace the domain name with an existing one, such as "DC=Fabrikam,
      DC=Com" in the following line.
        hr = ADsGetObject(L"LDAP://CN=Users,DC=Microsoft,DC=COM",  \
                          IID_IADsContainer, (void**) &pCont);
  4.  Create a user named "Jeff Smith" in the domain and change the
      domain name to the one specified in Step 3 in the following line.
        hr = pCont->MoveHere(L"LDAP://CN=Jeff Smith,CN=Users,  \
                      DC=Microsoft,DC=COM",L"CN=Jeffrey Smith", &pDisp );
  5.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Rename.exe".

You can also run the sample by selecting Execute Rename.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, there is no output.  You can verify
the result by opening and examining the Active Directory Users and Computers
snap-in of the Microsoft Management Console.

How the Sample Works
====================
The sample uses the LDAP: ADsPath to perform the bindings and the
IADsContainer interface to perform the rename.

See Also
========
IADsContainer interface
LDAP ADsPath
LDAP Binding String (ADsPath)

