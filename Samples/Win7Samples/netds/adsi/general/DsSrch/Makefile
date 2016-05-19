!if "$(CPU)" == "IA64"
!message Sorry building this sample for 64bit is not currently supported.
!else
#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\dssrch.exe 

$(OUTDIR)\dssrch.exe:	 $(OUTDIR) $(OUTDIR)\main.obj $(OUTDIR)\util.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\dssrch.exe $(conlflags) /PDB:$(OUTDIR)\dssrch.pdb /MACHINE:$(CPU) $(OUTDIR)\main.obj $(OUTDIR)\util.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\main.obj:	main.cxx
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Fp$(OUTDIR)\dssrch.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" main.cxx

$(OUTDIR)\util.obj:	util.cxx
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Fp$(OUTDIR)\dssrch.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" util.cxx

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
!endif
