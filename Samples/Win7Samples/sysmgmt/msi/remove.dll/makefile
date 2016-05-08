# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (c) Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile for Platform SDK
#
# Target both WinNT
TARGETOS=WINNT

# Nmake macros for building Win32 apps
!include <win32.mak>

PROJ = remove

all: $(OUTDIR) $(OUTDIR)\$(PROJ).dll

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\remove.obj
BASE_OBJS  =
EXTRA_LIBS = version.lib comctl32.lib urlmon.lib wininet.lib msi.lib netapi32.lib
GLOBAL_DEP =
RC_DEP     =
CL_PRE 	   = /DUNICODE=1

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Inference rule for updating the object files
.cpp{$(OUTDIR)}.obj:
    $(cc) $(CL_PRE) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Build rule for resource file
$(OUTDIR)\$(PROJ).res: $(PROJ).rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(PROJ).res $(PROJ).rc

# Build rule for DLL
$(OUTDIR)\$(PROJ).DLL: $(BASE_OBJS) $(PROJ_OBJS)
    $(link) $(ldebug) $(dlllflags) $(guilibsdll) \
    $(BASE_OBJS) $(PROJ_OBJS) $(EXTRA_LIBS) \
    -out:$(OUTDIR)\$(PROJ).DLL $(MAPFILE)

# Rules for cleaning out those old files
clean:
        $(CLEANUP)
