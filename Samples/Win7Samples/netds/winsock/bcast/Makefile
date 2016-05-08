TARGETOS=BOTH

!include <win32.mak>

LIBS = user32.lib ws2_32.lib

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

all:$(OUTDIR) $(OUTDIR)\bcast.exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\bcast.exe:$(OUTDIR)\bcast.obj
    $(link) $(ldebug) $(conlflags) -out:$(OUTDIR)\bcast.exe $(OUTDIR)\bcast.obj $(LIBS)

clean:
        $(CLEANUP)

cleanall:clean
	-del *.exe



