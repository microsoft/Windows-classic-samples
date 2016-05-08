SCQuery illustrates how to use SCard functions to query the smart card database.
In addition, it also makes use of low level SCardTransmit() API to interogate
ATR string and Master File Control Information from inserted smart card(s).

Files included in this sample
=============================

1) ReadMe.txt     - This file
2) SCQuery.cpp    - Main program
3) SCCommon.cpp   - Contains functions called by SCQuery.cpp
4) SCCommon.h     - Header file for SCCommon.cpp
5) MAKEFILE       - Makefile for NMAKE utility

SCard functions used in this sample
===================================

- SCardEstablishContext
- SCardListReaders
- SCardListCards
- SCardListReaderGroups
- SCardGetStatusChange
- SCardConnect
- SCardStatus
- SCardTransmit
- SCardDisconnect
- SCardReleaseContext

How to compile
==============

1) Make sure all smart card hardware and software components have been setup
   correctly per %MSSDK%\Samples\Winbase\SCard\ReadMe.txt.
2) From the SCQuery directory, run NMAKE to make SCQuery.exe.

See %MSSDK%\samples\winbase\scard\ReadMe.txt for more information about
running the SmartCard samples in general.

