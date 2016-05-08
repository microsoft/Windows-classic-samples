========
SUMMARY:
========
  The Modifyrecords sample demonstrates the 
  use of DnsModifyRecordsInSet() function to
  add the resource records.

============
DESCRIPTION:
============
  This sample demonstrates how to add Host
  records (A) and CNAME resource records.

  The following is the example of a Host record
  host1.example.microsoft.com.     IN  A  127.0.0.1

  To add host record run the program as follows:
  >Modifyrecords -n host1.example.microsoft.com -t A -l 360 -d 127.0.0.1 -s 1.1.1.1( optional)

  The following is the example of a Canonical name (CNAME) resource record.
  aliasname.example.microsoft.com.   CNAME   1 truename.example.microsoft.com.

  To add CNAME record run the program as follows:
  >Modifyrecords -n aliasname.example.microsoft.com -t CNAME -l 360 -d truename.example.microsoft.com -s 1.1.1.1(optional)


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
  Modifyrecords.cpp         DnsModifyRecordsInSet() main program





