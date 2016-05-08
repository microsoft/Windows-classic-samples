TARGETOS=BOTH
!include <win32.mak>

mainobjs=$(OUTDIR)\atmevent.obj $(OUTDIR)\sender.obj $(OUTDIR)\receiver.obj $(OUTDIR)\enumerator.obj

all: $(OUTDIR) $(OUTDIR)\atmevent.exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the resource if necessary

# Update the object file if necessary

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

# Update the executable file if necessary, and if so, add the resource back in.

$(OUTDIR)\atmevent.exe: $(mainobjs)  
    $(link) $(linkdebug) $(conlflags) -out:$(OUTDIR)\atmevent.exe $(mainobjs) $(conlibsmt) ws2_32.lib


clean:
        $(CLEANUP)
