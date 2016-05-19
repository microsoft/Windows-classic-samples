!include <Win32.Mak>

SOURCES=     \
    pnrp.c    \

OBJS=                         \
    $(OUTDIR)\pnrp.obj   \

LIBS=               \
    p2p.lib         \

ALL: $(OUTDIR) $(OUTDIR)\simpleapi.exe

"$(OUTDIR)":
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /DUNICODE /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\simpleapi.exe: $(OBJS)
    $(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\pnrp.pdb -out:$(OUTDIR)\pnrp.exe $(OBJS) $(conlibs) $(LIBS)

clean:
        $(CLEANUP)
