#Copyright (c) Microsoft Corporation. All rights reserved. 

!IF "$(TARGETOS)" == "WINNT"




#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\WinHttpAsync.exe 

$(OUTDIR)\WinHttpAsync.exe:	 $(OUTDIR) $(OUTDIR)\SimpleBrowser.obj $(OUTDIR)\Browser.obj
	$(link) $(ldebug) /OUT:$(OUTDIR)\WinHttpAsync.exe $(conlflags) /PDB:$(OUTDIR)\WinHttpAsync.pdb /MACHINE:$(CPU) $(OUTDIR)\SimpleBrowser.obj $(OUTDIR)\Browser.obj winhttp.lib $(baselibs)

$(OUTDIR)\SimpleBrowser.obj:	SimpleBrowser.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"UNICODE" /D"_UNICODE" SimpleBrowser.cpp

$(OUTDIR)\Browser.obj:	Browser.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"UNICODE" /D"_UNICODE" Browser.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)


!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF