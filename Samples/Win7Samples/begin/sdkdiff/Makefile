# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
# ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
# Copyright (c) Microsoft Corporation.  All Rights Reserved.
#
#
# Processor independent makefile

# Nmake macros for building makefile
!include <win32.mak>

PROJ = SDKDIFF 

all: $(OUTDIR) $(OUTDIR)\sdkdiff.exe $(OUTDIR)\sdkdiff.chm

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"
    
#--------------------------------------------------------------------#
    
OBJS=$(OUTDIR)\bar.obj      \
     $(OUTDIR)\compitem.obj \
     $(OUTDIR)\complist.obj \
     $(OUTDIR)\errorout.obj \
     $(OUTDIR)\file.obj     \
     $(OUTDIR)\findgoto.obj \
     $(OUTDIR)\gfile.obj    \
     $(OUTDIR)\gutils.obj   \
     $(OUTDIR)\line.obj     \
     $(OUTDIR)\list.obj     \
     $(OUTDIR)\profile.obj  \
     $(OUTDIR)\scandir.obj  \
     $(OUTDIR)\section.obj  \
     $(OUTDIR)\status.obj   \
     $(OUTDIR)\table.obj    \
     $(OUTDIR)\tpaint.obj   \
     $(OUTDIR)\tprint.obj   \
     $(OUTDIR)\tree.obj     \
     $(OUTDIR)\tscroll.obj  \
     $(OUTDIR)\utils.obj    \
     $(OUTDIR)\view.obj     \
     $(OUTDIR)\sdkdiff.obj   

PCH=$(OUTDIR)\precomp.obj

ELIBS=shell32.lib htmlhelp.lib

# Build rules for resource files
    
$(OUTDIR)\sdkdiff.res: sdkdiff.rc 
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\sdkdiff.res sdkdiff.rc

$(PCH):    
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\"  /Yc"precomp.h" precomp.cpp

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /Yu"precomp.h" $**
    
$(OUTDIR)\sdkdiff.exe: $(PCH) $(OBJS) $(OUTDIR)\sdkdiff.res
	$(link) $(ldebug) $(guilibs) $(guiflags) /MACHINE:$(CPU) -out:$(OUTDIR)\sdkdiff.exe $(OBJS) $(ELIBS) /PDB:$(OUTDIR)\$(PROJ).PDB $(OUTDIR)\sdkdiff.res 
	
# Build rule for help file
$(OUTDIR)\sdkdiff.chm: 
    copy sdkdiff.chm $(OUTDIR)\sdkdiff.chm

clean:
	$(CLEANUP)
