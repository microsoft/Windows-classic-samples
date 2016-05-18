# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1993 - 1999  Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile

# Nmake macros for building Windows 32-Bit apps

!IF "$(TARGETOS)" == "WINNT"
!include <win32.mak>

PROJ = appmem

all: $(OUTDIR) $(OUTDIR)\$(PROJ).dll

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)
    if not exist "..\pub\$(OUTDIR)/$(NULL)" mkdir ..\pub\$(OUTDIR)

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\perfutil.obj $(OUTDIR)\perfmem.obj $(OUTDIR)\datamem.obj $(OUTDIR)\sharemem.obj
BASE_OBJS  = $(OUTDIR)\perfmem.res
EXTRA_LIBS = advapi32.lib
GLOBAL_DEP = datamem.h perfmsg.h pertutil.h
RC_DEP     =


# Inference rule for updating the object files
.c{$(OUTDIR)}.obj:
  $(cc) $(cdebug) $(cflags) $(cvarsdll) /I$(OUTDIR) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**


# Build rule for resource file
$(OUTDIR)\perfmem.res: perfmem.rc $(OUTDIR)\memctrs.rc $(OUTDIR)\msg00001.bin $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /i $(OUTDIR) /fo $(OUTDIR)\perfmem.res perfmem.rc

# Build rule for message file
$(OUTDIR)\memctrs.h $(OUTDIR)\memctrs.rc $(OUTDIR)\msg00001.bin: memctrs.mc
   $(mc) -r $(OUTDIR) -h $(OUTDIR) -v memctrs.mc

# Build rule for DLL
$(OUTDIR)\$(PROJ).DLL: $(BASE_OBJS) $(PROJ_OBJS)
    $(link) $(ldebug) $(dlllflags) \
    $(BASE_OBJS) $(PROJ_OBJS) \
    $(guilibsdll) $(EXTRA_LIBS) \
    -out:..\pub\$(OUTDIR)\$(PROJ).dll /DEF:appmem.def /machine:$(CPU)


# Rules for cleaning out those old files
clean:
        $(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
