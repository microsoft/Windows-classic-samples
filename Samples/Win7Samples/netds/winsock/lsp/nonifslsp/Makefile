!include <win32.mak>

all: $(OUTDIR)  $(OUTDIR)\nonifslsp.dll

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

cflags= $(cflags) -D_PSDK_BLD

DLL_OBJS=  $(OUTDIR)\spi.obj        \
           $(OUTDIR)\extension.obj  \
           $(OUTDIR)\overlap.obj    \
           $(OUTDIR)\sockinfo.obj   \
           $(OUTDIR)\lspguid.obj    \
           $(OUTDIR)\asyncselect.obj

LIBS= ws2_32.lib ole32.lib rpcrt4.lib ..\common\$(OUTDIR)\lspcommon.lib $(baselibs)

.cpp{$(OUTDIR)}.obj:
	$(cc) $(cdebug) $(cflags) -DSTRICT -DFD_SETSIZE=1024 /GS /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\nonifslsp.dll: $(DLL_OBJS)
	$(link) $(linkdebug) $(dlllflags) -def:nonifslsp.def \
	-out:$*.dll -pdb:$*.pdb $(guilibsdll) $** $(LIBS) 
    if not exist "..\bin\$(OUTDIR)/$(NULL)" mkdir "..\bin\$(OUTDIR)/$(NULL)"
    copy $(OUTDIR)\nonifslsp.dll ..\bin\$(OUTDIR)
    if exist $(OUTDIR)\nonifslsp.pdb copy $(OUTDIR)\nonifslsp.pdb ..\bin\$(OUTDIR)
	Echo COMPILED NONIFSLSP.DLL

clean:
    rmdir /s /q $(OUTDIR)
