#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\CreateUser.exe 

$(OUTDIR)\CreateUser.exe:	 $(OUTDIR) $(OUTDIR)\CreateUserHelpers.obj $(OUTDIR)\main.obj $(OUTDIR)\UserProps.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\CreateUser.exe $(conlflags) /PDB:$(OUTDIR)\CreateUser.pdb /MACHINE:$(CPU) $(OUTDIR)\CreateUserHelpers.obj $(OUTDIR)\main.obj $(OUTDIR)\UserProps.obj activeds.lib adsiid.lib shell32.lib $(ole2libs) odbc32.lib odbccp32.lib 

$(OUTDIR)\CreateUserHelpers.obj:	CreateUserHelpers.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /FR /Fp$(OUTDIR)\CreateUser.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" CreateUserHelpers.cpp

$(OUTDIR)\main.obj:	main.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /FR /Fp$(OUTDIR)\CreateUser.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" main.cpp

$(OUTDIR)\UserProps.obj:	UserProps.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /FR /Fp$(OUTDIR)\CreateUser.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" UserProps.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
