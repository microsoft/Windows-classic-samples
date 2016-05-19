#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\bindtoparent.exe 

$(OUTDIR)\bindtoparent.exe:	 $(OUTDIR) $(OUTDIR)\bindtoparent.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\bindtoparent.exe $(conlflags) /PDB:$(OUTDIR)\bindtoparent.pdb /MACHINE:$(CPU) $(OUTDIR)\bindtoparent.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\bindtoparent.obj:	bindtoparent.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fp$(OUTDIR)\bindtoparent.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" /D"UNICODE" /D"_UNICODE" bindtoparent.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
