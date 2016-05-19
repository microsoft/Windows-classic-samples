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

DHCPRequest
-----------

Abstract:

  This sample demonstrates requesting parameters from the DHCP Client. 
  The parameters requested will either come from the DHCP Client cache 
  or the DHCP Client will request them from the DHCP Server via a
  DHCP_INFORM packet. Please note that not all DHCP Servers support the
  DHCP_INFORM packet so if no data is received either the server does
  not support it or the client timed out waiting for a response.

  This sample demonstrates requesting the Subnet Mask 
  (OPTION_SUBNET_MASK = 1) and the Gateway Address 
  (OPTION_ROUTER_ADDRESS = 3). The sample allocates an arbitrary length
  buffer for the API call and then will adjust it if necessary based on 
  the return value of the call. The data is then read from the data 
  members of the DHCPCAPI_PARAMS array structure. Note that the pointers
  in this structure point into the buffer passed into the API call. It
  is up to the caller to know what format the data is expected in and
  use it appropriately from there. This sample will convert the data to 
  a readable IP Address via the Winsock inet_ntoa() call. 

  RFC 2132 - "DHCP Options and BOOTP Vendor Extensions" provide descriptions 
  of the options and the data types that may be returned with a specific
  option. The header file for the DHCP API (DHCPCSDK.H) defines options 
  OPTION_SUBNET_MASK (1) through OPTION_BOOTFILE_NAME (67).

Supported OS:

  Windows 2000, Windows 98

Building:

  Build the sample using the latest Platform SDK via the MAKEFILE included. 
  When using the sample code in a project, be sure to link with the 
  DHCPCSVC.LIB and the IPHLPAPI.LIB libraries. 

Usage:

  The sample can be run directly on the command-line by typing 'DHCPREQUEST'. 
  An adapter can be specified on the command-line. To specify an adapter on 
  Windows 2000, use the adapter GUID, on Windows 98, the adapter index. These 
  are the values that are obtained via the IP Helper API, GetInterfaceInfo()
  as demonstrated in the code included.
