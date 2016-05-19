!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\instlsp.exe

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

cflags= $(cflags) -D_PSDK_BLD

INSTALL_OBJS= $(OUTDIR)\lspadd.obj     \
           $(OUTDIR)\lspdel.obj     \
           $(OUTDIR)\lspmap.obj     \
           $(OUTDIR)\lsputil.obj    \
           $(OUTDIR)\instlsp.obj    \
           $(OUTDIR)\prnpinfo.obj
           
LIBS= ws2_32.lib ole32.lib rpcrt4.lib ..\common\$(OUTDIR)\lspcommon.lib $(baselibs)

.cpp{$(OUTDIR)}.obj:
	$(cc) $(cdebug) $(cflags) -DSTRICT -DFD_SETSIZE=1024 /GS /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\instlsp.res:  instlsp.rc
    rc -r $(rcvars) /fo $(OUTDIR)\instlsp.res instlsp.rc

$(OUTDIR)\instlsp.exe: $(INSTALL_OBJS) $(OUTDIR)\instlsp.res
	$(link) $(linkdebug) $(conlflags) -out:$*.exe -pdb:$*.pdb $** $(LIBS) 
    if not exist "..\bin\$(OUTDIR)/$(NULL)" mkdir "..\bin\$(OUTDIR)/$(NULL)"
    copy $(OUTDIR)\instlsp.exe ..\bin\$(OUTDIR)
    if exist $(OUTDIR)\instlsp.pdb copy $(OUTDIR)\instlsp.pdb ..\bin\$(OUTDIR)
	Echo COMPILED INSTLSP.EXE

clean:
    rmdir /s /q $(OUTDIR)
