!include <Win32.Mak>

proj=lsaprivs

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).obj: $(proj).c
    $(cc) $(cflags) $(cvarsdll) $(cdebug) -I..\include /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(proj).c

$(OUTDIR)\$(proj).exe: $(OUTDIR)\$(proj).obj
    $(link) $(ldebug) $(conlflags) $(conlibs) -out:$(OUTDIR)\$(proj).exe $(OUTDIR)\$(proj).obj /PDB:$(OUTDIR)\$(proj).PDB

clean:
	$(CLEANUP)
