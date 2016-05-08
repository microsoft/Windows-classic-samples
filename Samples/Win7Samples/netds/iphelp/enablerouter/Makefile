# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright 1993 - 1998 Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile

# Nmake macros for building Win32 apps
!include <win32.mak>

PROJ = EnableRouter

all: $(PROJ).exe

# Define project specific macros
PROJ_OBJS  = $(PROJ).obj
BASE_OBJS  =
EXTRA_LIBS = 
GLOBAL_DEP = 
RC_DEP     = 



# Inference rule for updating the object files
.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c

# Build rule for resource file
$(PROJ).res: $(PROJ).rc $(RC_DEP)
    $(rc) $(rcflags) $(rcvars) /fo $(PROJ).res $(PROJ).rc

# Build rule for EXE
$(PROJ).EXE: $(BASE_OBJS) $(PROJ_OBJS) $(PROJ).res
    $(link) $(linkdebug) $(conlflags) \
    $(BASE_OBJS) $(PROJ_OBJS) $(PROJ).res $(conlibs) $(EXTRA_LIBS) \
    -out:$(PROJ).exe $(MAPFILE)

# Rules for cleaning out those old files
clean:
    del *.bak *.pdb *.obj *.res *.exp *.map *.sbr *.bsc
