# Copyright (C) Microsoft Corporation.  All Rights Reserved.

!IF "$(TARGETOS)" != "WINNT" || ("$(APPVER)" < "5.01") && ("$(APPVER)" < "5.0")
!MESSAGE  Sorry, AdvancedProvider is not supported on non NT platforms
!MESSAGE  and is only supported on Windows 2000 or higher
!ELSE

!include <win32.mak>

PROJ = AdvancedProvider
EVENTS = $(PROJ)Events
OUTDIR = Debug
PROJ_OBJS = $(OUTDIR)\$(PROJ).obj $(OUTDIR)\$(PROJ).res

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)


CreateHeader:
   $(mc)                                          \
   -um                                            \
   -h $(OUTDIR)                                   \
   -z $(EVENTS)                                   \
   $(PROJ).man


$(OUTDIR)\$(PROJ).obj: $(PROJ).cpp
   $(cc) $(cflags) $(cdebug) $(cvars)             \
   /Fo$(OUTDIR)\\                                 \
   /Fd$(OUTDIR)\\                                 \
   /I$(OUTDIR)                                    \
   $(PROJ).cpp                                    \

CreateRes:
	$(rc) /r $(PROJ).rc 

CleanupProjDir:
	move $(PROJ).res $(OUTDIR)\$(PROJ).res && del AdvancedProviderEvents*

$(OUTDIR)\$(PROJ).exe: CreateHeader	CreateRes CleanupProjDir\
	$(OUTDIR)\$(PROJ).obj	


$(OUTDIR)\$(PROJ).exe: $(PROJ_OBJS)
   $(link) $(conlflags) $(linkdebug)              \
   $(PROJ_OBJS)                                   \
   $(conlibs) shell32.lib user32.lib              \
   -out:$(OUTDIR)\$(PROJ).exe                   


clean:
        $(CLEANUP)


!ENDIF
