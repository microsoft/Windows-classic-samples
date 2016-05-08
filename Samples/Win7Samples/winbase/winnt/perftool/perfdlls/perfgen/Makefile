# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1993 - 1995  Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile

# Nmake macros for building Windows 32-Bit apps
!include <win32.mak>

PROJ = PERFGEN

all: $(OUTDIR) $(OUTDIR)\$(PROJ).dll

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Define project specific macros
PROJ_OBJS  = $(OUTDIR)\perfutil.obj $(OUTDIR)\datagen.obj $(OUTDIR)\perfgen.obj 
BASE_OBJS  =
EXTRA_LIBS = 
GLOBAL_DEP = datagen.h genctrnm.h perfmsg.h perfutil.h
RC_DEP     = 


# Inference rule for updating the object files
.c{$(OUTDIR)}.obj:
  $(cc) $(cdebug) $(cflags) $(cvarsdll) /I$(OUTDIR) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Build rule for resource file
$(OUTDIR)\$(PROJ).res: $(OUTDIR)\genctrs.rc $(OUTDIR)\msg00001.bin $(PROJ).rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /i $(OUTDIR) /fo $(OUTDIR)\$(PROJ).res $(PROJ).rc

# Build rule for message file
$(OUTDIR)\genctrs.h $(OUTDIR)\genctrs.rc $(OUTDIR)\msg00001.bin: genctrs.mc
    mc -r $(OUTDIR) -h $(OUTDIR) -v genctrs.mc

# Build rule for DLL
$(OUTDIR)\$(PROJ).DLL: $(OUTDIR)\$(PROJ).res $(BASE_OBJS) $(PROJ_OBJS) 
    $(link) $(linkdebug) $(dlllflags)\
    $(BASE_OBJS) $(PROJ_OBJS) $(OUTDIR)\$(PROJ).res $(winlibs) $(EXTRA_LIBS) \
    -out:$(OUTDIR)\$(PROJ).dll /DEF:perfgen.def /MACHINE:$(CPU)


# Rules for cleaning out those old files
clean:
        $(CLEANUP)
