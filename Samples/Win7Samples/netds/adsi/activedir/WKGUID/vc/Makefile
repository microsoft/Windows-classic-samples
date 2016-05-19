#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\userscontainer.exe 

$(OUTDIR)\userscontainer.exe:	 $(OUTDIR) $(OUTDIR)\wkguid.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\userscontainer.exe $(conlflags) /PDB:$(OUTDIR)\userscontainer.pdb /MACHINE:$(CPU) $(OUTDIR)\wkguid.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\wkguid.obj:	wkguid.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Fp$(OUTDIR)\userscontainer.pch /YX /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" wkguid.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
