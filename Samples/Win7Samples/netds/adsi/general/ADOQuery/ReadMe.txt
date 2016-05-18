//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  ADOQuery VBS Sample: Search Active Directory using ADO
//
//--------------------------------------------------------------------------

Description
===========
The ADOQuery sample demonstrates how to use the ADODB interface to search
for and enumerate the objects in Active Directory.  The sample illustrates
both LDAP and SQL query strings.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  ADOQuery.Vbs

Running the Sample
===============================
To build and run this sample
  1.  Open the script ADOQuery.Vbs in an editor.
  2.  Replace the computer name "yourServer" with an appropriate computer
      name, such as FABRIKAMDC, and the domain "DC=ArcadiaBay,DC=com" with
      the appropriate domain name, such as "DC=fabrikam,DC=com" in the
      following line.
        adDomainPath = "LDAP://yourServer/DC=ArcadiaBay,DC=com"
  3.  Save the changed script.
  4.  Open a command prompt and change to the directory of the sample.
  5.  Type the command "cscript ADOQuery.Vbs".

You can also try other variations of the script by editing the script and
following the suggestions presented in the comments.

Example Output
==============
If the sample executes successfully, it prints output to the command window
similar to the following.  (Only part of the output is shown to save space.)
Otherwise, an error message appears.

Name  =  fabrikam
Name  =  Users
Name  =  Computers
Name  =  Domain Controllers
Name  =  System
Name  =  LostAndFound
Name  =  Infrastructure
Name  =  ForeignSecurityPrincipals
Name  =  WinsockServices
Name  =  RpcServices
Name  =  FileLinks
Name  =  VolumeTable
Name  =  ObjectMoveTable
Name  =  Default Domain Policy
Name  =  AppCategories
Name  =  Meetings
Name  =  Policies
...
Name  =  Domain Computers
Name  =  Domain Controllers
Name  =  Schema Admins
Name  =  Enterprise Admins
Name  =  Cert Publishers
Name  =  Domain Admins
Name  =  Domain Users
Name  =  Domain Guests
Name  =  Group Policy Creator Owners
Name  =  RAS and IAS Servers
Name  =  Server Operators
Name  =  Account Operators
Name  =  Pre-Windows 2000 Compatible Access
Name  =  Incoming Forest Trust Builders
Name  =  RID Manager$
Name  =  RID Set
Name  =  DnsAdmins
Name  =  DnsUpdateProxy
Name  =  MicrosoftDNS
Name  =  RootDNSServers
Name  =  @
Name  =  a.root-servers.net
Name  =  b.root-servers.net
Name  =  c.root-servers.net
Name  =  d.root-servers.net
Name  =  e.root-servers.net
Name  =  f.root-servers.net
Name  =  g.root-servers.net
Name  =  h.root-servers.net
Name  =  i.root-servers.net
Name  =  j.root-servers.net
Name  =  k.root-servers.net
Name  =  l.root-servers.net
Name  =  m.root-servers.net
Name  =  Domain System Volume (SYSVOL share)
Name  =  FABRIKAMDC
Name  =  NTFRS Subscriptions
Name  =  Domain System Volume (SYSVOL share)
...
Name  =  Example Org Unit
Name  =  Win2K Group Name
Name  =  My New Group
Name  =  A Helper
Name  =  Test User
Name  =  First User
Name  =  Another Example Org Unit
Name  =  Jeffrey Smith
Name  =  Jane Johnson

How the Sample Works
====================
The sample uses the LDAP ADsPath to perform the binding and ActiveX Data
Objects (ADO) to perform the search and enumeration of the selected objects.
The sample contains examples of both SQL and LDAP syntax queries.

See Also
========
IADs interface
LDAP ADsPath
LDAP Binding String (ADsPath)

