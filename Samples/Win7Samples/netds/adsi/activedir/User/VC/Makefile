#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\IADSUSER.exe 

$(OUTDIR)\IADSUSER.exe:	 $(OUTDIR) $(OUTDIR)\iasuser.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\IADSUSER.exe $(conlflags) /PDB:$(OUTDIR)\IADSUSER.pdb /MACHINE:$(CPU) $(OUTDIR)\iasuser.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\iasuser.obj:	iasuser.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fp$(OUTDIR)\IADSUSER.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" /D"UNICODE" /D"_UNICODE" iasuser.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
