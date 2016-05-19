!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\ifslsp.dll

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

cflags= $(cflags) -D_PSDK_BLD

DLL_OBJS=  $(OUTDIR)\spi.obj        \
           $(OUTDIR)\extension.obj  \
           $(OUTDIR)\sockinfo.obj   \
           $(OUTDIR)\lspguid.obj    
           
LIBS= ws2_32.lib ole32.lib rpcrt4.lib ..\common\$(OUTDIR)\lspcommon.lib $(baselibs)

.cpp{$(OUTDIR)}.obj:
	$(cc) $(cdebug) $(cflags) -DSTRICT -DFD_SETSIZE=1024 /GS /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\ifslsp.dll: $(DLL_OBJS)
	$(link) $(linkdebug) $(dlllflags) -def:ifslsp.def \
	-out:$*.dll -pdb:$*.pdb $(guilibsdll) $** $(LIBS) 
    if not exist "..\bin\$(OUTDIR)/$(NULL)" mkdir "..\bin\$(OUTDIR)/$(NULL)"
    copy $(OUTDIR)\ifslsp.dll ..\bin\$(OUTDIR)
    if exist $(OUTDIR)\ifslsp.pdb copy $(OUTDIR)\ifslsp.pdb ..\bin\$(OUTDIR)
	Echo COMPILED IFSLSP.DLL

clean:
    rmdir /s /q $(OUTDIR)
