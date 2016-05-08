# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1996-1999.  Microsoft Corporation.  All rights reserved.
#

!IF "$(TARGETOS)" == "WINNT"

!include <Win32.Mak>

proj=textsid

all: $(proj).exe

.c.obj:
    $(cc) $(cflags) $(cvars) $(cdebug) $<

$(proj).exe: $(proj).obj
    $(link) $(ldebug) $(conlflags) \
    -out:$(proj).exe $(proj).obj \
    $(conlibs) advapi32.lib user32.lib shlwapi.lib
!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
