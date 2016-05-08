//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  CreateUser VC Sample: Creating a User Account in Active Directory
//
//--------------------------------------------------------------------------

Description
===========
The CreateUser sample uses ADSI with the LDAP provider to create a domain
account for a user.

The sample sets mandatory properties such as sAMAccountName as well as
additional optional properties.  The sample uses the IDirectoryObject
interface to create and initialize the user object.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  CreateUser.sln
  *  CreateUser.vcproj
  *  CreateUserHelpers.Cpp
  *  CreateUserHelpers.h
  *  Data.Txt
  *  Main.Cpp
  *  makefile
  *  readme.txt
  *  UserProps.Cpp
  *  UserProps.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution CreateUser.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
You must run this sample in a Windows domain where you have permission to
write to Active Directory.

To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command

        CreateUser /LDAP <LDAP_Path> /UNAME <W2K_User_Name>  \
                   /SAMNAME <Downlevel_Name>  [/FILE <File_Name>]  \
                   [/USER <User_Name> /PASS <Password>]

      where
        <LDAP_Path> is the Distinguished Name of the container
                    to hold the user object
        <W2K_User_Name> is the Common Name of the user to create
        <Downlevel_Name> is the NT4 downlevel SAM Account Name
                         (< 20 characters)
        <File_Name> is a filename containing detailed user information
        <User_Name>  is the User ID of alternative credentials to use
                     to create the account
        <Password> is the password for the User ID provided with <User_Name>

The final three parameters are optional, although the last two must appear
together.

Example Output
==============
Entering the command

  CreateUser /LDAP "LDAP://OU=Example Org Unit,DC=Fabrikam,DC=Com"  \
             /UNAME "Test User" /SAMNAME "TestU" /USER administrator  \
             /PASS password

creates the user "Test User" in the Organization Unit "Example Org Unit"
in the domain Fabrikam.Com with SAM Account name "TestU" with
administrator credentials.

The output from this command is the following.

 New User created with the following properties:

 NAME: CN=Test User
 CLASS: User
 GUID: 70da5316cb6ee74ea7f05bb413753d21  (this value differs in each run)
 ADSPATH: LDAP://CN=Test User,OU=Example Org Unit,DC=Fabrikam,DC=Com
 PARENT: LDAP://OU=Example Org Unit,DC=Fabrikam,DC=Com
 SCHEMA: LDAP://schema/User

Entering the command (using the supplied sample file Data.Txt)

  CreateUser /LDAP "LDAP://OU=Example Org Unit,DC=Fabrikam,DC=Com"
  /UNAME "Test User" /SAMNAME "TestU" /FILE Data.Txt

creates the same user (without administrator credentials) but with additional
attributes specified in the Data.Txt file.

The output from this command is the following.

 attrib:objectClass value:User
 attrib:sAMAccountName value:TestU
 attrib:LogonHours value:x00 x00 x00 x00 xe0 xff x03 xe0 xff x03 xe0 xff x03 xe0 xff x03 xe0 xff x03 x00 x00
 attrib:accountExpires value:125938656000000000
 attrib:assistant value:CN=A Helper,CN=Users,DC=Fabrikam,DC=Com
 attrib:description value:This is a user created from a file..

 New User created with the following properties:

 NAME: CN=Test User
 CLASS: User
 GUID: 14d5f80b623b1549877ba720a1877767
 ADSPATH: LDAP://CN=Test User,OU=Example Org Unit,DC=Fabrikam,DC=Com
 PARENT: LDAP://OU=Example Org Unit,DC=Fabrikam,DC=Com
 SCHEMA: LDAP://schema/User

How the Sample Works
====================
The sample uses the IDirectoryObject interface to create the user object
and the IADs interface to print some of its properties.

See Also
========
IADs interface
IDirectoryObject interface
Creating a User

