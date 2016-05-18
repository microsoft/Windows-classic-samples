//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) 1998-1999 Microsoft Corporation.  All Rights Reserved.
//
//  Sample Name:    IISSearch - Sample Forms for Querying Indexing
//                              Service using Internet Information Services
//
//--------------------------------------------------------------------------

Description
===========
  The IISSearch sample is a set of HTML and ASP forms that demonstrates how
  to use Internet Information Services (IIS) to query Indexing Service.
  The forms use Indexing Service Query Language queries and the Query Helper
  API, use SQL queries and the ADO and Query Helper APIs, and use IDQ+HTX
  and the ISAPI Extension API.

Path
====
  Source: mssdk\samples\winbase\indexing\IISSearch\
  
User's Guide
============
  * To install the sample files
      1. Open a command window and change the directory to the source path
         of the sample.
      2. Determine where your inetpub directory resides.
         * If it resides on %SystemDrive%, you can use the install.bat
           file without modifying it.
         * If it resides on another drive, modify the dst environment
           variable in the install.bat file to define the location of
           your inetpub directory. 
      3. At the command-line prompt, type "install".
      
      The sample files are copied to the directory specified by the
      dst environment variable, which is, by default,
      %SystemDrive%\inetpub\iissamples\issamples.

  * To use the sample forms
      1. Start Internet Explorer.
      2. Type in the URL "http://localhost/iissamples/issamples/default.htm".
      3. In the left frame, select the sample query to execute.

Programming Notes
=================
  The sample files include the following.

  advquery.asp
  ------------
    An advanced Active Server Page (ASP) example written in VBScript and
    JScript that illustrates server-side scripting to execute Indexing
    Service Query Language queries using the Query Helper API.

  advsqlq.asp
  ------------
    An advanced ASP example written in VBScript and JScript that
    illustrates server-side scripting to execute SQL queries using the ADO
    and Query Helper APIs.

  default.htm
  -----------
    An HTML page that provides easy access to each sample.

  fastq.htm, fastq.idq, fastq.htx
  -------------------------------
    An optimized IDQ+HTX example that uses the ISAPI Extensions API.

  query.asp
  ---------
    A simple ASP example written in VBScript and JScript that illustrates
    server-side scripting to execute Indexing Service Query Language queries
    using the Query Helper API.

  query.htm, query.idq, query.htx
  -------------------------------
    A simple IDQ+HTX example that uses the ISAPI Extensions API.

  sqlqhit.asp, sqlqhit.htm
  ------------------------
    A simple ASP example written in VBScript that illustrates executing
    queries using the OLE DB Provider for Indexing Service API.
