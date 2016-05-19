!include <win32.mak>

LIBS = kernel32.lib user32.lib oldnames.lib ws2_32.lib mswsock.lib

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\"  $**

all:$(OUTDIR) $(OUTDIR)\iocpserverex.exe  

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\iocpserverex.exe:  $(OUTDIR)\iocpserverex.obj
    $(link) $(ldebug) $(conlflags) -out:$*.exe $** $(LIBS)


clean:
        $(CLEANUP)

cleanall:clean
	-del *.exe
