!include <win32.mak>

LIBS = rasapi32.lib
proj = RasEnumDevices 

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).exe : $(OUTDIR)\$(proj).obj
	$(link) $(ldebug) $(conlibs) $(LIBS) $(OUTDIR)\$(proj).obj -out:$(OUTDIR)\$(proj).exe 

$(OUTDIR)\$(proj).obj: RasEnumDevices.c
	$(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" RasEnumDevices.c

clean :
	$(CLEANUP)
