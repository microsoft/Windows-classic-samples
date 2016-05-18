PICKLEP


The PICKLEP program demonstrates data procedure serialization. The program 
demonstrates a fixed buffer style pickling and an incremental style pickling 
depending on the switches being used.

The implicit_handle primitive is used as the pickling handle.

With default switches, the program encodes data to a file. To decode data 
from the file, use -d switch.

FILES
=====

The directory samples\rpc\pickle\picklp contains the following files for
building the distributed application PICKLP:

File          Description

README.TXT    Readme file for the PICKLE sample
PICKLP.IDL    Interface definition language file
PICKLP.ACF    Attribute configuration file
PICKLPC.C     Client main program
MAKEFILE      Nmake file to build for Windows 

-----------------------------------------------------------------------
BUILDING THE CLIENT
-----------------------------------------------------------------------

The following environment variables should already be set for you:

  set CPU=i386
  set INCLUDE=%SDKROOT%\h
  set LIB=%SDKROOT%\lib
  set PATH=%SDKROOT%\system32;%SDKROOT%\bin

where %SDKROOT% is the root directory for the 32-bit Windows SDK.

For mips,  set CPU=mips
For alpha, set CPU=alpha

Build the sample distributed application:

  nmake cleanall
  nmake

This builds the executable programs picklpc.exe.
There is no server side in this sample as the notion of server does not
apply to pickling.


-----------------------
RUNNING THE APPLICATION
-----------------------

The default setting for this application is to encode data to 
pickle.dat file. Enter:

  picklpc

To decode data that has been encoded, enter:

  picklpc -d

Several other command line switches are available to change settings for 
this program. For a listing of the switches available from the client 
program, enter:

  picklpc -?
