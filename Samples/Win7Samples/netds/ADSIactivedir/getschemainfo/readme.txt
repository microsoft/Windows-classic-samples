//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  GetSchemaInfo VC Sample: Querying the Active Directory Schema
//
//--------------------------------------------------------------------------

Description
===========
The GetSchemaInfo sample uses ADSI to bind to the Active Directory schema
container and search for classSchema and/or attributeSchema objects that
match a specified query.

For example, you can search for all global catalog attributes
(IsMemberOfPartialAttributeSet=TRUE) or search for a specific class
(cn=user).  You can specify the attributes to retrieve for the objects
that match the query.

The sample uses the IDirectorySearch interface to perform the search.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  GetSchemaInfo.cpp
  *  GetSchemaInfo.sln
  *  GetSchemaInfo.vcproj
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the workspace GetSchemaInfo.Dsw.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
You must run this sample on a computer using the Windows 2000 or later
operating system.

To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command

        GetSchemaInfo [/C|/A] [/V] [<querystring>]

      where
        /C specifies to query for classes.
        /A specifies to query for attributes.
        /V specifies to return all properties for the found items.
        <querystring> is the query criteria in LDAP query format.

If you specify neither /A nor /C, the query includes both classes and
attributes.  If you don't specify /V, the query returns only the
ldapDisplayName and CN of the resulting items.  If you don't specify a
<querystring>, the query returns all classes and/or attributes.

Example Output
==============
Entering the command

  getschemainfo /A (IsSingleValued=TRUE)

returns all single-valued attributes in the schema.

Here are some example, common <querystring>s for attributes:

  (cn=Street-Address)
    Finds the attribute with CN of Street-Address.
  (ldapdisplayname=street)
    Finds the attribute with ldapdisplayname of street.
  (IsSingleValued=TRUE)
    Finds single-valued attributes.
  (IsSingleValued=FALSE)
    Finds mulit-valued attributes.
  (systemFlags:1.2.840.113556.1.4.804:=00000001)
    Finds non-replicated attributes.
  (systemFlags:1.2.840.113556.1.4.804:=00000004)
    Finds constructed attributes.
  (searchFlags=1)
    Finds indexed attributes.
  (isMemberOfPartialAttributeSet=TRUE)
    Finds attributes included in the global catalog.

How the Sample Works
====================
The sample defines the following functions.

  wmain
    Initializes using the IADs interface, calls the FindAttributesOrClasses
    function, uninitializes.
  FindAttributesOrClasses
    Uses the IDirectorySearch interface to perform the query.

See Also
========
IADs interface
IDirectorySearch interface
Reading the Abstract Schema
Searching with IDirectorySearch

