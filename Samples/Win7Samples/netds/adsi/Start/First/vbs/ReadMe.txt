//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  First VBS Sample: Enumerating Domain and Computer Objects with ADSI
//
//--------------------------------------------------------------------------

Description
===========
The First sample binds to a specified domain and computer using the WinNT
provider and enumerates the objects in each.

This sample uses the WinNT: provider and is suitable for Windows NT(R) 4.0
networks as well as Windows 2000 and later networks running Active Directory.

Sample Files
============
  *  First.Vbs

Running the Sample
===============================
To build and run this sample
  1.  Open the script First.Vbs in an editor.
  2.  Replace the computer name "mymachine" with an appropriate computer
      name, such as FABRIKAMDC, in the following line.
        machineName = "mymachine"
  3.  Replace the domain name "myDomain" with the appropriate domain
      name, such as Fabrikam, in the following line.
        domainName = "myDomain"
  4.  Save the changed script.
  5.  Open a command prompt and change to directory of the sample.
  6.  Type the command "cscript First.Vbs".

Example Output
==============
If the sample executes successfully, it prints output to the command window
similar to the following.  (Only part of the output is shown to save space.)
Otherwise, an error message appears.

Administrator
Guest
Helper
TestU
user1
Administrators
Users
Guests
Print Operators
Backup Operators
Replicator
Remote Desktop Users
Network Configuration Operators
Server Operators
Account Operators
Pre-Windows 2000 Compatible Access
Incoming Forest Trust Builders
Cert Publishers
RAS and IAS Servers
HelpServicesGroup
DnsAdmins
DHCP Users
DHCP Administrators
DnsUpdateProxy
Domain Admins
Domain Computers
Domain Controllers
Domain Guests
Domain Users
Downlevel Name
Enterprise Admins
Group Policy Creator Owners
NewGroup
Schema Admins
Alerter
ALG
...
FABRIKAMDC
Schema

How the Sample Works
====================
The sample uses the WinNT ADsPath to perform the binding and the
IADs interface to enumerate the properties.

See Also
========
IADs interface
WinNT ADsPath
WinNT Binding String (ADsPath)

