# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1998.  Microsoft Corporation.  All rights reserved.

!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>

PROJ1 = mount
PROJ2 = unmount

all: $(OUTDIR) $(OUTDIR)\$(PROJ1).exe $(OUTDIR)\$(PROJ2).exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

PROJ1_OBJS = $(OUTDIR)\mount.obj
PROJ2_OBJS = $(OUTDIR)\unmount.obj

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(PROJ1).exe: $(PROJ1_OBJS)
    $(link) $(ldebug) $(conlflags)\
    $(PROJ1_OBJS) \
    -out:$(OUTDIR)\$(PROJ1).exe \
    $(conlibs)


$(OUTDIR)\$(PROJ2).exe: $(PROJ2_OBJS)
    $(link) $(ldebug) $(conlflags)\
    $(PROJ2_OBJS) \
    -out:$(OUTDIR)\$(PROJ2).exe \
    $(conlibs)

clean:
        $(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
