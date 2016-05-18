#----- Nmake macros for building Windows 32-Bit apps

!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\testdll.dll

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)


#----- Update the object files if necessary

$(OUTDIR)\testdll.obj: testdll.c
	 $(cc) $(cflags) $(cvarsdll) $(cdebug) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" testdll.c

$(OUTDIR)\testmib.obj: testmib.c
	 $(cc) $(cflags) $(cvarsdll) $(cdebug) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" testmib.c


#----- Update the import library

$(OUTDIR)\testdll.lib: $(OUTDIR)\testdll.obj $(OUTDIR)\testmib.obj testdll.def
	 $(implib) -machine:$(CPU)     \
	 -def:testdll.def     \
	 $(OUTDIR)\testdll.obj          \
	 $(OUTDIR)\testmib.obj          \
	 -out:$(OUTDIR)\testdll.lib

#----- Update the dynamic link library

$(OUTDIR)\testdll.dll: $(OUTDIR)\testdll.lib $(OUTDIR)\testdll.obj $(OUTDIR)\testmib.obj testdll.def
	 
	 $(link) $(lflags) $(dlllflags) \
	 -base:0x1C000000  \
	 -out:$(OUTDIR)\testdll.dll   \
	 $(OUTDIR)\testdll.exp $(OUTDIR)\testdll.obj $(OUTDIR)\testmib.obj \
         $(guilibsdll) snmpapi.lib
