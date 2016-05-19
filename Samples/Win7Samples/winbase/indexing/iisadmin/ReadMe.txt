//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) 1998-1999 Microsoft Corporation.  All Rights Reserved.
//
//  Sample Name:    IISAdmin - Sample Forms for Administering Indexing
//                             Service using Internet Information Services
//
//--------------------------------------------------------------------------

Description
===========
  The IISAdmin sample is a set of HTML and ASP forms that demonstrates how
  to use Internet Information Services (IIS) to administer Indexing Service.
  The forms retrieve indexing statistics, force a master merge, rescan paths,
  and check for documents that could not be filtered.
 
Path
====
  Source: mssdk\samples\winbase\indexing\IISAdmin\
  
User's Guide
============
  * To install the sample files
      1. Open a command window and change the directory to the source path
         of the sample.
      2. At the command-line prompt, enter "install".
      
    The sample files are copied to the directory
    %windir%\system32\inetsrv\iisadmin\isadmin.

  * To use the sample forms
      1. Start Internet Explorer.
      2. Type in the URL "http://localhost/iisadmin/isadmin/admin.htm".
      3. In the left frame, select the desired administration operation.

Programming Notes
=================
  The forms in the IISAdmin sample require that IIS be running.

  The main sample form is admin.htm, which controls all the
  administrative operations.
