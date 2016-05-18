#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\ADsPropertyList.exe 

$(OUTDIR)\ADsPropertyList.exe:	 $(OUTDIR) $(OUTDIR)\ADSIhelpers.obj $(OUTDIR)\main.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\ADsPropertyList.exe $(conlflags) /PDB:$(OUTDIR)\ADsPropertyList.pdb /MACHINE:$(CPU) $(OUTDIR)\ADSIhelpers.obj $(OUTDIR)\main.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.Lib adsiid.lib 

$(OUTDIR)\ADSIhelpers.obj:	ADSIhelpers.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /FR /Fp$(OUTDIR)\ADsPropertyList.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" ADSIhelpers.cpp

$(OUTDIR)\main.obj:	main.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /FR /Fp$(OUTDIR)\ADsPropertyList.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" main.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
