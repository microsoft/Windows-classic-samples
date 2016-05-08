# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1995-1999  Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile

# Nmake macros for building Windows 32-Bit apps

!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>

PROJ = appmema

all: $(OUTDIR) $(OUTDIR)\$(PROJ).dll

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)
    if not exist "..\pub\$(OUTDIR)/$(NULL)" mkdir ..\pub\$(OUTDIR)



# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\$(PROJ).obj $(OUTDIR)\dllinit.obj $(OUTDIR)\$(PROJ).res
BASE_OBJS  =
EXTRA_LIBS = advapi32.lib ..\perfdll\$(OUTDIR)\sharemem.obj
GLOBAL_DEP = $(PROJ).h
RC_DEP     =


# Inference rule for updating the object files
.c{$(OUTDIR)}.obj:
  $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**


# Build rule for resource file
$(OUTDIR)\$(PROJ).res: $(PROJ).rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(PROJ).res $(PROJ).rc

# Build rule for DLL
$(OUTDIR)\$(PROJ).DLL: $(BASE_OBJS) $(PROJ_OBJS) 
    $(link) $(ldebug) $(dlllflags) $(BASE_OBJS) $(PROJ_OBJS) \
    $(guilibsdll) $(EXTRA_LIBS) \
    -out:..\pub\$(OUTDIR)\$(PROJ).dll /DEF:$(PROJ).def



# Rules for cleaning out those old files
clean:
        $(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
