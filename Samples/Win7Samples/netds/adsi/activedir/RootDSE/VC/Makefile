#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\RootDSE.exe 

$(OUTDIR)\RootDSE.exe:	 $(OUTDIR) $(OUTDIR)\RootDSE.obj $(OUTDIR)\StdAfx.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\RootDSE.exe $(conlflags) /PDB:$(OUTDIR)\RootDSE.pdb /MACHINE:$(CPU) $(OUTDIR)\RootDSE.obj $(OUTDIR)\StdAfx.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib adsiid.lib activeds.lib 

$(OUTDIR)\StdAfx.obj:	StdAfx.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" StdAfx.cpp

$(OUTDIR)\RootDSE.obj:	RootDSE.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" RootDSE.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
