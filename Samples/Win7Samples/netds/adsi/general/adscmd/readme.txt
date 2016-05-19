//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  ADsCmd VC Sample: ADSI Command-Line Browser
//
//--------------------------------------------------------------------------

Description
===========
The ADsCmd sample uses a specified ADsPath to bind to a directory object and
then either lists all its contained objects or dumps all the properties of
the object.

Sample Files
============
  *  ADsCmd.sln
  *  ADsCmd.vcproj
  *  Dump.cpp
  *  Enum.cpp
  *  Main.cpp
  *  Main.h
  *  sources
  *  Util.cpp

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the workspace ADsCmd.cpp.
  2.  From the Build menu, select Build.



Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command

        ADsCmd [list|dump] <ADsPath>

      where
        list    specifies to display all objects.
        dump    specifies an object for which to display attributes.
        ADsPath specifies the ADsPath of the directory service to display.

Example Output
==============
Entering the command

 ADsCmd list WinNT://Fabrikam

produces output similar to the following in the Fabrikam domain.

  Administrator(User)
  Guest(User)
  Helper(User)
  TestU(User)
  DnsUpdateProxy(Group)
  Domain Admins(Group)
  Domain Computers(Group)
  Domain Controllers(Group)
  Domain Guests(Group)
  Domain Users(Group)
  Downlevel Name(Group)
  Enterprise Admins(Group)
  Group Policy Creator Owners(Group)
  NewGroup(Group)
  Schema Admins(Group)
  Administrators(Group)
  Users(Group)
  Guests(Group)
  Print Operators(Group)
  Backup Operators(Group)
  Replicator(Group)
  Remote Desktop Users(Group)
  Network Configuration Operators(Group)
  Server Operators(Group)
  Account Operators(Group)
  Pre-Windows 2000 Compatible Access(Group)
  Incoming Forest Trust Builders(Group)
  Cert Publishers(Group)
  RAS and IAS Servers(Group)
  HelpServicesGroup(Group)
  DnsAdmins(Group)
  DHCP Users(Group)
  DHCP Administrators(Group)
  FABRIKAM1(Computer)
  FABRIKAMDC(Computer)
  Schema(Schema)
Total Number of Objects enumerated is 37

Entering the command

  ADsCmd dump LDAP://CN=Computers,DC=Fabrikam,DC=Com

produces output similar to the following in the Fabrikam.Com domain.

cn                              : Computers
instanceType                    : 4
nTSecurityDescriptor            : Data type is 9

objectCategory                  : CN=Container,CN=Schema,  \
                                  CN=Configuration,DC=fabrikam,DC=com
objectClass                     : top, container

How the Sample Works
====================
The sample uses the specified command switch to chose between listing the
contents of the specified object or dumping the properties of the specified
object.  When listing the contents of the specified object, the sample uses
the IADsContainer interface to enumerate the contents.  The sample contains
optional code to filter the type of objects enumerated.  When dumping the
properties of the specified object, the sample uses the IADs interface to
access schema to find the properties to dump.

See Also
========
ADSI Service Providers
IADs interface
IADsContainer interface

