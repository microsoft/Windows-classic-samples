# Nmake macros for building Windows 32-Bit apps

!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\demo.exe $(OUTDIR)\select.dll

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the object files if necessary

$(OUTDIR)\demo.obj: demo.c
    $(cc) $(cflags) $(cvarsdll) $(cdebug) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" demo.c

$(OUTDIR)\select.obj: select.c
    $(cc) $(cflags) $(cvarsdll) $(cdebug) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" select.c

# Update the resources if necessary

$(OUTDIR)\demo.res: demo.rc demo.h
    $(rc) $(rcflags) $(rcvars) /fo$(OUTDIR)\demo.res  demo.rc

# Update the dynamic link library

$(OUTDIR)\select.dll: $(OUTDIR)\select.obj select.def
    $(link) $(linkdebug) $(dlllflags)   -out:$(OUTDIR)\select.dll /DEF:select.def $(OUTDIR)\select.obj $(guilibsdll)
    mt -manifest $(OUTDIR)\Select.dll.manifest -outputresource:$(OUTDIR)\Select.dll;2


# Update the executable file if necessary.
# If so, add the resource back in.

$(OUTDIR)\demo.exe: $(OUTDIR)\demo.obj $(OUTDIR)\select.dll $(OUTDIR)\demo.res
    $(link) $(linkdebug) $(guiflags) -out:$(OUTDIR)\demo.exe $(OUTDIR)\demo.obj $(OUTDIR)\select.lib $(OUTDIR)\demo.res $(guilibsdll)

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
        $(CLEANUP)
