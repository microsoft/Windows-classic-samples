# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1996-1999.  Microsoft Corporation.  All rights reserved.
#
# Nmake macros for building Windows 32-Bit apps

!IF "$(TARGETOS)" == "WINNT"

!include <Win32.Mak>

all: $(OUTDIR) $(OUTDIR)\proclist.exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\proclist.obj: proclist.c
    $(cc) $(cdebug) $(cflags) $(cvars) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" proclist.c

$(OUTDIR)\proclist.res: proclist.rc
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\proclist.res proclist.rc

$(OUTDIR)\proclist.exe: $(OUTDIR)\proclist.obj $(OUTDIR)\proclist.res
    $(link) $(ldebug) $(conlflags) -out:$(OUTDIR)\proclist.exe $(OUTDIR)\proclist.obj $(OUTDIR)\proclist.res $(conlibs) pdh.lib

clean:
        $(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
