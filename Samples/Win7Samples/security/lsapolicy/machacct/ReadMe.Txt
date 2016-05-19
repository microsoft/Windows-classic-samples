//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  MachAcct Sample: Creating Machine Accounts
//
//--------------------------------------------------------------------------

Description
===========
The MACHACCT sample demonstrates how to create a machine account on the
specified domain.

Machine account types are defined by the following flags:

   UF_SERVER_TRUST_ACCOUNT (Backup domain controller)
   UF_WORKSTATION_TRUST_ACCOUNT (Workstation and server)
   UF_INTERDOMAIN_TRUST_ACCOUNT (Interdomain trust account)

This sample attempts to create a workstation machine account, of type
UF_WORKSTATION_TRUST_ACCOUNT. This account type used for workstations
and non-domain-controller servers that are domain members.

If the computer account creation fails and GetLastError retruns
ERROR_ACCESS_DENIED, the sample attempts to enable the
SeMachineAccountPrivilege for the caller. If the privilege is enabled
successfully, the computer account add operation is retried.

Sample Files
============
  * MachAcct.c
  * ReadMe.txt
  * makefile

Building the Sample
===================
You can build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built the
      sample.
  2.  Type the command "machacct.exe".


Example Output
==============
The following commandline creates a machine account named WINBASE in
the domain named CORP

   machacct.exe WINBASE CORP


Notes
==============

Deleting machine accounts can be accomplished using the NetUserDel
Ported Lan Manager function.

Account update operations against a domain must be performed against
the primary domain controller for the specified domain. This sample
uses the NetGetDCName function to determine the computer name of the
primary domain controller.

Commandline parameter argv[1] indicates the name of the account to
create, which is typically the name of the machine.

Commandline parameter argv[2] is optional and indicates the target domain.
If this commandline argument is omitted, the machine account is created
on the local domain. It is recommended that you always supply a domain
name with this sample, as this insures that the update occurs at the primary
domain controller.

