include <win32.mak>

proj = CustomDial 

LIBS = rasapi32.lib

all: $(OUTDIR) $(OUTDIR)\customdial.dll

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\$(proj).obj: CustomDial.cpp
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" CustomDial.cpp

$(OUTDIR)\Customdialreg.obj: CustomDialreg.cpp
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" CustomDialreg.cpp

$(OUTDIR)\customdial.dll: $(OUTDIR)\Customdial.obj $(OUTDIR)\Customdialreg.obj $(OUTDIR)\customdial.res
    $(link) $(ldebug) $(dlllflags) $(guilibs) -def:customdial.def -out:$*.dll $** $(LIBS)

$(OUTDIR)\customdial.res: customdial.rc
    $(rc) $(rcflags) $(rcvars) /fo$(OUTDIR)\$(proj).res CustomDial.rc

clean:
	$(CLEANUP)
