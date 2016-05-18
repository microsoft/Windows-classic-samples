!include <win32.mak>

LIBS = comctl32.lib rasapi32.lib 

proj = PhoneEntry 

all: $(OUTDIR) $(OUTDIR)\$(proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).exe : $(OUTDIR)\$(proj).obj $(OUTDIR)\$(proj).res
    $(link) $(debug) $(guilibs) $(OUTDIR)\$(proj).obj $(LIBS) -out:$(OUTDIR)\$(proj).exe 

$(OUTDIR)\$(proj).obj: PhoneEntry.c
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" PhoneEntry.c

$(OUTDIR)\$(proj).res : PhoneEntry.rc
    $(rc) $(rcflags) $(rcvars) /fo$(OUTDIR)\$(proj).res PhoneEntry.rc

clean :
	$(CLEANUP)
