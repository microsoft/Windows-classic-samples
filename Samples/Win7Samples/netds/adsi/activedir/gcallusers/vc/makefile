#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\gcallusers.exe 

$(OUTDIR)\gcallusers.exe:	 $(OUTDIR) $(OUTDIR)\gcallusers.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\gcallusers.exe $(conlflags) /PDB:$(OUTDIR)\gcallusers.pdb /MACHINE:$(CPU) $(OUTDIR)\gcallusers.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\gcallusers.obj:	gcallusers.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Fp$(OUTDIR)\gcallusers.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" /D"_UNICODE" /D"UNICODE" gcallusers.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
