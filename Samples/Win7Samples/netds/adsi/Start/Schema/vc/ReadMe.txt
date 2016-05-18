//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Schema VC Sample: Reading the WinNT Schema
//
//--------------------------------------------------------------------------

Description
===========
The Schema sample program enumerates the objects in the schema using the
WinNT provider.

This sample uses the WinNT: provider and is suitable for Windows NT(R) 4.0
networks as well as Windows 2000 and later networks running Active Directory.

Sample Files
============
  *  makefile
  *  Schema.cpp
  *  Schema.sln
  *  Schema.vcproj
  *  StdAfx.cpp
  *  StdAfx.h

Building the Sample
===================
When you build this sample using Visual Studio, be sure that you have the
INCLUDE directory for the Platform SDK set first in the Options list of
include files.

To build this sample
  1.  Open the solution Schema.sln.
  2.  Open the source file Schema.cpp.
  3.  Replace the domain name "INDEPENDENCE" with the appropriate domain
      name, such as FABRIKAM, in the following line.
        hr = ADsGetObject(L"WinNT://INDEPENDENCE/Schema",  \
                          IID_IADsContainer, (void**) &pSchema );
  4.  From the Build menu, select Build.

You can also build this sample at a command prompt using the supplied
makefile.

Running the Sample
==================
To run this sample
  1.  Open a command prompt and change to the directory where you built
      the sample.
  2.  Type the command "Schema.exe".

You can also run the sample by selecting Execute Schema.exe from
the Build menu.

Example Output
==============
If the sample executes successfully, it prints output similar to the
following in a command window.

Domain                  (Class)
Computer                (Class)
User                    (Class)
Group                   (Class)
Service                 (Class)
FileService             (Class)
Session                 (Class)
Resource                (Class)
FileShare               (Class)
FPNWFileService         (Class)
FPNWSession             (Class)
FPNWResource            (Class)
FPNWFileShare           (Class)
PrintQueue              (Class)
PrintJob                (Class)
Boolean                 (Syntax)
Counter                 (Syntax)
ADsPath                 (Syntax)
EmailAddress            (Syntax)
FaxNumber               (Syntax)
Integer                 (Syntax)
Interval                (Syntax)
List                    (Syntax)
NetAddress              (Syntax)
OctetString             (Syntax)
Path                    (Syntax)
PhoneNumber             (Syntax)
PostalAddress           (Syntax)
SmallInterval           (Syntax)
String                  (Syntax)
Time                    (Syntax)
MinPasswordLength       (Property)
MinPasswordAge          (Property)
MaxPasswordAge          (Property)
MaxBadPasswordsAllowed  (Property)
PasswordHistoryLength   (Property)
AutoUnlockInterval      (Property)
LockoutObservationInterval (Property)
Owner                   (Property)
Division                (Property)
OperatingSystem         (Property)
OperatingSystemVersion  (Property)
Processor               (Property)
ProcessorCount          (Property)
Description             (Property)
FullName                (Property)
AccountExpirationDate   (Property)
PasswordAge             (Property)
UserFlags               (Property)
LoginWorkstations       (Property)
BadPasswordAttempts     (Property)
MaxLogins               (Property)
MaxStorage              (Property)
PasswordExpired         (Property)
PasswordExpirationDate  (Property)
LastLogin               (Property)
LastLogoff              (Property)
HomeDirectory           (Property)
Profile                 (Property)
Parameters              (Property)
HomeDirDrive            (Property)
LoginScript             (Property)
LoginHours              (Property)
PrimaryGroupID          (Property)
objectSid               (Property)
RasPermissions          (Property)
groupType               (Property)
HostComputer            (Property)
DisplayName             (Property)
ServiceType             (Property)
StartType               (Property)
Path                    (Property)
ErrorControl            (Property)
LoadOrderGroup          (Property)
ServiceAccountName      (Property)
Dependencies            (Property)
Version                 (Property)
MaxUserCount            (Property)
User                    (Property)
Computer                (Property)
ConnectTime             (Property)
IdleTime                (Property)
LockCount               (Property)
CurrentUserCount        (Property)
PrinterPath             (Property)
PrinterName             (Property)
Model                   (Property)
Datatype                (Property)
PrintProcessor          (Property)
ObjectGUID              (Property)
Action                  (Property)
Location                (Property)
StartTime               (Property)
UntilTime               (Property)
DefaultJobPriority      (Property)
JobCount                (Property)
Priority                (Property)
Attributes              (Property)
BannerPage              (Property)
PrintDevices            (Property)
HostPrintQueue          (Property)
TimeSubmitted           (Property)
TotalPages              (Property)
Size                    (Property)
Notify                  (Property)
TimeElapsed             (Property)
PagesPrinted            (Property)
Position                (Property)
Name                    (Property)

How the Sample Works
====================
The sample uses the WinNT ADsPath and the IADs interface to perform
the bindings and the IADsContainer interface to enumerate the objects
in the schema.

See Also
========
IADsContainer interface
WinNT ADsPath
WinNT Binding String (ADsPath)

