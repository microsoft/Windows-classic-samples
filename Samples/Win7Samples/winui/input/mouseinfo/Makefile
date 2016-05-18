# Nmake macros for building Windows 32-Bit apps

!IF "$(TARGETOS)" == "WINNT"
!include <win32.mak>

!IF "$(TARGETLANG)" == ""
all: $(OUTDIR) $(OUTDIR)\mousinfo.exe
!ELSE
all: WARN_MSG
!ENDIF


WARN_MSG:
 @echo "Warning: new mouse functionality not available in beta"

# Update the object file if necessary

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\mousinfo.obj: mousinfo.c mousinfo.h
    $(cc) $(cflags) $(cvars) $(cdebug) /WX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" mousinfo.c

# Update the resources if necessary

$(OUTDIR)\mousinfo.res: mousinfo.rc mousinfo.h
    rc $(rcflags) $(rcvars) /fo $(OUTDIR)\mousinfo.res mousinfo.rc

# Update the executable file if necessary, and if so, add the resource back in.

$(OUTDIR)\mousinfo.exe: $(OUTDIR)\mousinfo.obj  $(OUTDIR)\mousinfo.res
    $(link) $(linkdebug) $(guiflags) /out:$(OUTDIR)\mousinfo.exe \
	$(OUTDIR)\mousinfo.obj $(OUTDIR)\mousinfo.res $(guilibs)

clean:
        $(CLEANUP)

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
