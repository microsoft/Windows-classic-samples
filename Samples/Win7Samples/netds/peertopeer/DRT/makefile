!include <Win32.Mak>

SOURCES= \
    DrtSdkSample.cpp \
    CAPIWrappers.cpp \
    FirewallConfig.cpp \
    CustomBootstrapper.cpp \
    CustomSecurityProvider.cpp \

OBJS= \
    $(OUTDIR)\DrtSdkSample.obj \
    $(OUTDIR)\CAPIWrappers.obj \
    $(OUTDIR)\FirewallConfig.obj \
    $(OUTDIR)\CustomBootstrapper.obj \
    $(OUTDIR)\CustomSecurityProvider.obj \

LIBS= \
    p2p.lib \
    drt.lib \
    drtprov.lib \
    drttransport.lib \
    ole32.lib \
    oleaut32.lib \

ALL: $(OUTDIR) $(OUTDIR)\DrtSdkSample.exe

"$(OUTDIR)":
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) /EHsc /DUNICODE /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\DrtSdkSample.exe: $(OBJS)
    $(link) $(ldebug) $(conflags) /PDB:$(OUTDIR)\DrtSdkSample.pdb -out:$(OUTDIR)\DrtSdkSample.exe $(OBJS) $(conlibs) $(LIBS)

clean:
        $(CLEANUP)
