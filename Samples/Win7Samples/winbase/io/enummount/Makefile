# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) Microsoft Corporation.  All rights reserved.


!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>

PROJ1 = EnumMountPoints

all: $(OUTDIR) $(OUTDIR)\$(PROJ1).exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

PROJ1_OBJS = $(OUTDIR)\EnumMountPoints.obj

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $<

$(OUTDIR)\$(PROJ1).exe: $(PROJ1_OBJS)
    $(link) $(ldebug) $(conlflags)\
    $(PROJ1_OBJS) \
    -out:$(OUTDIR)\$(PROJ1).exe \
    $(conlibs)

clean:
    del $(OUTDIR)\*.bak
    del $(OUTDIR)\*.opt
    del $(OUTDIR)\*.pdb
    del $(OUTDIR)\*.obj

cleaner: clean
    del $(OUTDIR)\*.exe

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
