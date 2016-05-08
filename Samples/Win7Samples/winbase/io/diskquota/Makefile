# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1999.  Microsoft Corporation.  All rights reserved.


!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>

PROJ1 = DskQuota
cflags = $(cflags) /D_UNICODE /DUNICODE
conlflags = $(conlflags) ole32.lib

all: $(OUTDIR) $(OUTDIR)\$(PROJ1).exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

PROJ1_OBJS = $(OUTDIR)\DskQuota.obj $(OUTDIR)\Commands.obj $(OUTDIR)\PrintMenu.obj $(OUTDIR)\ProcessMenu.obj

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(PROJ1).exe: $(PROJ1_OBJS)
    $(link) $(ldebug) $(conlflags)\
    $(PROJ1_OBJS) \
    -out:$(OUTDIR)\$(PROJ1).exe \
    $(conlibs)

clean:
    del *.bak
    del *.opt
    del *.pdb
    del *.obj

cleaner: clean
    del *.exe
!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
