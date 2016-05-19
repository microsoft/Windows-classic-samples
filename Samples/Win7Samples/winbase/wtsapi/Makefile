!include <win32.mak>

proj = api
all:    $(OUTDIR) $(OUTDIR)\$(proj).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ELIBS=wtsapi32.lib

PCH=$(OUTDIR)\StdAfx.obj

LINK32_OBJS=$(OUTDIR)\API.obj \
     $(OUTDIR)\APIDoc.obj \
     $(OUTDIR)\APIView.obj \
     $(OUTDIR)\MainFrm.obj \
     $(OUTDIR)\MessageDlg.obj \
     $(OUTDIR)\ProcessDlg.obj \
     $(OUTDIR)\QueryDlg.obj \
     $(OUTDIR)\ServerDlg.obj \
     $(OUTDIR)\SessionDlg.obj \
     $(OUTDIR)\ShutdownDlg.obj \
     $(OUTDIR)\WaitSystemDlg.obj 

$(PCH):    
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /EHsc /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /Yc"stdafx.h" stdafx.cpp

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /EHsc /Fp"$(OUTDIR)\\" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /Yu"stdafx.h" $**

$(OUTDIR)\$(proj).res: $(proj).rc 
    $(rc) $(rcflags) $(rcvars) /fo$(OUTDIR)\$(proj).res $(proj).rc

$(OUTDIR)\$(proj).exe : $(OUTDIR)\$(proj).res $(PCH) $(LINK32_OBJS)
	$(link) $(linkdebug) $(guiflags) $(guilibs) /MACHINE:$(CPU) -out:$(OUTDIR)\$(proj).exe $(LINK32_OBJS) $(PCH) $(ELIBS) /PDB:$(OUTDIR)\$(proj).PDB $(OUTDIR)\$(proj).res 


clean:
     $(CLEANUP)
