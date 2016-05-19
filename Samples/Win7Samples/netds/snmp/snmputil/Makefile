#----- Nmake macros for building Windows 32-Bit apps

!include <win32.mak>

!if "$(CPU)" == "i386"
cflags = $(cflags) -D_CRTAPI1=_cdecl -D_CRTAPI2=_cdecl
!else
cflags = $(cflags) -D_CRTAPI1= -D_CRTAPI2=
!endif

#----- Build object files if necessary
.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

all:$(OUTDIR) $(OUTDIR)\snmputil.exe

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

#----- Build executable file
$(OUTDIR)\snmputil.exe: $(OUTDIR)\snmputil.obj
    $(link) $(linkdebug) $(conflags) -out:$(OUTDIR)\snmputil.exe $(OUTDIR)\snmputil.obj $(conlibsdll) \
    advapi32.lib snmpapi.lib mgmtapi.lib

clean:   
        $(CLEANUP)

cleanall:clean
	-del *.exe