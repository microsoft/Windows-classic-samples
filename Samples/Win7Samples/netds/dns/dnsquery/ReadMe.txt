========
SUMMARY:
========
  The Dnsquery sample demonstrates the use of 
  DnsQuery() function to send queries to a DNS 
  server to resolve the host name to an IP address
  and vice versa.

============
DESCRIPTION:
============
  This sample sends a query to a DNS server to 
  query host (A) record or PTR record set. 
   
  1.To resolve a host name to an Ip address run 
  the program as follows:
  >Dnsquery -n host1 -t A -s 1.1.1.1( optional)


  2.To resolve an IP address for example 1.2.3.4
  to a name run the program as follows:
  >Dnsquery -n 4.3.2.1.in-addr.arpa -t PTR  -s 1.1.1.1(optional)



====================
SUPPORTED PLATFORMS:
====================

  Windows 2000

=========
Building:
=========
  To build, type "nmake" at the command line. 
  When using the sample code in a project, be 
  sure to link with the DNSAPI.LIB and the 
  WS2_32.LIB libraries. 

=====
FILES
=====

  The directory contains the following files:

   File                       Description

  README.TXT                This file
  MAKEFILE                  Project make file
  dnsquery.cpp              DnsQuery() main program


