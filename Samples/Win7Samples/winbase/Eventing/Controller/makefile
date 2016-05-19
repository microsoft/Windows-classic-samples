# Copyright (C) Microsoft Corporation.  All Rights Reserved.

!IF "$(TARGETOS)" != "WINNT" || ("$(APPVER)" < "5.01") && ("$(APPVER)" < "5.0")
!ERROR  Sorry, Tracelog is not supported on non NT platforms
!ERROR and is only supported on Windows 2000 or higher
!ENDIF

!include <win32.mak>

PROJ = Tracelog

OUTDIR = Output

PROJ_OBJS = $(OUTDIR)\$(PROJ).obj

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)


$(OUTDIR)\$(PROJ).obj: $(PROJ).c
   $(cc) $(cflags) $(cdebug) $(cvars)             \
   /Fo$(OUTDIR)\\                                 \
   /Fd$(OUTDIR)\\                                 \
   /I$(OUTDIR)                                    \
   $(PROJ).c

$(OUTDIR)\$(PROJ).exe: $(PROJ_OBJS)
   $(link) $(conlflags) $(linkdebug) \
   $(PROJ_OBJS)						 \
   $(conlibs) shell32.lib user32.lib \
   -out:$(OUTDIR)\$(PROJ).exe 

clean:
	$(CLEANUP)

