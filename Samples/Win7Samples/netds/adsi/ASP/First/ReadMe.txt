//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  First ASP Sample: Enumerating a Computer Object using ADSI and ASP
//
//--------------------------------------------------------------------------

Description
===========
The First sample shows how to use ADSI in an Active Server Page (ASP) to bind
to a specified computer and enumerate information about the computer.

This sample uses the WinNT: provider and is suitable for Windows NT(R) 4.0
networks as well as Windows 2000 and later networks running Active Directory.

Sample Files
============
  *  Default.Htm
  *  Enum.Asp

Running the Sample
==================
This sample requires that you install Microsoft Internet Information Services
on a Web server in the domain.  For example, the location of the Web server
might be www.fabrikam.com.

To run this sample
  1.  Copy the two sample files to the wwwroot folder of the Web server.
  2.  In Microsoft Internet Explorer, enter "http://www.fabrikam.com"
      in the Address field and select Go.
  3.  In the resulting "ADSI ASP Sample: Enumerating a computer object" page,
      enter a computer name to enumerate and a user name and password to use
      for credentials.

Example Output
==============
The sample produces HTML output similar to the following, which is for the
computer FABRIKAMDC.  Most of the output is deleted to save space.

Computer Name: fabrikamdc

Contains the following objects:

 Administrator      User 
 ASPNET             User 
 Guest              User 
 ...
 Administrators     Group 
 Users              Group 
 Guests             Group 
 Print Operators    Group 
 Backup Operators   Group 
 ... 
 Alerter            Service 
 ALG                Service 
 AppMgmt            Service 
 appmgr             Service 
 aspnet_state       Service 
 AudioSrv           Service 
 BITS               Service 
 Browser            Service 
 ...

How the Sample Works
====================
The Default.Htm file accepts the computer name, user name and password and
posts them to the Enum.Asp file.

The Enum.Asp file contains the script statements

  compName = Request.Form("computer")
  usrName = Request.Form("userName")
  password = Request.Form("password")

  adsPath = "WinNT://" & compName & ",computer"
  Set dso = GetObject("WinNT:")
  Set comp = dso.OpenDSObject(adsPath, userName, password, 1)

which specify the computer object for the query.  The ASP file then
enumerates the objects in the specified computer object and outputs the
Name and Class of each enumerated object.


