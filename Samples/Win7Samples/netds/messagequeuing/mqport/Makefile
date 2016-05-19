#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

PROJ = mqport
all:    $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

EXTRA_LIBS = odbccp32.lib odbc32.lib shell32.lib advapi32.lib mqrt.lib

LINK32_OBJS = $(OUTDIR)\mqport.obj

.cpp{$(OUTDIR)}.obj:
     $(cc) $(cflags) $(cdebug) $(cvarsdll) /EHsc /D_MBCS /D_CONSOLE /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**

$(OUTDIR)\$(PROJ).exe:   $(LINK32_OBJS)
     $(link) $(ldebug) $(conlflags) /MACHINE:$(CPU) /PDB:$(OUTDIR)\$(PROJ).pdb -out:$(OUTDIR)\$(PROJ).exe $(LINK32_OBJS) $(EXTRA_LIBS) $(baselibs) $(olelibs) 

clean:
     $(CLEANUP)
