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
# Target both Win9X and WinNT
TARGETOS=BOTH

# Target IE 4.0 and above
_WIN32_IE=0x0400

# Nmake macros for building Win32 apps
!include <win32.mak>

PROJ = setup

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\setup.obj $(OUTDIR)\setupui.obj $(OUTDIR)\upgrdmsi.obj $(OUTDIR)\utils.obj $(OUTDIR)\vertrust.obj
BASE_OBJS  =
EXTRA_LIBS = version.lib comctl32.lib urlmon.lib wininet.lib
GLOBAL_DEP = common.h resource.h setup.h setupui.h
RC_DEP     = resource.h

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
    $(link) $(linkdebug) $(guilflags) \
    $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res $(guilibs) $(EXTRA_LIBS) \
    -out:$(OUTDIR)\$(PROJ).exe $(MAPFILE)

# Rules for cleaning out those old files
clean:
        $(CLEANUP)

!endif