!include <Win32.Mak>

SOURCES=         \
    GraphChat.c  \
    Pnrp.c       \
	InvitationHelpers.c   \
	PeoplePickerDialog.c  \
	PeoplePickerModel.c   \

OBJS=                         \
    $(OUTDIR)\GraphChat.obj   \
    $(OUTDIR)\Pnrp.obj        \
	$(OUTDIR)\InvitationHelpers.obj   \
	$(OUTDIR)\PeoplePickerDialog.obj  \
	$(OUTDIR)\PeoplePickerModel.obj   \
    $(OUTDIR)\GraphChat.res   	      \
	$(OUTDIR)\PeoplePickerDialog.res  \

LIBS=               \
    comctl32.lib    \
    p2p.lib         \
    p2pgraph.lib    \

ALL: $(OUTDIR) $(OUTDIR)\GraphChat.exe

"$(OUTDIR)":
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

.rc{$(OUTDIR)}.res:
    $(rc) $(rcflags) $(rcvars) /Fo"$@" $**

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /DUNICODE /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\GraphChat.exe: $(OBJS)
    $(link) $(ldebug) $(guilflags) /PDB:$(OUTDIR)\GraphChat.pdb -out:$(OUTDIR)\GraphChat.exe $(OBJS) $(guilibs) $(LIBS)

clean:
        $(CLEANUP)
