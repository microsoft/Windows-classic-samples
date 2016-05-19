# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1999-2003  Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile

# Nmake macros for building Windows 32-Bit apps

!if ("$(TARGETOS)" == "WINNT") && (("$(APPVER)" != "4.0") && ("$(APPVER)" != "5.0")) \
    && ("$(CPU)" != "IA64") && ("$(CPU)" != "AMD64")

!if ("$(DXSDK_DIR)" == "")
!message !!!See Readme.Txt about setting the DXSDK_DIR enviroment var!!!
!endif

# DXSDK_DIR Environment variable points to the Direct X SDK . Use DXSDK_DIR in include path

INCLUDE=$(DXSDK_DIR)\Include\DShowIDL;$(DXSDK_DIR)\Include;$(INCLUDE);$(VCINSTALLDIR)\PlatformSDK\Include
LIB=$(DXSDK_DIR)\lib;$(LIB)

!include <win32.mak>

PROJ = sampmsp

!ifndef MIDL
MIDL = midl
!endif

all: $(OUTDIR) $(OUTDIR)\sampmsp_i.c $(OUTDIR)\$(PROJ).dll 

MIDLFLAGS = /Oicf -char unsigned -ms_ext -c_ext -error all -header sampmsp.h  \
            -robust -cpp_cmd $(CC) -DMSPLOG -DUNICODE  -Di386 -D_X86_         \
            -D_WCHAR_T_DEFINED

#Run MIDL compiler to generate the header and definition files
$(OUTDIR)\sampmsp.h $(OUTDIR)\sampmsp_i.c : sampmsp.idl
    $(MIDL) $(MIDLFLAGS) /tlb sampmsp.tlb /out $(OUTDIR) sampmsp.idl

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\sampmsp.obj \
             $(OUTDIR)\sampaddr.obj \
             $(OUTDIR)\sampcall.obj \
             $(OUTDIR)\sampstrm.obj
BASE_OBJS  =
EXTRA_LIBS = kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib \
             ole32.lib oleaut32.lib uuid.lib winmm.lib \
             strmiids.lib rtutils.lib \
             atl.lib 


#
# note: you need to build ..\MSPBase to produce MSPBaseSample.lib file before
# building SampleMSP
#


EXTRA_LIBS = $(EXTRA_LIBS) ..\lib\$(OUTDIR)\MSPBaseSample.lib

GLOBAL_DEP = resource.h
RC_DEP     = resource.h
DEFFILE    = sampmsp.def

cflags = $(cflags) -DMSPLOG -DUNICODE -GR -EHsc

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Inference rule for updating the object files
.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /I$(OUTDIR) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**


# Build rule for resource file
$(OUTDIR)\$(PROJ).res: $(PROJ).rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /i $(OUTDIR) /fo $(OUTDIR)\$(PROJ).res $(PROJ).rc

# Build rule for DLL
$(OUTDIR)\$(PROJ).DLL: $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res
    $(link) /DLL $(linkdebug) /DEF:$(DEFFILE) $(guilflags) \
    $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res $(EXTRA_LIBS) \
    -out:$(OUTDIR)\$(PROJ).dll $(MAPFILE)
    if exist $(OUTDIR)\$(PROJ).DLL.manifest mt.exe -manifest $(OUTDIR)\$(PROJ).DLL.manifest -outputresource:$(OUTDIR)\$(PROJ).DLL;2

# Rules for cleaning out those old files
clean:
    $(CLEANUP)

!else
!if ("$(CPU)" == "IA64") || ("$(CPU)" == "AMD64")
!message Sorry, this sample is not supported on the 64-bit platforms.
!else
!message Sorry, this sample is only supported on the Windows XP (32-bit) and greater.
!endif
!endif
