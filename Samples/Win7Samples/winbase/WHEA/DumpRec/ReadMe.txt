What is it?
==========
The DumpRec tool is a utility to retrieve and parse WHEA error records.  DumpRec will parse the event log or an exported
event log file for WHEA error records.  It will parse each record displaying the details of the record to the console.

This sample is divided into two directories.  The first directory contains the files for the command line executable.  
The second directory contains a library of routines used to interpret WHEA error records.  The routines in this library 
may be used in your own applications.



Files
=====
Readme.txt:		This file

Makefile:		Make file

exe\Makefile:		The make file for the command line executable.

exe\dumprec.c:		The main module for the command line executable.

exe\dumprec.h:		The shared definitions and declarations used in the command line executable.

exe\dumprec.rc:		The resource file for the command line executable.

exe\dumprecs.h:		The identifiers for string resources used in the command line executable

inc\cperhlp.h:		The functions provided by the WHEA error record interpretation library.

lib\Makefile:		The make file for the WHEA record interpretation library.

lib\cper.c:		The top level implementation of the functions provided by the WHEA record interpretation library.

lib\cperhlpp.h:		The precompiled header for the WHEA record interpretation library

lib\events.c:		The routines used to extract WHEA error records from the WHEA event log channel.


How to Build
============
Under the same directory where the sample code is present, 

	type "nmake"


How to Run
==========

Run DUMPREC.EXE from a command line.  

Usage:
       dumprec <filename>
       dumprec [/m <computername> [/u <username> <password>|*]]

       <filename> is an exported event log.

       <computername> is the name of a remote computer.
       <username> and <password> are remote computer credentials.



