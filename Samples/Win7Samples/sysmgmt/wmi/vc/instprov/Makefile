#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

PROJ = instprov
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).dll

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

EXTRA_LIBS = wbemuuid.lib odbccp32.lib odbc32.lib shell32.lib advapi32.lib


LINK32_OBJS = $(OUTDIR)\utils.obj \
              $(OUTDIR)\maindll.obj \
              $(OUTDIR)\instprov.obj \
              $(OUTDIR)\classfac.obj

.cpp{$(OUTDIR)}.obj:
     $(cc) $(cflags) $(cdebug) $(cvarsdll) /DSTRICT /D_DLL /DWIN32ANSI /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(PROJ).dll:   $(LINK32_OBJS)
     $(link) $(ldebug) $(dlllflags) /MACHINE:$(CPU) /PDB:$(OUTDIR)\$(PROJ).pdb -out:$(OUTDIR)\$(PROJ).dll /DEF:instprov.def  $(LINK32_OBJS) $(EXTRA_LIBS) $(baselibs) $(olelibs) 
     mt -manifest $(OUTDIR)\$(PROJ).dll.manifest -outputresource:$(OUTDIR)\$(PROJ).dll;2

clean:
     $(CLEANUP)

