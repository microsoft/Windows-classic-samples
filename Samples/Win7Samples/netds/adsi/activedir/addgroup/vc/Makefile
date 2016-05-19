#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\AddGroup.exe 

$(OUTDIR)\AddGroup.exe:	 $(OUTDIR) $(OUTDIR)\main.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\AddGroup.exe $(conlflags) /PDB:$(OUTDIR)\AddGroup.pdb /MACHINE:$(CPU) $(OUTDIR)\main.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.Lib adsiid.lib 

$(OUTDIR)\main.obj:	main.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /FR /Fp$(OUTDIR)\AddGroup.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" main.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
