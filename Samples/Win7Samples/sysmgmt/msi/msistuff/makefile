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
# Target only i386

!if "$(CPU)" == "IA64"
!message Sorry. Building this sample for the IA64 platform is not currently supported.
!elseif "$(CPU)" == "AMD64"
!message Sorry. Building this sample for the AMD64 platform is not currently supported.
!else
# Makefile for i386
# Only target WinNT
TARGETOS=WINNT

# Nmake macros for building Win32 apps
!include <win32.mak>

PROJ = msistuff

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\msistuff.obj
BASE_OBJS  = 
EXTRA_LIBS = 
GLOBAL_DEP = 
RC_DEP     = 

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Inference rule for updating the object files
.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Build rule for resource file
$(OUTDIR)\$(PROJ).res: $(PROJ).rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(PROJ).res $(PROJ).rc

# Build rule for EXE
$(OUTDIR)\$(PROJ).EXE: $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res
    $(link) $(linkdebug) $(conlflags) \
    $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res $(guilibs) $(EXTRA_LIBS) \
    -out:$(OUTDIR)\$(PROJ).exe $(MAPFILE)

# Rules for cleaning out those old files
clean:
        $(CLEANUP)

!endif