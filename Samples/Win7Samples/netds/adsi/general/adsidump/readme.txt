//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  ADSIDump VC Sample: ADSI Dump of ADsPath
//
//--------------------------------------------------------------------------

Description
===========
The ADSIDump sample traverses the Active Directory tree and dumps a listing
of all objects and their attributes for the specified ADsPath to a file.

Sample Files
============
  *  ADSIDump.cpp
  *  ADSIDump.sln
  *  ADSIDump.vcproj
  *  StdAfx.cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution ADSIDump.sln.
  2.  From the Build menu, select Build.


Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command

        ADSIDump <ADsPath> <file_name>

      where
        ADsPath   specifies an LDAP binding string.
        file_name specifies a file to which to print the results.

Example Output
==============
Entering the command

  ADSIDump CN=Administrator,CN=Users,DC=Fabrikam,DC=Com

in the Fabrikam.Com domain produces output similar to the following
(much of the output is deleted to save space).

=======================================================

***  CN=Administrator  ***
  ROOT OBJECT
  Full ADs path: "LDAP://CN=Administrator,CN=Users,DC=Fabrikam,DC=Com"
  Class: user
  Schema: LDAP://schema/user
  Attributes -------------------
    cn : (BSTR) "Administrator"
    instanceType : (INT) 4
    nTSecurityDescriptor : (Unknown variant type)
    objectCategory : (BSTR) "CN=Person,CN=Schema,CN=Configuration,DC=fabrikam,DC=com"
    objectClass : (ARRAY) [  "top"  "person"  "organizationalPerson"  "user"]
    objectSid : (Unknown variant type)
    ...
    description : (BSTR) "Built-in account for administering the computer/domain"
    ...
    memberOf : (ARRAY) [  "CN=Group Policy Creator Owners,CN=Users,DC=fabrikam,DC=com"  "CN=Domain Admins,CN=Users,DC=fabrikam,DC=com"  "CN=Enterprise Admins,CN=Users,DC=fabrikam,DC=com"  "CN=Schema Admins,CN=Users,DC=fabrikam,DC=com"  "CN=Administrators,CN=Builtin,DC=fabrikam,DC=com"]
    ...

How the Sample Works
====================
The sample uses the selected ADsPath to bind to Active Directory and then,
starting at the selected object, it traverses the directory tree, printing
each object and its attributes to the specified file.

See Also
========
LDAP ADsPath
IADs interface

