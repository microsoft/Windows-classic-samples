!include <win32.mak>

Proj = mqf_draw

ObjList = $(OUTDIR)\mqfdraw.obj   \
          $(OUTDIR)\drawarea.obj  \
          $(OUTDIR)\drawdlg.obj   \
          $(OUTDIR)\logindlg.obj  \
          $(OUTDIR)\connectdlg.obj  \
          $(OUTDIR)\StdAfx.obj    \
          $(OUTDIR)\mqfdraw.res


ALL : $(OUTDIR) $(OUTDIR)\$(Proj).exe

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cflags) $(cdebug) $(cvarsdll) /D_AFXDLL /D_MBCS /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\mqfdraw.res: $(SOURCE) $(OUTDIR)
    $(rc) $(rcflags) $(rcvars) /d_AFXDLL /fo $(OUTDIR)\mqfdraw.res mqfdraw.rc

$(OUTDIR)\$(Proj).exe: $(ObjList)
    $(link) $(ldebug) $(guilflags) $(ObjList) /PDB:$(OUTDIR)\$(Proj).pdb -out:$(OUTDIR)\$(Proj).exe $(LINK32_OBJS) mqrt.lib $(guilibsdll)


