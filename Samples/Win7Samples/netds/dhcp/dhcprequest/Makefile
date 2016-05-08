#THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
#ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
#THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
#PARTICULAR PURPOSE.
#
#Copyright (C) 1999  Microsoft Corporation.  All Rights Reserved.

include <win32.mak>

LIBS = iphlpapi.lib dhcpcsvc.lib

.cpp.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $*.cpp

all:dhcprequest.exe

dhcprequest.exe:dhcprequest.obj
    $(link) $(ldebug) $(conlflags) $(conlibs) -out:$*.exe $** $(LIBS)

clean:
	-del *.obj 
	-del *.pdb 
	-del *.ilk

cleanall:clean
	-del *.exe
