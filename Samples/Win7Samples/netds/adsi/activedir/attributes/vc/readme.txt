//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Attributes VC Sample: Enumerating Attribute Types
//                        in the Active Directory Schema
//
//--------------------------------------------------------------------------

Description
===========
The Attributes sample uses ADSI to bind to the Active Directory schema
container and enumerate some types of its attributes.

The sample uses the IDirectorySearch interface to enumerate indexed
attributes, global catalog attributes, constructed attributes, and
non-replicated attributes.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  Attributes.Cpp
  *  Attributes.sln
  *  Attributes.vcproj
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Attributes.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built the
      sample.
  2.  Type the command "attributes.exe".

Example Output
==============
Typical output from the sample follows.  Much of the output is deleted to
conserve space.

This program displays the following types of attributes in the schema:
Non-Replicated, Indexed, Constructed, Global Catalog

----------------------------------------------
Non-Replicated attributes (stored on each domain controller but are not replicated elsewhere)
Find non-replicated attributes
----------------------------------------------
Global Catalog attributes (replicated to the Global Catalog)
Find attributes included in the global catalog
ldapDisplayName: altSecurityIdentities
ldapDisplayName: cACertificate
...
ldapDisplayName: userCertificate
----------------------------------------------
Constructed attributes (not stored in the directory but are calculated by the domain controller)
Find constructed attributes
ldapDisplayName: allowedAttributes
ldapDisplayName: allowedAttributesEffective
...
ldapDisplayName: subSchemaSubEntry
----------------------------------------------
Indexed attributes (indexed for efficient search)
Find indexed attributes
ldapDisplayName: altSecurityIdentities
ldapDisplayName: birthLocation
...
ldapDisplayName: volTableIdxGUID

How the Sample Works
====================
The sample defines the following functions.

  main
    Initializes COM, uses the IADs interface to get the Naming Context,
    uses the IDirectorySearch interface to get the schema container's
    DN, calls each of the FindXXX functions to enumerate the attributes,
    and uninitializes.
  FindAttributesByType
    Uses the IDirectorySearch interface to find and enumerate all attributes
    of a specified type.
  FindGCAttributes
    Uses the IDirectorySearch interface to find and enumerate all attributes
    included in the global catalog.
  FindIndexedAttributes
    Uses the IDirectorySearch interface to find and enumerate all indexed
    attributes.

See Also
========
IADs interface
IDirectorySearch interface
Searching with IDirectorySearch

