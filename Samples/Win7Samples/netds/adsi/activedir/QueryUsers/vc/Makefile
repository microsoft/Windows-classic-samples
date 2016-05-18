#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\queryusers.exe 

$(OUTDIR)\queryusers.exe:	 $(OUTDIR) $(OUTDIR)\queryusers.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\queryusers.exe $(conlflags) /PDB:$(OUTDIR)\queryusers.pdb /MACHINE:$(CPU) $(OUTDIR)\queryusers.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\queryusers.obj:	queryusers.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Fp$(OUTDIR)\queryusers.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" /D"UNICODE" /D"_UNICODE" queryusers.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
