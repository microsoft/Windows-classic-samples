#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\Credentials.exe 

$(OUTDIR)\Credentials.exe:	 $(OUTDIR) $(OUTDIR)\Credentials.obj $(OUTDIR)\StdAfx.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\Credentials.exe $(conlflags) /PDB:$(OUTDIR)\Credentials.pdb /MACHINE:$(CPU) $(OUTDIR)\Credentials.obj $(OUTDIR)\StdAfx.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\StdAfx.obj:	StdAfx.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" StdAfx.cpp

$(OUTDIR)\Credentials.obj:	Credentials.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" Credentials.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
