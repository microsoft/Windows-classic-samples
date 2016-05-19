THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999  Microsoft Corporation.  All Rights Reserved.

DHCP Client Options API Samples
-------------------------------

The DHCP Client Options API provides an API that allows an application 
to hook into the DHCP Client of a machine to request additional options 
from a DHCP Server or receive notification of options when they change.

DHCPPersist
-----------

Abstract:

  This sample demonstrates how to request an option to be made persistent
  (i.e. the option is retrieved during every DHCP Client boot). This sample
  will request that the Time Servers option (OPTION_TIME_SERVERS = 4) 
  be requested persistently. When the command-line option /p is used the
  DhcpRequestParams() API is used with the DHCPCAPI_REQUEST_PERSISTENT
  flag. The request is recorded and will be pulled down during a client
  boot. The server must support the option for this to work. Once the option
  has been made persistent, the normal DhcpRequestParams() call will retrieve
  the option (this sample with no /p or /r). 

  Specifying /r removes the request for the persistence of the option and
  will remove the request from the client. This is performed via the 
  DhcpUndoRequestParams() API. When registered and undoing this, each set is
  specified by a unique Request ID string. The string used to setup the
  persistence is also used to undo the persistence. 

Supported OS:

  Windows 2000, Windows 98

Building:

  Build the sample using the latest Platform SDK via the MAKEFILE included. 
  When using the sample code in a project, be sure to link with the 
  DHCPCSVC.LIB and the IPHLPAPI.LIB libraries. 

Usage:

  The sample can be run directly on the command-line by typing 'DHCPPERSIST'
  and then specifying /p to add the persistent option or /r to remove the 
  persistent option.   

  An adapter can be specified on the command-line. To specify an adapter on 
  Windows 2000, use the adapter GUID, on Windows 98, the adapter index. These 
  are the values that are obtained via the IP Helper API, GetInterfaceInfo()
  as demonstrated in the code included.
