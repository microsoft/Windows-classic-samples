---
page_type: sample
languages:
- c
products:
- windows-api-win32
name: INOUT sample
urlFragment: inout
extendedZipContent:
- path: LICENSE
  target: LICENSE
description: The INOUT program demonstrates the use of [in,out] parameters.
---

# INOUT sample

The INOUT program demonstrates the use of [in,out] parameters.

## FILES

The directory samples\rpc\data\inout contains the following files for building the sample distributed application INOUT:

File          Description

README.TXT    Readme file for the INOUT sample
INOUT.IDL     Interface definition language file
INOUT.ACF     Attribute configuration file
INOUTC.C      Client main program
INOUTS.C      Server main program
INOUTP.C      Remote procedures
MAKEFILE      Nmake file to build for Windows

## BUILDING CLIENT AND SERVER APPLICATIONS

The following environment variables should be already set for you:

  set CPU=i386
  set INCLUDE=%SDKROOT%\h
  set LIB=%SDKROOT%\lib
  set PATH=%SDKROOT%\system32;%SDKROOT%\bin

where %SDKROOT% is the root directory for the 32-bit Windows SDK.

For mips, set CPU=mips
For alpha, set CPU=alpha

Build the sample distributed application:

  nmake cleanall
  nmake

This builds the executable programs inoutc.exe (client) and 
inouts.exe (server).

RUNNING THE CLIENT AND SERVER APPLICATIONS

On the server, enter:

  inouts

On the client, enter:

  net start workstation
  inoutc

Note: The client and server applications can run on the same Microsoft Windows NT computer when you use different screen groups.

Several command-line switches are available to change settings for this program. For a listing of the switches available from the client program, enter:

  inoutc -?

For a listing of switches available from the server program, enter:

  inouts -?
