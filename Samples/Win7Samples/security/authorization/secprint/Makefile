# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1997-1999 Microsoft Corporation.  All Rights Reserved.

!include <win32.mak>

.c.obj:
        $(cc) $(cflags) $(cdebug) $(cvarsmt) $<

.obj.exe:
        $(link) $(conlflags) $(ldebug) $*.obj \
        $(conlibsmt) winspool.lib user32.lib

#dependencies
secprint.exe: secprint.obj
secprint.obj: secprint.c

clean:
        del *.obj
        del *.pdb
        del *.bak

cleaner: clean
        del secprint.exe
