//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  PropertyList VC Sample: Using ADSI Property Lists
//
//--------------------------------------------------------------------------

Description
===========
The PropertyList sample uses ADSI to bind to Active Directory and enumerate
its attributes.  The sample illustrates use of the IADs, IADsPropertyList,
IADsPropertyEntry, and IADsPropertyValue interfaces.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  ADSIHelpers.Cpp
  *  ADSIHelpers.h
  *  ADsPropertyList.sln
  *  ADsPropertyList.vcproj
  *  Main.Cpp
  *  makefile
  *  readme.txt

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.  You must build and run this sample using Windows 2000 or
later.

To build this sample
  1.  Open the solution ADsPropertyList.sln.
  2.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.	Open a command prompt and change to the directory where you built the
      sample.
  2.	Type the command "ADsPropertyList.exe".

Example Output
==============
Typical output from the sample follows.  Much of the output is deleted to
conserve space.

Binding to a server using LDAP://rootDSE

Binding to the path LDAP://DC=fabrikam,DC=com

Successfully bound to LDAP://DC=fabrikam,DC=com
 NAME: DC=fabrikam
 CLASS: domainDNS
 GUID: 213dd8252be49048bf533e67f8a87a49
 ADSPATH: LDAP://DC=fabrikam,DC=com
 PARENT: LDAP://DC=com
 SCHEMA: LDAP://schema/domainDNS

Enumerating this object's properties using the IADsPropertyList interface

 The Object has 44 properties

 NAME:objectClass
The Values Variant style is :
VT_R4 VT_BSTR VT_VARIANT VT_ARRAY
<top>
<domain>
<domainDNS>

 NAME:description
The Values Variant style is :
VT_R4 VT_BSTR VT_VARIANT VT_ARRAY
<Fabrikam Corporate Domain>

 NAME:distinguishedName
The Values Variant style is :
VT_R4 VT_BSTR VT_VARIANT VT_ARRAY
<DC=fabrikam,DC=com>

...

 NAME:subRefs
The Values Variant style is :
VT_R4 VT_BSTR VT_VARIANT VT_ARRAY
<DC=ForestDnsZones,DC=fabrikam,DC=com>
<DC=DomainDnsZones,DC=fabrikam,DC=com>
<CN=Configuration,DC=fabrikam,DC=com>

...

 NAME:fSMORoleOwner
The Values Variant style is :
VT_R4 VT_BSTR VT_VARIANT VT_ARRAY
<CN=NTDS Settings,CN=FABRIKAMDC,CN=Servers,CN=Default-First-Site-Name,
  CN=Sites,CN=Configuration,DC=fabrikam,DC=com>

...

How the Sample Works
====================
The sample defines the following functions.

  main
    Initializes COM, uses the IADs interface to get the Naming Context
    and bind to the domain controller, calls the PrintIADSObject function,
    and enumerates all the attributes using the IADsPropertyList,
    IADsPropertyEntry, and IADsPropertyValue interfaces, and uninitializes.
  CheckHRESUlT
    Displays a message box for a bad HRESULT.
  GetIADsPropertyValueAsBSTR
    Returns a string that describes a VARIANT vt_member.
  GetVariantStyle
    Returns a basic string value for a propety value.
  PrintIADSObject
    Gets and prints all of the attributes of an IADs object.

See Also
========
IADs interface
IADsPropertyList interface
IADsPropertyEntry interface
IADsPropertyValue interface
Property Cache

