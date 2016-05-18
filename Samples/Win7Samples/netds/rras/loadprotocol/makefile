!include <win32.mak>

LIBS = kernel32.lib user32.lib mprapi.lib 

proj=LoadProtocol

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"


$(OUTDIR)\$(proj).exe : $(OUTDIR)\LoadProtocol.obj
	$(link) $(debug) $(conlflags) $(conlibs) $** $(LIBS) -out:$(OUTDIR)\$(proj).exe /PDB:$(OUTDIR)\$(proj).PDB 

$(OUTDIR)\$(proj).obj: LoadProtocol.c
	$(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" LoadProtocol.c

clean:
	$(CLEANUP)
