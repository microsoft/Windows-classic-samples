From Network Programming for Microsoft Windows, Second Edition by 
Anthony Jones and James Ohlund.  
Copyright 2002.   Reproduced by permission of Microsoft Press.  
All rights reserved.

SUMMARY
======= 

The Ping sample illustrates the use of SOCK_RAW in Winsock 2.0 on Windows NT in 
order to write an ICMP_ECHO utility (commonly seen in the 'ping' program  on 
various systems).

FILES
=====

README.TXT      Readme file
MAKEFILE        Makefile
PING.CPP        Command-line Argument Processing and main()
RESOLVE.CPP     Address resolution and reverse name lookup routines for ping
RESOLVE.H       Header file for the routines in resolve.cpp
IPHDR.H         Definitions of IP and ICMP structures for IPv4 and IPv6

PLATFORMS SUPORTED
==================

Windows 2000 or later (Windows XP or later for IPv6 functionality).


RUNNING THE PING SAMPLE
=======================

To build, type "nmake" at the command line.

To run the application, refer to the following usage:

 Usage:
     ping [options] <host>    
         host           Remote machine to ping
         options:
           -a 4|6       Address family (default AF_UNSPEC)
           -i ttl       Time to live (default: 128)
           -l bytes     Amount of data to send (default: 32)
           -r           Record route (IPv4 only)

  Examples:
    [Remember to invoke .\ping.exe from the directory where the sample ping.exe
     is located instead of just ping.exe to make sure you are using the ping 
     sample and not the standard ping utility that ships with Windows]
     
    .\ping.exe 9.9.9.1
        - pings the machine with the ip 9.9.9.1
        
    .\ping.exe -r 9.9.9.1
        - pings the machine with the ip 9.9.9.1 and also prints the route
        
    .\ping.exe -a 6 3ffe:2900:d005:f282:2d0:b7ff:fe1a:3a09
        - ping the machine with the above IPv6 address
        
    .\ping.exe -a 6 computer1
		- ping the machine 'computer1' using its IPv6 address (if no IPv6
			address is registered for computer1, ping will fail)
    
        
