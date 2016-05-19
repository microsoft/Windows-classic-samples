# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile

# Nmake macros for building Windows 32-Bit apps

!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>

PROJ = STATLIST

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe $(OUTDIR)\$(PROJ).hlp

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\statlist.obj $(OUTDIR)\aboutdlg.obj $(OUTDIR)\mainwnd.obj $(OUTDIR)\winutils.obj
BASE_OBJS  =
EXTRA_LIBS = version.lib pdh.lib
GLOBAL_DEP = statlist.h resource.h
RC_DEP     = resource.h



# Inference rule for updating the object files
.c{$(OUTDIR)}.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Build rule for resource file
$(OUTDIR)\$(PROJ).res: $(PROJ).rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(PROJ).res $(PROJ).rc

# Build rule for EXE
$(OUTDIR)\$(PROJ).EXE: $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res
    $(link) $(ldebug) $(guilflags) \
    $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res $(guilibs) $(EXTRA_LIBS) \
    -out:$(OUTDIR)\$(PROJ).exe $(MAPFILE)

# Build rule for help file
$(OUTDIR)\$(PROJ).hlp: $(PROJ).rtf $(PROJ).hpj
    copy $(PROJ).hlp $(OUTDIR)

# Rules for cleaning out those old files
clean:
        $(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
