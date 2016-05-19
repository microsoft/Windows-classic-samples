========================================================================
    CONSOLE APPLICATION : isb Project Overview
========================================================================

This file contains a summary of what you will find in each of the files that
make up your isb application.


isb.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

isb.cpp
    This is the main application source file.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named isb.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
/******************************************************************************\
* isb.cpp
*
* This simple IPv6 sample demonstrates the use of Winsock Ideal Send Backlog functionality.
*
* The Ideal Send Backlog functionality is new to Windows Sockets in Windows Vista SP1
* and Windows Server 2008.
* 
* This sample requires that TCP/IP version 6 be installed on the system (default
* configuration for Windows Vista and Windows Server 2008).
*
* For an application to effectively use this functionality, the application should: 
* 1. First query for the initial ideal send backlog value (idealsendbacklogquery).
* 2. Post overlapped idealsendbacklognotify to receive isb change indications. 
* 3. Upon notify completion, immediately post another idealsendbacklognotify and
*    query for new isb value (idealsendbacklogquery).
* 
* The I/O model in the sample uses blocking send/recv calls to keep the sample simple.
* A real world application could use non-blocking or overlapped I/O for possibly better performance.
* 
* Note: 
* On Windows 7/Server 2008 R2 and later versions, a new feature called Send Path Auto-Tuning is available.
* With this new functionaly, Windows can perform the send auto-tuning (ideal send backlog)
* on the application's behalf. To enable this functionality, the application:
* 1. Must not change the connected socket's send buffer limit (SO_SNDBUF).
* 2. Must not query for the ideal send backlog value. In other words, application must 
*    not call idealsendbacklogquery.
* 
* If application does either of the above, the send auto-tuning will be disabled for that connection. 
* 
* The appication may however, post idealsendbacklognotify to receive indications that a send auto-tuning
* event has occurred.
* 
*
*
* This is a part of the Microsoft Source Code Samples.
* Copyright 1996 - 2009 Microsoft Corporation.
* All rights reserved.
* This source code is only intended as a supplement to
* Microsoft Development Tools and/or WinHelp documentation.
* See these sources for detailed information regarding the
* Microsoft samples programs.
\******************************************************************************/

To run the sample in Ideal Send Backlog mode (on VistaSP1/Server2008 and later):

Listener:
isb -l -e <port#>

For example:
isb -l -e 12345

Originator:
isb -n <ListenerNameOrIPAddress> -e <port#>

For example:
isb -n machine1.live.com -e 12345

To run the sample in Send Auto Tuning mode (on Windows 7/Server 2008 R2 and later):
Add -a switch to the listener commandline.

For example:
isb -l -e 12345 -a


