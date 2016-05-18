!include <win32.mak>

proj=mapname

objlist = \
   $(OUTDIR)\dllmain.obj \
   $(OUTDIR)\mapname.obj \
   $(OUTDIR)\multisz.obj \
   $(OUTDIR)\radutil.obj

all: $(OUTDIR) $(OUTDIR)\$(proj).dll

$(OUTDIR):
   if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

.c{$(OUTDIR)}.obj:
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /Fo"$@" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(proj).dll: $(objlist)
   $(link) $(ldebug) $(dlllflags) $(conlibsdll) -def:$(proj).def -out:$@ $**
