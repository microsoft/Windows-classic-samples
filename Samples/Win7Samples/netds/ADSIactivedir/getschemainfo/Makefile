#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\getschemainfo.exe 

$(OUTDIR)\getschemainfo.exe:	 $(OUTDIR) $(OUTDIR)\getschemainfo.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\getschemainfo.exe $(conlflags) /PDB:$(OUTDIR)\getschemainfo.pdb /MACHINE:$(CPU) $(OUTDIR)\getschemainfo.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\getschemainfo.obj:	getschemainfo.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /FR /Fp$(OUTDIR)\getschemainfo.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" /D"_UNICODE" /D"UNICODE" getschemainfo.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
