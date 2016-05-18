# Nmake macros for building Tst4Wts.Exe
# Define NODEBUG to build Tst4Wts.Exe without debugging information.
# Define UNICODE to build Tst4Wts.Exe with UNICODE characters.

!IF "$(TARGETOS)" == "WINNT"

!include <Win32.Mak>

!ifdef UNICODE
unicode = -DUNICODE
!else
unicode =
!endif

all: $(OUTDIR) $(OUTDIR)\Tst4Wts.Exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the resource if necessary

$(OUTDIR)\Tst4Wts.Res: Tst4Wts.Rc
    $(rc) $(rcvars) -r /fo $(OUTDIR)\tst4wts.res Tst4Wts.Rc

# Update the object files if necessary

$(OUTDIR)\Tst4Wts.Obj: Tst4Wts.c
    $(cc) $(cdebug) $(cflags) $(cvars) $(unicode) /Fo"$(OUTDIR)\\" Tst4Wts.c

# Update the executable file if necessary, and if so, add the resource back in.

$(OUTDIR)\Tst4Wts.Exe: $(OUTDIR)\Tst4Wts.Obj $(OUTDIR)\Tst4Wts.Res makefile
    $(link) $(linkdebug)  $(conflags) -out:$(OUTDIR)\Tst4Wts.Exe\
    $(OUTDIR)\Tst4Wts.Obj $(OUTDIR)\Tst4Wts.Res\
    Version.lib $(conlibsmt)
    
clean:
    del $(OUTDIR)\*.Obj
    del $(OUTDIR)\*.Res
    del $(OUTDIR)\*.Exe
    del $(OUTDIR)\*.pdb

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
