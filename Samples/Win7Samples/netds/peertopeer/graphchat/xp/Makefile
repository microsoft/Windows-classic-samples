!include <Win32.Mak>

SOURCES=         \
    GraphChat.c  \
    Pnrp.c       \

OBJS=                         \
    $(OUTDIR)\GraphChat.obj   \
    $(OUTDIR)\Pnrp.obj        \
    $(OUTDIR)\GraphChat.res   \

LIBS=               \
    comctl32.lib    \
    p2p.lib         \
    p2pgraph.lib    \

ALL: $(OUTDIR) $(OUTDIR)\GraphChat.exe

"$(OUTDIR)":
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\GraphChat.res: GraphChat.rc
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\GraphChat.res GraphChat.rc

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /DUNICODE /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\GraphChat.exe: $(OBJS)
    $(link) $(ldebug) $(guilflags) /PDB:$(OUTDIR)\GraphChat.pdb -out:$(OUTDIR)\GraphChat.exe $(OBJS) $(guilibs) $(LIBS)

clean:
        $(CLEANUP)
