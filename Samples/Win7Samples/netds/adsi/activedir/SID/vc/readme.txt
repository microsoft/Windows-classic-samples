//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  SID VC Sample: Retrieving a User's SID
//
//--------------------------------------------------------------------------

Description
===========
The SID sample uses ADSI to bind to the local domain partition in
Active Directory.  It then searches for a specified user object.  When it
finds the object, the sample retrieves the user's SID and converts the
binary SID to a SID string format.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  makefile
  *  ObjectSid.cpp
  *  UserObjectSid.sln
  *  UserObjectSid.vcproj
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution UserObjectSid.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command

        UserObjectSid.exe "<username>"

      where <username> is the Common Name of an existing user.

Alternatively, you can enter just the command "UserObjectSid.exe" at the
command prompt, and the sample will prompt you for the Common Name.

Example Output
==============
The following output results from entering the Common Name of the existing
user "Guest" in the Fabrikam.Com domain.

Finding user: Guest...
Found User.
ADsPath: LDAP://CN=Guest,CN=Users,DC=fabrikam,DC=com
----------------------------------------------
----------Call GetLPBYTEtoOctetString---------
----------------------------------------------
objectSid:S-1-5-21-1757981266-1606980848-1957994488-501

How the Sample Works
====================
The sample defines the following functions.

  wmain
    Determines the user name, initializes COM, calls the other functions,
    and uninitializes.
  FindUserByName
    Uses the IDirectorySearch interface to find the selected user.
  GetLPBYTEtoOctetString
    Gets the octet string of the user's SID.

See Also
========
IADs interface
IDirectorySearch interface
Converting a Binary SID to String Format
Searching with IDirectorySearch
