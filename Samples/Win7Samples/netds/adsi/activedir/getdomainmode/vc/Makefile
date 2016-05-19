#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\getdomainmode.exe 

$(OUTDIR)\getdomainmode.exe:	 $(OUTDIR) $(OUTDIR)\getdomainmode.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\getdomainmode.exe $(conlflags) /PDB:$(OUTDIR)\getdomainmode.pdb /MACHINE:$(CPU) $(OUTDIR)\getdomainmode.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\getdomainmode.obj:	getdomainmode.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Fp$(OUTDIR)\getdomainmode.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" /D"UNICODE" /D"_UNICODE" getdomainmode.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
