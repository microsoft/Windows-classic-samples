# Copyright (C) Microsoft Corporation.  All Rights Reserved.

!IF "$(TARGETOS)" != "WINNT" || ("$(APPVER)" < "6.0") && ("$(APPVER)" < "6.01")
!ERROR  Sorry, SimpleProvider is not supported on non NT platforms
!ERROR and is only supported on Windows Vista or higher
!ELSE

!include <win32.mak>

PROJ = gas
OUTDIR = Output
PROJ_OBJS = $(OUTDIR)\$(PROJ).obj $(OUTDIR)\$(PROJ).res

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

CreateHeader:
    ctrpp.exe -o $(PROJ)Counters.h -rc $(PROJ)Counters.rc $(PROJ).man


$(OUTDIR)\$(PROJ).obj: $(PROJ).c
   $(cc) $(cflags) $(cdebug) $(cvars) \
   /WX  \
   /Fo$(OUTDIR)\\                                 \
   /Fd$(OUTDIR)\\                                 \
   /I$(OUTDIR)                                    \
   $(PROJ).c

CreateRes:
	$(rc) /r $(PROJ).rc  

CleanupProjDir:
	move $(PROJ).res $(OUTDIR)\$(PROJ).res && move $(PROJ)COunter* $(OUTDIR)

$(OUTDIR)\$(PROJ).exe: CreateHeader CreateRes \
	$(OUTDIR)\$(PROJ).obj CleanupProjDir


$(OUTDIR)\$(PROJ).exe: $(PROJ_OBJS)
   $(link) $(conlflags) $(linkdebug)              \
   $(PROJ_OBJS)                                   \
   $(conlibs) shell32.lib user32.lib              \
   -out:$(OUTDIR)\$(PROJ).exe                   


        

clean:
        $(CLEANUP)
        

!ENDIF

