# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (C) 1993-1999.  Microsoft Corporation.  All rights reserved.
#
# Nmake macros for building Windows 32-Bit apps
#

!include <Win32.Mak>
INCLUDE = $(INCLUDE);..\inc

LIBRARY = cperhlp

CPERHLPBASE_FLAGS = /DMSPLOG /DUNICODE /GR /EHsc

!ifndef NODEBUG
CPERHLPBASE_FLAGS = $(CPERHLPBASE_FLAGS) /DDBG /D_DEBUG
!endif

all: $(OUTDIR) $(OUTDIR)\$(LIBRARY).lib

CPERHLP_OBJS =  $(OUTDIR)\cper.obj \
                $(OUTDIR)\events.obj

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

.c{$(OUTDIR)}.obj:
    @echo Compiling $(@B).cpp
    $(cc) $(cflags) $(cvars) $(cdebug) $(CPERHLPBASE_FLAGS) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(LIBRARY).lib: $(CPERHLP_OBJS)
    lib -out:$*.lib -machine:$(CPU) $(CPERHLP_OBJS)

