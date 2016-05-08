//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  AddGroup VC Sample: Adding a group to Active Directory
//
//--------------------------------------------------------------------------

Description
===========
The AddGroup sample adds a group object to a specified container in Active
Directory.  The specified container is commonly an organizational unit.

The sample uses the IDirectoryObject interface to create and set properties
for the group object.  Optionally, the sample can create a local, global, or
universal domain group; and can also specify whether the new group is a
distribution group, such as an e-mail distribution list, or a security group
for use in access-control entries (ACEs).

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  AddGroup.sln
  *  AddGroup.vcproj
  *  Main.Cpp
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution AddGroup.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
You must run this sample in a Windows domain where you have permission to
write to Active Directory.

To run this sample
  1.  Open a command prompt and change to the directory where you built the
      sample.
  2.  After selecting suitable parameters, type the followin command.
      The last two parameters of the command are optional.

        addgroup <ADSPath> <Windows2000_Group> <DownLevel_Name>  \
                 [{global | local | universal}] [{Security | NoSecurity}]

      In this command:

        <ADSPath> is Active Directory service path of the container in which
                  to create the group.
        <Windows2000_Group> is the Windows 2000 group name.
        <DownLevel_Name> is the Windows NT 4 downlevel compatible group name.
        {global | local | universal} specifies the scope of the group.
        {security | nosecurity} specifies the security context of the group.

Example Output
==============
The following example assumes that the domain is Fabrikam.Com and that an
organizational unit named "Example Org Unit" exists in that domain.  The
command

Addgroup "LDAP://OU=Example Org Unit,DC=Fabrikam,DC=Com" "Win2K Group Name" \
         "Downlevel Name" global security

creates the group "Win2K Group Name" in the Fabrikam.Com domain with global
scope and with security enabled.

The output from this command is the following.

 New Group created with the following properties:
 NAME: CN=Win2K Group Name
 CLASS: group
 GUID: 8edcc9e9d52cbd48a30f1bd737b530d2   (this value differs in each run)
 ADSPATH: LDAP://CN=Win2K Group Name,OU=Example Org Unit,DC=Fabrikam,DC=Com
 PARENT: LDAP://OU=Example Org Unit,DC=Fabrikam,DC=Com
 SCHEMA: LDAP://schema/group

You can verify the result by opening and examining the Active Directory Users
and Computers snap-in of the Microsoft Management Console.

How the Sample Works
====================
The sample defines the following functions.

  main
    Performs initialization, calls other functions, and performs finalization.
  CheckADHRESULT
    Displays ADSI or COM error message given an HRESULT.
  CrackADError
    Converts HRESULT to error string.
  CreateGroup
    Uses the IDirectoryObject inteface to create a group and set
    its properties.
  ParseCommandLine
    Parses the command line to determine parameters for the group.
  PrintADSObject
    Prints all the attributes of an Active Directory service object.

See Also
========
IDirectoryObject::CreateDSObject
Creating Groups in a Domain

