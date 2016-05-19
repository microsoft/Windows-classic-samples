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

!if "$(TARGETOS)" == "WINNT" && "APPVER"!= "4.0" \
    && ("$(CPU)" != "IA64") && ("$(CPU)" != "AMD64")

PROJ = T3OUT

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\outgoing.obj
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

# Inference rule for updating the object files
.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) -EHsc $(cvars) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $<

# Build rule for resource file
$(OUTDIR)\$(PROJ).res: outgoing.rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\$(PROJ).res outgoing.rc

# Build rule for EXE
$(OUTDIR)\$(PROJ).EXE: $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res
    $(link) $(linkdebug) $(guilflags) \
    $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res $(guilibs) $(EXTRA_LIBS) \
    -out:$(OUTDIR)\$(PROJ).exe $(MAPFILE)
    if exist $(OUTDIR)\$(PROJ).exe.manifest mt.exe -manifest $(OUTDIR)\$(PROJ).exe.manifest -outputresource:$(OUTDIR)\$(PROJ).exe;1


# Rules for cleaning out those old files
clean:
        $(CLEANUP)

!else
!if ("$(CPU)" == "IA64") || ("$(CPU)" == "AMD64")
!message Sorry, this sample is not supported on the 64-bit platforms
!else
!message Sorry this sample is only supported on the Windows 2000 or greater (32-bit)
!endif
!endif