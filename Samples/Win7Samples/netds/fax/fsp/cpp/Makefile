##==========================================================================
##
## THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
## ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
## THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
## PARTICULAR PURPOSE.
##
## Copyright (c) Microsoft Corporation. All rights reserved.
##
##--------------------------------------------------------------------------

# Nmake macros for building Windows 32-Bit apps
!include <win32.mak>

!if ("$(TARGETOS)" == "WINNT") && ("$(CPU)" == "IA64") 

!message Sorry, this is not supported on the IA64 platforms.

!else

PROJ = SampleFSP

all: $(OUTDIR) $(OUTDIR)\$(PROJ).dll

!message $(OUTDIR)
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\SampleFSP.obj $(OUTDIR)\Reg.obj $(OUTDIR)\Util.obj

INCLUDE  = $(DXSDKROOT)\Include;$(INCLUDE)

$(OUTDIR)\SampleFSP.obj : SampleFSP.cpp SampleFSP.h Macros.h 
   $(cc) $(cdebug) $(cflags) /Od /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /Gm /EHsc /RTC1 /c /Wp64 /TP  SampleFSP.cpp $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" 
	
$(OUTDIR)\Reg.obj: Reg.cpp Reg.h  
   $(cc) $(cdebug) $(cflags) /Od /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /Gm /EHsc /RTC1 /c /Wp64 /TP  Reg.cpp $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" 
	
$(OUTDIR)\Util.obj : Util.cpp
   $(cc) $(cdebug) $(cflags) /Od /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /Gm /EHsc /RTC1 /c /Wp64 /TP  Util.cpp $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" 
	
INCLUDE  = $(DXSDKROOT)\Include;$(INCLUDE)

EXTRA_LIBS = $(STRMBASE) kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib

!IFDEF NODEBUG
DEBUG=
!ELSE
DEBUG=/DEBUG /PDB:$(OUTDIR)\SampleFSP.pdb
!ENDIF

# Build rule for DLL
$(OUTDIR)\$(PROJ).DLL: $(BASE_OBJS) $(PROJ_OBJS) SampleFSP.def
    $(link) /OUT:$(OUTDIR)\SampleFSP.dll /INCREMENTAL $(PROJ_OBJS) $(DEBUG) /MACHINE:$(CPU) $(EXTRA_LIBS) -def:SampleFSP.def -base:0x1C000000  $(dlllflags)
# embedd the manifest
    if exist $@.manifest mt.exe -manifest $@.manifest -outputresource:$@;2

# Rules for cleaning out old files
clean:
        $(CLEANUP)
!endif