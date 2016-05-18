!include <Win32.Mak>

proj=scquery

EXTRA_LIBS = winscard.lib

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).obj: $(proj).cpp
    $(cc) $(cflags) $(cvarsdll) $(cdebug) -I..\include /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(proj).cpp

$(OUTDIR)\sccommon.obj: sccommon.cpp
    $(cc) $(cflags) $(cvarsdll) $(cdebug) -I..\include /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" sccommon.cpp

$(OUTDIR)\$(proj).exe: $(OUTDIR)\$(proj).obj $(OUTDIR)\sccommon.obj
    $(link) $(ldebug) $(conlflags) $(conlibs) $(EXTRA_LIBS) -out:$(OUTDIR)\$(proj).exe $(OUTDIR)\sccommon.obj $(OUTDIR)\$(proj).obj /PDB:$(OUTDIR)\$(proj).PDB

clean:
	$(CLEANUP)
