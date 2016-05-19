!include <win32.mak>

# define these if you want a debug version
#PDB = IpRoute.pdb
#CDEBUG= -Zi -Fd"$(PDB)"
#LDEBUG = -debug:full

PROJ = IpRoute
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

LIBS = wsock32.lib iphlpapi.lib

.cpp{$(OUTDIR)}.obj:
     $(cc) $(cflags) $(cdebug) $(cvarsmt) /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(PROJ).exe: IpRoute.obj
     $(link) $(ldebug) $(conlflags) $(conlibs) /MACHINE:$(CPU) /PDB:$(OUTDIR)\$(PROJ).pdb -out:$(OUTDIR)\$(PROJ).exe  IpRoute.obj $(LIBS)

clean:
	$(CLEANUP)
