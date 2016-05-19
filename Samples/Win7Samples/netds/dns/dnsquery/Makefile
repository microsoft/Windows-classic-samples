#THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
#ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
#THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
#PARTICULAR PURPOSE.
#
# Copyright © 2000  Microsoft Corporation.  All Rights Reserved.

! include <win32.mak>

LIBS = dnsapi.lib 

.cpp.obj:
	$(cc) $(cdebug) $(cflags) $(cvarsmt) $*.cpp
all:Dnsquery.exe 

dnsquery.exe:Dnsquery.obj
	$(link) $(ldebug) $(conlflags)  -out:$*.exe $*.obj user32.lib $(conlibsmt) $(LIBS)


clean:
	-del *.obj
        -del *.pdb
       

cleanall:clean
	-del *.exe
