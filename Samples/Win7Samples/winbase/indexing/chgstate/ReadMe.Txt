//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Sample Name:    ChgState - Sample Indexing Service Admin Application
//
//--------------------------------------------------------------------------

Description
===========
  The ChgState sample is a command-line application written in C++ that can
  either report the current state of a catalog or change the state of the
  catalog using the SetCatalogState function of the OLE DB Helper API.
  
Path
====
  Source: mssdk\samples\winbase\indexing\ChgState\
  
User's Guide
============
  * To build the example
      1. Set the Lib environment variable to "D:\mssdk\Lib;%Lib%" and the
         Include environment variable to "D:\mssdk\Include;%Include%",
         where D: is the drive on which you installed the Platform SDK.
      2. Correctly set the CPU environment variable to, for example, "i386".
      3. Open a command window and change the directory to the source path
         of the sample.
      4. Build the example by entering, at the command-line prompt, "nmake".

  * To find or change the catalog state using the example
      1. Open a command window and change the directory to the path of the
         built sample.
      2. Submit a catalog state command by entering, at the command-line
         prompt,

         chgstate /a:<action> [/c:<catalog>] [/m:<machine>]

         where

         <action>   is a catalog state action; one of
                        RO          Set read-only (queries only)
                        RW          Set read/write (queries and indexing)
                        Stop        Set stopped (no queries or indexing)
                        GetState    Get current state only
         <catalog>  is the name of the catalog  (default is "system")
         <machine>  is the name of the machine  (default is ".")

Programming Notes
=================
  The previous state of the specified catalog is always returned in the
  dwOldState parameter of the SetCatalogState function and printed by the
  sample.

  The state of the catalog will change to the new, specified state if that
  state does not violate read-only constraints on the catalog.  If the
  catalog resides on a read-only medium, the catalog is specified as
  read-only in the registry, or the file cicat.hsh is set read-only, the
  catalog is set to read-only even when the read/write state is requested.
  
  Parameters
  ----------
    The <action> parameter is the action desired.  Four actions are possible: 
      * RO (Read-only): queries are allowed, but no indexing occurs.
      * RW (Read/write): queries are allowed and indexing occurs.
      * Stop (Stopped): stop catalog completely (no queries, no indexing).
      * GetState (Get state): get the current state of a catalog only.  The
        catalog state prior to any specified action is always returned.
    
    The <catalog> parameter is the name of the catalog on which to perform
    the action.  The default is "system", which is the default catalog
    installed with Windows 2000.  Additional catalogs can be created with
    the Indexing Service snap-in of the Microsoft Management Console (MMC).

    The <machine> parameter is the name of the machine on which the request
    is executed.  The default is ".", which is the local machine.  Machine
    names should not be preceeded with two backslashes.

  Examples
  --------
    chgstate /a:RW /c:system
      Change the "system" catalog on the local machine to the
      Read/write state.
    
    chgstate /a:GetState /m:SERVERNAME 
      Get the current state of the "system" catalog on machine SERVERNAME.

    chgstate /a:RO /c:CATALOGNAME /m:SERVERNAME
      Set the catalog CATALOGNAME on machine SERVERNAME to the Read-only 
      state.  (The catalog can be queried, but indexing is stopped.)

