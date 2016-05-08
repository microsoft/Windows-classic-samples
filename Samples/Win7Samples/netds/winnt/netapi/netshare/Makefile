!include <Win32.Mak>

proj=netshare

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).obj: $(proj).c
    $(cc) $(cflags) $(cvarsmt) $(cdebug) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(proj).c

$(OUTDIR)\$(proj).exe: $(OUTDIR)\$(proj).obj
    $(link) $(ldebug) $(conlflags) $(conlibs) -out:$(OUTDIR)\$(proj).exe $(OUTDIR)\$(proj).obj /PDB:$(OUTDIR)\$(proj).PDB netapi32.lib advapi32.lib

clean:
	$(CLEANUP)

