!include <Win32.Mak>

OBJS=                         \
    $(OUTDIR)\GroupChat.obj   \
    $(OUTDIR)\GroupChat.res   \

LIBS=               \
    comctl32.lib    \
    p2p.lib         \

ALL: $(OUTDIR) $(OUTDIR)\GroupChat.exe

"$(OUTDIR)":
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\GroupChat.res: GroupChat.rc
    $(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\GroupChat.res GroupChat.rc

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /DUNICODE /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\GroupChat.exe: $(OBJS)
    $(link) $(ldebug) $(guilflags) /PDB:$(OUTDIR)\GroupChat.pdb -out:$(OUTDIR)\GroupChat.exe $(OBJS) $(guilibs) $(LIBS)

clean:
        $(CLEANUP)
