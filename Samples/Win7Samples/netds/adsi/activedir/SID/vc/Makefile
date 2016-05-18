#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\USEROBJECTSID.exe 

$(OUTDIR)\USEROBJECTSID.exe:	 $(OUTDIR) $(OUTDIR)\objectSid.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\USEROBJECTSID.exe $(conlflags) /PDB:$(OUTDIR)\USEROBJECTSID.pdb /MACHINE:$(CPU) $(OUTDIR)\objectSid.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\objectSid.obj:	objectSid.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fp$(OUTDIR)\USEROBJECTSID.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" /D"UNICODE" /D"_UNICODE" objectSid.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
