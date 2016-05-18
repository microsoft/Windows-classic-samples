!include <win32.mak>

LIBS = rasapi32.lib

proj = RasGetConnectionStatistics 

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).exe : $(OUTDIR)\$(proj).obj
	$(link) $(ldebug) $(conlibs) $(OUTDIR)\$(proj).obj $(LIBS) -out:$(OUTDIR)\$(proj).exe 

$(OUTDIR)\$(proj).obj: RasGetConnectionStatistics.c
	$(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" RasGetConnectionStatistics.c

clean :
	$(CLEANUP)
