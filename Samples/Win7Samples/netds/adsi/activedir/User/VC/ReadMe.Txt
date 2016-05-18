//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  User VC Sample: Using the IADsUser Interface
//
//--------------------------------------------------------------------------

Description
===========
The User sample binds to a user object in Active Directory and uses
the IADsUser interface to retrieve information about the user.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  IAsUser.cpp
  *  IADsUser.sln
  *  IADsUser.vcproj
  *  makefile
  *  readme.txt


Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution IADsUser.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command

        IADsUser.exe "<username>"

      where <username> is the Common Name of an existing user.

Example Output
==============
The following output results from entering the Common Name of the existing
user "Test User" in the Fabrikam.Com domain.

Finding user: Test User...
Found User.
ADsPath: LDAP://CN=Test User,CN=Users,DC=fabrikam,DC=com
----------------------------------------------
--------Call GetUserPropertyMethods-----------
AccountDisabled: FALSE
AccountExpirationDate: 1/1/1970
BadLoginAddress property is not supported on this platform.
BadLoginCount: 0
Department: Test
Description: A user account for samples
Division property not found.
EmailAddress: testu@fabrikam.com
EmployeeID property not found.
FaxNumber property not found.
FirstName: Test
FullName: Test User
GraceLoginsAllowed property is not supported on this platform.
GraceLoginsRemaining property is not supported on this platform.
HomeDirectory property not found.
HomePage: http://www.fabrikam.com/~testu
IsAccountLocked: FALSE
Languages property is not supported on this platform.
LastName: User
LoginHours property not found.
LoginScript property not found.
LoginWorkstations property not found.
Manager property not found.
MaxLogins property is not supported on this platform.
MaxStorage property not found.
NamePrefix property not found.
NameSuffix property not found.
OfficeLocations: Penthouse
OtherName property not found.
PasswordLastChanged: 6/30/2002 3:38:44 PM
PasswordMinimumLength property not found.
PasswordRequired: FALSE
Picture property not found.
PostalAddresses property not found.
PostalCodes: 99999
Profile property not found.
RequireUniquePassword property not found.
SeeAlso property not found.
TelephoneHome: 888-555-1234
TelephoneMobile property not found.
TelephoneNumber: 888-555-1212
TelephonePager property not found.
Title: Director of Testing
GetUserPropertyMethods HR: 0

How the Sample Works
====================
The sample defines the following functions.

  wmain
    Determines the user name, initializes COM, calls the other functions,
    and uninitializes.
  FindUserByName
    Uses the IDirectorySearch interface to find the selected user.
  GetUserPropertyMethods
    Uses the IADsUser interface to get the properties of the user.

See Also
========
IADs interface
IADsUser interface
IDirectorySearch interface
Searching with IDirectorySearch

