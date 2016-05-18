!include <win32.mak>

LIBS = user32.lib gdi32.lib snmpapi.lib wsnmp32.lib mgmtapi.lib oldnames.lib

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

all:$(OUTDIR) $(OUTDIR)\wsnmputil.exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\wsnmputil.exe: $(OUTDIR)\wsnmputil.obj $(OUTDIR)\helper.obj
    $(link) $(ldebug) $(conlflags) $(conlibs) -out:$*.exe $** $(LIBS)

clean:   
        $(CLEANUP)

cleanall:clean
	-del *.exe
