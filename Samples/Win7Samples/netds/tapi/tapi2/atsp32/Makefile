!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\atsp32.tsp

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\atsp.obj: atsp.c atsp.h
    $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" atsp.c

$(OUTDIR)\atsp.res: atsp.rc
    $(rc) $(rcvars) -r -fo $(OUTDIR)\atsp.res atsp.rc

$(OUTDIR)\atsp32.lib: $(OUTDIR)\atsp.obj atsp32.def $(OUTDIR)\atsp.res
     $(implib) -machine:$(CPU) \
     -def:atsp32.def   \
!IF "$(CPU)" == "i386"
     $(OUTDIR)\atsp.obj \
!ENDIF
     -out:$(OUTDIR)\atsp32.lib

$(OUTDIR)\atsp32.tsp: $(OUTDIR)\atsp.obj atsp32.def $(OUTDIR)\atsp.res $(OUTDIR)\atsp32.lib
    $(link) $(linkdebug) $(dlllflags) \
    -base:0x1C000000     \
    $(OUTDIR)\atsp.obj $(OUTDIR)\atsp32.exp -out:$(OUTDIR)\atsp32.dll $(OUTDIR)\atsp.res $(guilibsdll)
    if exist $(OUTDIR)\atsp32.dll.manifest mt.exe -manifest $(OUTDIR)\atsp32.dll.manifest -outputresource:$(OUTDIR)\atsp32.dll;2
    copy $(OUTDIR)\atsp32.dll $(OUTDIR)\atsp32.tsp

clean:
        $(CLEANUP)
