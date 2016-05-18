
!include <win32.mak>
#!if "$(CPU)" != "i386"
#!message Sorry building this sample for 64bit is not currently supported
#!else
# Nmake macros for building Windows 32-Bit apps

Proj=sa

all:$(OUTDIR) $(OUTDIR)\$(Proj).exe

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"
    

$(OUTDIR)\$(Proj).obj: sa.cpp
    $(cc) $(cflags) $(cvarsmt) $(cdebug) /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(Proj).cpp

$(OUTDIR)\$(Proj).exe: $(OUTDIR)\$(Proj).obj
    $(link) $(ldebug) $(conflags) /MACHINE:$(CPU) -out:$(OUTDIR)\$(Proj).exe $(OUTDIR)\$(Proj).obj /PDB:$(OUTDIR)\$(Proj).PDB $(olelibs)

clean:
    $(CLEANUP)


#!endif
