#Copyright (c) Microsoft Corporation. All rights reserved. 

!IF "$(TARGETOS)" == "WINNT"




#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\FormSubmit.exe 

$(OUTDIR)\FormSubmit.exe:	 $(OUTDIR) $(OUTDIR)\WinHttpFormSubmitSample.obj
	$(link) $(ldebug) /OUT:$(OUTDIR)\FormSubmit.exe $(conlflags) /PDB:$(OUTDIR)\FormSubmit.pdb /MACHINE:$(CPU) $(OUTDIR)\WinHttpFormSubmitSample.obj winhttp.lib $(baselibs)

$(OUTDIR)\WinHttpFormSubmitSample.obj:	WinHttpFormSubmitSample.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"UNICODE" /D"_UNICODE" WinhttpFormSubmitSample.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)


!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF