!IF "$(TARGETOS)" == "WINNT"
!include <win32.mak>

LIBS = rasapi32.lib

proj = customentry 

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).exe : $(OUTDIR)\$(proj).obj
	$(link) $(ldebug) $(lconflags) $(conlibsmt) $** $(LIBS) /PDB:$(OUTDIR)\$(PROJ).PDB -out:$(OUTDIR)\$(proj).exe 

$(OUTDIR)\$(proj).obj: customentry.c
	$(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" customentry.c

clean :
	$(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
