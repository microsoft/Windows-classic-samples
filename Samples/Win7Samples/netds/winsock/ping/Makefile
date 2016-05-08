!include <win32.mak>

all:$(OUTDIR) $(OUTDIR)\ping.exe

LIBS = user32.lib ws2_32.lib

$(OUTDIR)\resolve.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\"  Resolve.cpp

$(OUTDIR)\Ping.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\"  Ping.cpp

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\ping.exe: $(OUTDIR)\ping.obj $(OUTDIR)\resolve.obj  
    $(link) $(ldebug) $(conlflags) -out:$*.exe $** $(LIBS)

clean:
        $(CLEANUP)

cleanall:clean
	-del *.exe
