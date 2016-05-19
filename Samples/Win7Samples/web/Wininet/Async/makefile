PROJ=async

!include <win32.mak>
all:$(OUTDIR) $(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)\async.obj: async.c
    $(cc) $(cflags) $(cvars) $(cdebug) /EHsc /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" async.c

async.exe: $(OUTDIR)\async.obj
    $(link) $(conlflags)  $(ldebug) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).exe $(OUTDIR)\async.obj wininet.lib  user32.lib kernel32.lib

clean:
    $(CLEANUP)
