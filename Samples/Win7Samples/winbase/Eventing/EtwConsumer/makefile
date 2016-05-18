# Copyright (C) Microsoft Corporation.  All Rights Reserved.

!IF "$(TARGETOS)" != "WINNT" || ("$(APPVER)" < "6.0") && ("$(APPVER)" < "6.01")
!ERROR  Sorry, SimpleProvider is not supported on non NT platforms
!ERROR and is only supported on Windows Vista or higher
!ENDIF

!include <win32.mak>

cflags = $(cflags) /D UNICODE /D _UNICODE

PROJ = EtwConsumer
TDH_UTIL = TdhUtil
COMMON = common

OUTDIR = Output

PROJ_OBJS = $(OUTDIR)\$(PROJ).obj $(OUTDIR)\TdhUtil.obj $(OUTDIR)\common.obj 

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\$(PROJ).obj: $(PROJ).cpp
   $(cc) $(cflags) $(cdebug) $(cvars)		  \
   /Fo$(OUTDIR)\\                                 \
   /Fd$(OUTDIR)\\                                 \
   /I$(OUTDIR)                                    \
   $(PROJ).cpp 

$(OUTDIR)\TdhUtil.obj: $(TDH_UTIL).cpp
   $(cc) $(cflags) $(cdebug) $(cvars)		  \
   /Fo$(OUTDIR)\\                                 \
   /Fd$(OUTDIR)\\                                 \
   /I$(OUTDIR)                                    \
   $(TDH_UTIL).cpp
   
$(OUTDIR)\common.obj: $(COMMON).cpp
   $(cc) $(cflags) $(cdebug) $(cvars)		  \
   /Fo$(OUTDIR)\\                                 \
   /Fd$(OUTDIR)\\                                 \
   /I$(OUTDIR)                                    \
   $(COMMON).cpp

$(OUTDIR)\$(PROJ).exe: $(PROJ_OBJS)
   $(link) $(conlflags) $(linkdebug) \
   $(PROJ_OBJS)			     \
   $(conlibs) tdh.lib ole32.lib      \
   -out:$(OUTDIR)\$(PROJ).exe 

clean:
  $(CLEANUP)

