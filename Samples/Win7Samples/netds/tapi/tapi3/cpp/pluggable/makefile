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

INCLUDE = $(DXSDK_DIR)\Include\DShowIDL;$(INCLUDE);$(VCINSTALLDIR)\PlatformSDK\Include
LIB=$(DXSDK_DIR)\lib;$(LIB)

!include <win32.mak>

PROJ = PlgTerm

!ifndef MIDL
MIDL = midl
!endif

all: $(OUTDIR) $(OUTDIR)\PlgTermSample_i.c $(OUTDIR)\$(PROJ).dll 


MIDLFLAGS = /Oicf -char unsigned -ms_ext -c_ext -error all -header PlgTermSample.h  \
            -robust -cpp_cmd $(CC) -DMSPLOG -DUNICODE  -Di386 -D_X86_         \
            -D_WCHAR_T_DEFINED

#Run MIDL compiler to generate the header and definition files
$(OUTDIR)\PlgTermSample.h $(OUTDIR)\PlgTermSample_i.c : PlgTermSample.idl
    $(MIDL) $(MIDLFLAGS) /tlb PlgTermSample.tlb /out $(OUTDIR) PlgTermSample.idl


INCLUDE  = $(DXSDK_DIR)\Include;..\..\..\..\..\..\samples\Multimedia\DirectShow\baseclasses;$(INCLUDE)

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\Pluggable.obj \
	     $(OUTDIR)\PlgTermFilter.obj \
             $(OUTDIR)\PlgTermPin.obj \
             $(OUTDIR)\PlgTerm.obj


BASE_OBJS  =


#
# note: you need to build $(MSSDK)\samples\Multimedia\DirectShow\baseclasses
# to produce UNICODE strmbase.lib or strmbasd.lib file before
# building PlgTerm sample
#

DXDBGOUTDIR = Debug
DXRELOUTDIR = Release

!IFDEF NODEBUG
STRMBASE="..\..\..\..\..\..\samples\Multimedia\DirectShow\baseclasses\$(DXRELOUTDIR)\strmbase.lib" /NODEFAULTLIB:libcmt.lib
!else
STRMBASE="..\..\..\..\..\..\samples\Multimedia\DirectShow\baseclasses\$(DXDBGOUTDIR)\strmbasd.lib" /NODEFAULTLIB:libcmtd.lib
!endif

EXTRA_LIBS = uuid.lib winmm.lib \
	     rtutils.lib $(STRMBASE) 


#
# note: you need to build ..\MSPBase to produce MSPBaseSample.lib file before
# building PlgTerm sample
#

EXTRA_LIBS = $(EXTRA_LIBS) ..\MSP\lib\$(OUTDIR)\MSPBaseSample.lib


GLOBAL_DEP = resource.h
RC_DEP     = resource.h
DEFFILE    = PlgTerm.def

cflags = $(cflags) -DMSPLOG -DUNICODE -GR -EHsc

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Inference rule for updating the object files
#PlgTermSample_i.obj : PlgTermSample.h PlgTermSample_i.c


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

!ELSE
!IF ("$(CPU)" == "IA64") || ("$(CPU)" == "AMD64")
!MESSAGE Sorry, this sample is not supported on the 64-bit platforms.
!ELSE
!MESSAGE Sorry, this sample builds for Windows XP and greater.
!ENDIF
!ENDIF