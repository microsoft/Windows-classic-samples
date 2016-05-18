//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Credentials VC Sample: Binding With Alternate Credentials
//
//--------------------------------------------------------------------------

Description
===========
The Credentials sample uses ADSI and the LDAP provider to bind to Active
Directory using specified credentials rather than using the default
credentials of the logged-on user.

The sample allows the user to specify alternate credentials for the current
domain by using a user name that can be:
  1.  NT 4.0 Style name such as MYDOMAIN\MyUser.
  2.  Windows 2000 User Principal Name such as myUser@mydomain.com.
  3.  User name without the domain name such as Administrator.
      In this case, it assumes the domain name is the current domain.
  4.  Distinguished Name such as CN=Administrator,DC=Fabirkam,DC=Com.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  Credentials.cpp
  *  Credentials.sln
  *  Credentials.vcproj
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
  1.  Open the solution Credentials.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Credentials.exe".
  3.  At the username prompt, enter the user name for the alternative
      credentials to use.
  4.  At the password prompt, enter the password for the alternative
      user name.

Example Output
==============
Entering the command "Credentials.exe" and responding to the prompts
produces the following output.

Domain Name is :DC=fabrikam,DC=com

username:FABRIKAM\Administrator

password:<password>
Successful logon!

How the Sample Works
====================
The sample uses the IADsOpenDSObject interface to specify the credentials.

See Also
========
IADsOpenDSObject interface
ADsOpenObject and IADsOpenDSObject::OpenDSObject

