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

DHCPNotify
----------

Abstract:

  This sample demonstrates obtaining notification of parameters when they 
  change. This example monitors the change of the Gateway address 
  (OPTION_ROUTER_ADDRESS = 3) in a DHCP Client. The DHCP Client Options API 
  will signal an application via an Event handle whenever this value changes. 
  This change can occur during the renew or release state of a DHCP Client. 

  This sample shows how to use the API to register for this event. It then 
  will sit in a loop waiting for the parameter to change or for the exit 
  event to become signaled. If the DHCP Client Options API signals the 
  application, the value could then be read and changed in the application 
  relying on the information. The reading of the value is not demonstrated 
  here as that is in the DHCPRequest sample located in the Platform SDK.

  If an exit event is received the sample will break out of the loop and 
  unregister the notification and exit.

Supported OS:

  Windows 2000, Windows 98

Building:

  Build the sample using the latest Platform SDK via the MAKEFILE included. 
  When using the sample code in a project, be sure to link with the 
  DHCPCSVC.LIB and the IPHLPAPI.LIB libraries. 

Usage:

  The sample can be run directly on the command-line by typing 'DHCPNOTIFY'. 
  An adapter can be specified on the command-line. To specify an adapter on 
  Windows 2000, use the adapter GUID, on Windows 98, the adapter index. These 
  are the values that are obtained via the IP Helper API, GetInterfaceInfo()
  as demonstrated in the code included.

  To provide the sample with an event, use the IPCONFIG utility to release
  or renew the adapter information. For example 'IPCONFIG /RENEW' will
  cause the event to trigger.

  Press CTRL-C to end the sample application.