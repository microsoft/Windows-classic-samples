!include <win32.mak>

all: $(OUTDIR) $(OUTDIR)\lspcommon.lib

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

cflags= $(cflags) -D_PSDK_BLD

LIB_OBJS=  $(OUTDIR)\provider.obj

LIBS= ws2_32.lib ole32.lib rpcrt4.lib $(baselibs)

$(OUTDIR)\provider.obj:
	$(cc) provider.cpp $(cdebug) $(cflags) -DSTRICT -DFD_SETSIZE=1024 /GS /Fo$(OUTDIR)\provider.obj

.cpp{$(OUTDIR)}.obj:
	$(cc) $(cdebug) $(cflags) -DSTRICT -DFD_SETSIZE=1024 /GS /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\lspcommon.lib: $(LIB_OBJS)
	$(link) /lib $(libflags) -out:$*.lib  $**
	Echo COMPILED LSPCOMMON.LIB

clean:
    rmdir /s /q $(OUTDIR)
