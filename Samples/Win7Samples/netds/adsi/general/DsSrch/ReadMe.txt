//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  DSSrch VC Sample: Directory Services Search
//
//--------------------------------------------------------------------------

Description
===========
The DSSrch sample demonstrates how to search a directory using the
IDirectorySearch interface.

Sample Files
============
  *  DSSrch.sln
  *  DSSrch.vcproj
  *  DSSrch.rc
  *  Main.cpp
  *  Main.h
  *  makefile
  *  Util.cpp

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution DSSrch.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command

        DSSrch /b <baseObject> /f <search_filter> [/f <attrlist>]  \
               [/p <preference>=<pref_value>] [/u <UserName> <Password>]  \
               [/t <flagName>=<flag_value>

      where
        <baseObject>     Specifies the ADsPath of the base of the search.
        <search_filter>  Contains a search filter string in LDAP format.
        <attrlist>       Contains a list of the attributes to display.
        <preference>     Specifies an element from a subset of the
                         ADS_SEARCHPREF_ENUM enumeration; one of:
                           *  Asynchronous
                           *  AttrTypesOnly
                           *  DerefAliases
                           *  SizeLimit
                           *  TimeLimit
                           *  TimeOut
                           *  PageSize
                           *  SearchScope
                           *  SortOn
                           *  CacheResults
        <pref_value>     Specifies a value for the selected search preference.
                           *  YES or NO for a Boolean value
                           *  The respective integer for an integer value
                           *  Base, OneLevel, or Subtree for a scope
        <UserName>       Specifies the DN of a user in the domain to use
                         for credentials.
        <Password>       Specifies the password of the selected user.
        <flagName>       Specifies an authentication option from from a subset
                         of the ADS_AUTHENTICATION_ENUM enumeration: one of:
                           *  SecurAuth
                           *  UseEncrypt
        <flag_value>     Specifies a value for the selected authentication
                         option, either YES or NO.

Example Output
==============
Entering the command

  DSSrch /b "LDAP://fabrikam.com/OU=Example Org Unit,DC=fabrikam,DC=com"
         /f "(objectClass=*)" /a "ADsPath, name, usnchanged"
         /u "CN=Test User,CN=Users,DC=Fabrikam,DC=COM" "password"
         /p SearchScope=OneLevel /t SecureAuth=no
         /p SortOn=name /p CacheResults=no

in the Fabrikam.Com domain produces output similar to the following
(most of the output is deleted to save space).

ADsPath = LDAP://CN=A Helper,CN=Users,DC=fabrikam,DC=com
name = A Helper

ADsPath = LDAP://CN=Administrator,CN=Users,DC=fabrikam,DC=com
name = Administrator
usnchanged = 133002

...

ADsPath = LDAP://CN=Test User,CN=Users,DC=fabrikam,DC=com
name = Test User
usnchanged = 141477

Total Rows: 26

How the Sample Works
====================
The sample uses the ADsPath to bind to the selected base object of Active
Directory.  Then it uses the methods of the IDirectorySearch interface to
search, filter, and print the selected attributes.

See Also
========
ADS_AUTHENTICATION_ENUM
ADS_SEARCHPREF_ENUM
IDirectorySearch interface
LDAP ADsPath
Searching with IDirectorySearch
