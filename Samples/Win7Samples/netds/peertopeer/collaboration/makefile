!include <Win32.Mak>

SOURCES=        \
    Contacts.c     \
    EndpointInfo.c \
    Events.c       \
    Invite.c       \
    Main.c         \
    PNM.c          \
    Presence.c     \
    Publication.c  \
    Shared.c       \
    SignIn.c

OBJS=                          \
    $(OUTDIR)\Contacts.obj     \
    $(OUTDIR)\EndpointInfo.obj \
    $(OUTDIR)\Events.obj       \
    $(OUTDIR)\Invite.obj       \
    $(OUTDIR)\Main.obj         \
    $(OUTDIR)\PNM.obj          \
    $(OUTDIR)\Presence.obj     \
    $(OUTDIR)\Publication.obj  \
    $(OUTDIR)\Shared.obj       \
    $(OUTDIR)\SignIn.obj


LIBS=               \
    p2p.lib         \
    ws2_32.lib      \
    Rpcrt4.lib

ALL: $(OUTDIR) $(OUTDIR)\Collaboration.exe

"$(OUTDIR)":
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /DUNICODE /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\Collaboration.exe: $(OBJS)
    $(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\Collaboration.pdb -out:$(OUTDIR)\Collaboration.exe $(OBJS) $(conlibs) $(LIBS)

clean:
        $(CLEANUP)
