# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) Microsoft Corporation.  All rights reserved.
#

!IF "$(TARGETOS)" == "WINNT"

!include <Win32.Mak>


all: $(OUTDIR) $(OUTDIR)\simple.exe $(OUTDIR)\client.exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**


## Simple dependencies

simple_objs = $(OUTDIR)\simple.obj $(OUTDIR)\service.obj

$(OUTDIR)\simple.exe: $(simple_objs)
    $(link) $(ldebug) $(conlflags) $(simple_objs) \
    -out:$(OUTDIR)\simple.exe  $(conlibsmt) advapi32.lib shell32.lib

## Client dependencies

$(OUTDIR)\client.exe: $(OUTDIR)\client.obj
    $(link) $(ldebug) $(conlflags) -out:$(OUTDIR)\client.exe $(OUTDIR)\client.obj $(conlibsmt)

clean:
        $(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
