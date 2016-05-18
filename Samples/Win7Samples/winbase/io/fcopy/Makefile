# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1999 Microsoft Corporation.  All rights reserved.
#

!include <win32.mak>

PROJ = FCOPY

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

PROJ_OBJS = $(OUTDIR)\fcopy.obj

.cpp{$(OUTDIR)}.obj:
   $(cc) $(cdebug) $(cflags) $(cvars) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(PROJ).exe: $(PROJ_OBJS)
   $(link) $(ldebug) $(conlflags) $(PROJ_OBJS) $(conlibs) -out:$(OUTDIR)\$(PROJ).exe

clean:
        $(CLEANUP)

