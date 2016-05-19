!include <win32.mak>

LIBS = rasapi32.lib
proj = RasEnumEntries 

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).exe : $(OUTDIR)\$(proj).obj
	$(link) $(ldebug) $(conlibs) $(LIBS) $(OUTDIR)\$(proj).obj -out:$(OUTDIR)\$(proj).exe 

$(OUTDIR)\$(proj).obj: RasEnumEntries.c
	$(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" RasEnumEntries.c

clean :
	$(CLEANUP)
