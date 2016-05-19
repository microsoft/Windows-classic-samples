#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

PROJ = dcomperm
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

EXTRA_LIBS = odbccp32.lib odbc32.lib shell32.lib advapi32.lib

LINK32_OBJS = $(OUTDIR)\wrappers.obj \
              $(OUTDIR)\utils.obj \
              $(OUTDIR)\srvcmgmt.obj \
              $(OUTDIR)\sdmgmt.obj \
              $(OUTDIR)\listacl.obj \
              $(OUTDIR)\dcomperm.obj \
              $(OUTDIR)\aclmgmt.obj

.cpp{$(OUTDIR)}.obj:
     $(cc) $(cflags) $(cdebug) $(cvarsdll) /D_WIN32_DCOM /D_MBCS /D_CONSOLE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(PROJ).exe:   $(LINK32_OBJS)
     $(link) $(ldebug) $(conlflags) /MACHINE:$(CPU) /PDB:$(OUTDIR)\$(PROJ).pdb -out:$(OUTDIR)\$(PROJ).exe $(LINK32_OBJS) $(EXTRA_LIBS) $(baselibs) $(olelibs) 

clean:
     $(CLEANUP)
