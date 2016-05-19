# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1997-2003  Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile

# Nmake macros for building Windows 32-Bit apps
!include <win32.mak>

!IF ("$(TARGETOS)" == "WINNT") && ("$(CPU)" != "IA64") && ("$(CPU)" != "AMD64")

PROJ = T3IN

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe 

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\incoming.obj \
             $(OUTDIR)\callnot.obj
BASE_OBJS  =
EXTRA_LIBS = version.lib ole32.lib oleaut32.lib uuid.lib strmiids.lib
GLOBAL_DEP = resource.h
RC_DEP     = resource.h

!if ("$(DXSDK_DIR)" == "")
!message !!!See Readme.Txt about setting the DXSDK_DIR enviroment var!!!
!endif
# DXSDK_DIR Environment variable points to the Direct X SDK . Use DXSDK_DIR in include path
INCLUDE=$(DXSDK_DIR)\Include;$(INCLUDE)

LIB=$(DXSDK_DIR)\Lib;$(LIB)

CFLG=$(cflags) -DUNICODE

# If $(OUTDIR) does not exist, then create
$(OUTDIR) :
  if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Inference rule for updating the object files
.cpp{$(OUTDIR)}.obj:
  $(cc) $(cdebug) $(CFLG) -EHsc $(cvars) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Build rule for resource file
$(OUTDIR)\$(PROJ).res: incoming.rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(PROJ).res incoming.rc

# Build rule for EXE
$(OUTDIR)\$(PROJ).EXE: $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res
    $(link) $(linkdebug) $(guilflags) \
    $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res $(guilibs) $(EXTRA_LIBS) \
    -out:$(OUTDIR)\$(PROJ).exe $(MAPFILE)


# Rules for cleaning out those old files
clean:
    $(CLEANUP)

!ELSE
!IF ("$(CPU)" == "IA64") || ("$(CPU)" == "AMD64")
!MESSAGE This sample is not supported on a 64-bit platform.
!ELSE
!MESSAGE This sample is only supported for Windows XP (32-bit) platforms or greater.
!ENDIF
!ENDIF