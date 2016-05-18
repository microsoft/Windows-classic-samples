THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999  Microsoft Corporation.  All Rights Reserved.



Abstract:

      This application demonstrates a very simple-minded Winsock application
      that uses the native ATM Winsock 2 service provider.  The specific 
      ATM protocol addressing demonstrated by this application is AAL5.  This 
      application makes no attempt to do anything useful per se, but concentrates 
      how setting up a connection using AAL5 and sending some data.  Additionally, 
      this application demonstrates several Winsock2-specific APIs and techniques 
      that are useful whether or not your main interest is ATM or not.

      Commandline parameters determine whether this application functions as 
      either a sender (-n), a receiver (-i), or simply allows ATM interfaces to
      be enumerated.  When specifying the ATM interface to bind to (on the receiver)
      or the remote ATM interface to connect to on the sender the interface
      is a 40 character NSAP address, with the last 2 hex digits (0 - 255 decimal)
      representing the port.  The commandline switch (-a) can be used to enumerate 
      all ATM interface on the local machine - note the last 2 digits will be 00.

      Use the -? commandline switch to determine available options.

      This application simply sends a buffer of data when acting as the sender,
      to an instance of itself acting as a receiver.  Any number of senders (up
      to MAXCLIENTS) can be started to interact with one instance of a receiver.
      The size of the data buffer (-b), its contents (-d), and the number of 
      times to send (-l) this buffer are controllable via commandline switches. 
      A repeat value of 0 implies continuous sending.  A delay 
      commandline switch (-s:#), measured in milliseconds can be specified for the 
      sender to insure the total transfer spans a measured unit of time.  Additionally,
      the commandline switch (-c:#) is available to delay closing the connection,
      measured in milliseconds, to allow the receiver time to pick up the data.  This
      is important because neither AAL5 nor this application has any handshake
      mechanism to determine when the receiver has received all the data - AAL5
      does not support a graceful connection closure.

      

APIs of Interest:
    WSAEnumProtocols 
    WSASocket 
    WSAConnect
    WSAAccept 
    WSAEventSelect
    WSAWaitForMultipleEvents 
    WSASend
    WSAAddressToString
    WSAStringToAddress



Build:

This sample uses the following .cpp and .h files contained in this directory.

    atmevent.h
    atmevent.cpp
    receiver.cpp
    sender.cpp
    enumerator.cpp


To compile/link this sample type nmake and use the supplied makefile or
build a project with VC++.  Please remember to use the headers and libs from 
the May 1999 Platform SDK later and link with ws2_32.lib.  This application
was only tested on the Beta 3 release of Windows 2000.



Usage:

This application is targeted to run on either NT5 Beta 3 (or later) and an
upcoming release of Windows 98 with ATM support.  It is not targeted to run on 
NT4 or Windows 95.  

Usage:
      Enumerate ATM interfaces
          atmevent -a
      Act as a sender and send buffer 10 times
      and wait 2 seconds before closing the connection
          atmevent -n:receiver_atm_interface -l:10 -c:2000
      Act as a receiver
          atmevent -i:local_atm_interface

