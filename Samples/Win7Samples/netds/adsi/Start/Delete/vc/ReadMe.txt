//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Delete VC Sample: Deleting User Accounts
//
//--------------------------------------------------------------------------

Description
===========
The Delete sample uses the WinNT: provider and the IADsContainer inteface
to delete a local user object.  It also uses the LDAP provider and the
IDirectoryObject interface to delete a user account from Active Directory.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  Delete.cpp
  *  Delete.sln
  *  Delete.vcproj
  *  makefile
  *  readme.txt
  *  StdAfx.Cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Delete.sln.
  2.  Open the source file Delete.Cpp.
  3.  Replace the domain name "INDEPENDENCE" with an appropriate domain name,
      such as "FABRIKAM", in the following line.
        hr = ADsGetObject(L"WinNT://INDEPENDENCE",  \
                          IID_IADsContainer, (void**) &pCont);
  4.  Create (or ensure that one exists) a user named "AliceW" in the domain.
  5.  Replace the organiztional unit name and domain name with an appropriate
      organizational unit name and domain name, such as "OU=Example Org Unit,
      DC=fabrikam,DC=com", in the following line.
      hr = ADsGetObject(L"LDAP://OU=testOU,DC=testDom1,DC=testDom2,  \
                            DC=microsoft,DC=com",IID_IDirectoryObject,  \
                            (void**) &pDirObject );
  6.  Create (or ensure that one exists) a user named "Mike Smith" in the domain.
  7.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Delete.exe".

You can also run the sample by selecting Execute Delete.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, there is no output.

The sample deletes users "AliceW" and "Mike Smith".  You can verify the
result by opening and examining the Active Directory Users and Computers
snap-in of the Microsoft Management Console.

How the Sample Works
====================
The sample uses the WinNT ADsPath to perform the first binding and the
IADsContainer interface to delete the user "AliceW".  It uses the LDAP
ADsPath and the IDirectoryObject interface to delete the user "Mike Smith".

See Also
========
IADsContainer interface
IDirectoryObject interface
LDAP ADsPath
LDAP Binding String (ADsPath)
WinNT ADsPath
WinNT Binding String (ADsPath)

