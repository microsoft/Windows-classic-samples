!include <win32.mak>

LIBS = rasapi32.lib

proj = RasDial 

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).exe : $(OUTDIR)\$(proj).obj
	$(link) $(debug) $(conlibs) $(OUTDIR)\$(proj).obj $(LIBS) -out:$(OUTDIR)\sdk$(proj).exe 

$(OUTDIR)\$(proj).obj: RasDial.c
	$(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" RasDial.c

clean :
	$(CLEANUP)
