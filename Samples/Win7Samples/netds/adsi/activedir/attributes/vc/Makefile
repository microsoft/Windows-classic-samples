#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\attributes.exe 

$(OUTDIR)\attributes.exe:	 $(OUTDIR) $(OUTDIR)\attributes.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\attributes.exe $(conlflags) /PDB:$(OUTDIR)\attributes.pdb /MACHINE:$(CPU) $(OUTDIR)\attributes.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\attributes.obj:	attributes.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /Fp$(OUTDIR)\attributes.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" /D"_UNICODE" /D"UNICODE" attributes.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
