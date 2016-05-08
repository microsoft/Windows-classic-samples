//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  WAB ASP Sample: Using ADSI and ASP to search Active Directory
//
//--------------------------------------------------------------------------

Description
===========
The WAB sample shows how an Active Server Page (ASP) can use ADSI to
search Active Directory.  The sample uses ActiveX Data Objects (ADO) to
search for a specified user or group name and retrieves information from
the objects returned by the search.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  Banner.Gif
  *  Default.Asp
  *  Detail.Asp
  *  Global.Asa
  *  Group.Asp
  *  Person.Asp
  *  Print.Gif
  *  Search.Asp
  *  Search.Gif
  *  Search.Jpg

Running the Sample
==================
This sample requires that you install Microsoft Internet Information Services
on a Web server in the domain.  For example, the location of the Web server
might be www.fabrikam.com.

To run this sample
  1. Edit the Global.Asa file to include the appropriate domain name and
     user credentials for a query.  Change the lines

       Session("ADsDomain") = "YourDominDNSNameHere"
       Application("UserID") = "YourDomainHere\YourUserNameHere"
       Application("Password") = "yourPasswordHere"

     appropriately for your situation.  For example, for the Fabrikam.Com
     domain, this might be

       Session("ADsDomain") = "fabrikam.com"
       Application("UserID") = "FABRIKAM\administrator"
       Application("Password") = "password"

     Note that you usually should not hardcode this information.
  2. Copy all the sample files to the wwwroot folder of the Web server.
  3. On any computer in the domain using Microsoft Internet Explorer, enter
        "http://www.fabrikam.com/Default.Asp"
     in the Address field and select Go.
  4. In the resulting "Windows Address Book - ADSI" page, enter an alias,
     first name, last name, group name, or distribution list name in the
     Name field and select Search.

Example Output
==============
The sample produces HTML output similar to the following, which is for the
Fabrikam.Com domain when the search is for "user".

Search for:   user     

Name        Phone          Title                 Office      Department
 
 
Test User   888-555-1212   Director of Testing   Penthouse   Test 
First User     
Users       (Group)
  
3 object(s) found 


Query was executed in: 0.1640625 second(s)
Total Time (Execute, Enumeration and Rendering): 0.1953125 second(s)

If you click on an entry in the Name column, an additional page appears
with details about that entry.

How the Sample Works
====================
The Default.Asp file accepts the alias, first name, last name, group name,
or distribution list name and posts it to the Search.Asp file.

The Search.Asp file performs an ADO query and then enumerates the resulting
objects as the results of the search.

The Person.Asp and Group.Asp files provide details about a selected person
or group.

