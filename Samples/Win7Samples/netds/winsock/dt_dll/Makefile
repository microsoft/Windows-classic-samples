!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\dt_dll.dll

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the resource if necessary

OBJS = $(OUTDIR)\dt_dll.obj $(OUTDIR)\handlers.obj $(OUTDIR)\huerror.obj

LIBS = ws2_32.lib advapi32.lib

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) -DSTRICT $(cvarsdll) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\dt_dll.dll: $(OBJS)
    $(link) $(linkdebug) $(dllflags) $(guilibs) -out:$(OUTDIR)\dt_dll.dll -def:dt_dll.def $** $(LIBS)

clean:
	$(CLEANUP)
