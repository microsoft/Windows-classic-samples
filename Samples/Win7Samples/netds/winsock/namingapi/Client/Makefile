!include <Win32.Mak>

SOURCES=                \
    SampleClient.c    \

OBJS=                             \
    $(OUTDIR)\SampleClient.obj  \

LIBS=               \
    rpcrt4.lib      \

cvars = $(cvars) -DWIN32_LEAN_AND_MEAN -DUNICODE -I$(OUTDIR)

ALL: $(OUTDIR) $(OUTDIR)\SampleClient.exe

"$(OUTDIR)":
    if not exist "$(OUTDIR)" mkdir "$(OUTDIR)"

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /DUNICODE /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\SampleClient.exe: $(OBJS)
    $(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\SampleClient.pdb -out:$(OUTDIR)\SampleClient.exe $(OBJS) $(conlibs) $(LIBS)

clean:
        $(CLEANUP)
