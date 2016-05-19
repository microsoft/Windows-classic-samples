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
INCLUDE = $(INCLUDE);..\inc

PROJ = dumprec

PROJBASE_FLAGS = /DMSPLOG /DUNICODE /GR /EHsc

!ifndef NODEBUG
PROJBASE_FLAGS = $(PROJBASE_FLAGS) /DDBG /D_DEBUG
!endif

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

PROJ_OBJS  = $(OUTDIR)\dumprec.obj
RC_DEP     = dumprecs.h
EXTRA_LIBS = ..\lib\$(OUTDIR)\cperhlp.lib wevtapi.lib

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Inference rule for updating the object files
.c{$(OUTDIR)}.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $(PROJBASE_FLAGS) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Build rule for resource file
$(OUTDIR)\$(PROJ).res: $(PROJ).rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(PROJ).res $(PROJ).rc

# Build rule for EXE
$(OUTDIR)\$(PROJ).EXE: $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res
    $(link) $(ldebug) $(guilflags) \
    $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res $(guilibs) $(EXTRA_LIBS) \
    -entry:wmainCRTStartup \
    -subsystem:console \
    -out:$(OUTDIR)\$(PROJ).exe $(MAPFILE)

# Rules for cleaning out those old files
clean:
        $(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
