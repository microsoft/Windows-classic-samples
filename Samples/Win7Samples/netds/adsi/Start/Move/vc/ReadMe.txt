//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Move VC Sample: Moving an Object Using ADSI
//
//--------------------------------------------------------------------------

Description
===========
The Move sample moves an object from one container to another by moving
a user from one organizational unit to another.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  makefile
  *  Move.cpp
  *  Move.sln
  *  Move.vcproj
  *  StdAfx.cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

You must run this sample in a Windows domain where you have permission to
write to Active Directory

To build this sample
  1.  Open the solution Move.sln.
  2.  Open the source file Move.Cpp.
  3.  Replace the destination organizational unit and the domain with
      existing ones, such as "OU=Another Example Org Unit,DC=Fabrikam,
      DC=Com" in the following line.
        hr = ADsGetObject(L"LDAP://OU=trOU,  \
                          DC=domain1,DC=domain2,DC=microsoft,DC=com",
  4.  Create a user named "Mike Smith" in a source organizational
      unit, such as "OU=Example Org Unit", in the domain and change the
      organizational unit and domain in the following line.
        hr = pCont->MoveHere(L"LDAP://CN=Mike Smith,OU=srOU,  \
               DC=domain1,DC=domain2,DC=microsoft,DC=com", NULL, &pDisp );
  5.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Move.exe".

You can also run the sample by selecting Execute Move.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, there is no output.  You can verify
the result by opening and examining the Active Directory Users and Computers
snap-in of the Microsoft Management Console.

How the Sample Works
====================
The sample uses the LDAP: ADsPath to perform the bindings and the
IADsContainer interface to perform the move.

See Also
========
IADsContainer interface
LDAP ADsPath
LDAP Binding String (ADsPath)

