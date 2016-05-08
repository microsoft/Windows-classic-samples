!include <win32.mak>

LIBS = rasapi32.lib

proj = RasDialAsync 

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"


$(OUTDIR)\$(proj).exe : $(OUTDIR)\$(proj).obj
	$(link) $(ldebug) $(conlibs) $(OUTDIR)\$(proj).obj $(LIBS) -out:$(OUTDIR)\$(proj).exe 

$(OUTDIR)\$(proj).obj: RasDialAsync.c
	$(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" RasDialAsync.c

clean :
	$(CLEANUP)
