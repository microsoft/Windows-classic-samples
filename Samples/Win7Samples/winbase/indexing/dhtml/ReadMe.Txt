//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Sample Name:    DHtml - Sample Dynamic HTML Query Page
//
//--------------------------------------------------------------------------

Description
===========
  The DHTML sample (CiDHtml.htm) is an example dynamic-HTML application
  using VBScript and JScript that creates a client-side query form to query
  an Indexing Service catalog.  The sample uses the Query Helper API for the
  query and an ADO recordset to return the results.

Path
====
  Source: mssdk\samples\winbase\indexing\DHtml\

User's Guide
============
  * To run the sample
      1. Load CiDHtml.htm into any DHTML-capable browser.
      2. Enter the query text in the first text box using Indexing
         Service Query Language Dialect 2.
      3. Click Go to run the query.

Programming Notes
=================
  CIDHtml.HTM uses dynamic HTML to build a client-side query form that can
  search an Indexing Service catalog.  The sample uses the Query object
  of the Query Helper API and the ADO Recordset object.  The sample uses
  VBScript to program the objects.  Data entry uses an HTML form, and the
  query results are formatted using dynamic HTML.  The sample uses JScript
  to present help in querying.
