!include <Win32.Mak>

SOURCES=                \
    SampleProvider.c    \
    Install.c           \
    Utils.c           \
    Reg.c               \

OBJS=                             \
    $(OUTDIR)\SampleProvider.obj  \
    $(OUTDIR)\Install.obj         \
    $(OUTDIR)\Utils.obj         \
    $(OUTDIR)\Reg.obj             \

LIBS=               \
    rpcrt4.lib      \

cvars = $(cvars) -DWIN32_LEAN_AND_MEAN -DUNICODE -I$(OUTDIR)

ALL: $(OUTDIR) $(OUTDIR)\SampleProvider.exe

"$(OUTDIR)":
    if not exist "$(OUTDIR)" mkdir "$(OUTDIR)"

.c{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /DUNICODE /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\SampleProvider.exe: $(OBJS)
    $(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\SampleProvider.pdb -out:$(OUTDIR)\SampleProvider.exe $(OBJS) $(conlibs) $(LIBS)

clean:
        $(CLEANUP)
