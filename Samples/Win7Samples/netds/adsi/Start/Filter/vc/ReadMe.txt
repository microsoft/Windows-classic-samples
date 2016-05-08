//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Filter VC Sample: Filtering a Container's Objects with ADSI
//
//--------------------------------------------------------------------------

Description
===========
The Filter sample enumerates selected objects in a container, printing the
name and class of the filtered objects in a specified WinNT domain.

This sample uses the WinNT: provider and is suitable for Windows NT(R) 4.0
networks as well as Windows 2000 and later networks running Active Directory.

Sample Files
============
  *  Filter.cpp
  *  Filter.sln
  *  Filter.vcproj
  *  makefile
  *  StdAfx.cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Filter.sln.
  2.  Open the source file Filter.Cpp.
  3.  Replace the domain name "INDEPENDENCE" with an appropriate domain name,
      such as "FABRIKAM", in the following line.
        hr = ADsGetObject(L"WinNT://INDEPENDENCE",  \
                          IID_IADsContainer, (void**) &pCont );
  4.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Filter.exe".

You can also run the sample by selecting Execute Filter.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, it prints output similar to the following
in a command window.

Administrator           (User)
Guest           (User)
Helper          (User)
TestU           (User)
DnsUpdateProxy          (Group)
Domain Admins           (Group)
Domain Computers                (Group)
Domain Controllers              (Group)
Domain Guests           (Group)
Domain Users            (Group)
Downlevel Name          (Group)
Enterprise Admins               (Group)
Group Policy Creator Owners             (Group)
NewGroup                (Group)
Schema Admins           (Group)
Administrators          (Group)
Users           (Group)
Guests          (Group)
Print Operators         (Group)
Backup Operators                (Group)
Replicator              (Group)
Remote Desktop Users            (Group)
Network Configuration Operators         (Group)
Server Operators                (Group)
Account Operators               (Group)
Pre-Windows 2000 Compatible Access              (Group)
Incoming Forest Trust Builders          (Group)
Cert Publishers         (Group)
RAS and IAS Servers             (Group)
HelpServicesGroup               (Group)
DnsAdmins               (Group)
DHCP Users              (Group)
DHCP Administrators             (Group)

How the Sample Works
====================
The sample uses the WinNT ADsPath to perform the binding and the
IADsContainer interface to filter (only user and group objects) and
enumerate the objects in the domain.

See Also
========
IADsContainer interface
WinNT ADsPath
WinNT Binding String (ADsPath)

