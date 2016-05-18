#Copyright (c) Microsoft Corporation. All rights reserved. 

!IF "$(TARGETOS)" == "WINNT"




#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\PostSample.exe 

$(OUTDIR)\PostSample.exe:	 $(OUTDIR) $(OUTDIR)\WinHttpPostSample.obj
	$(link) $(ldebug) /OUT:$(OUTDIR)\PostSample.exe $(conlflags) /PDB:$(OUTDIR)\PostSample.pdb /MACHINE:$(CPU) $(OUTDIR)\WinHttpPostSample.obj winhttp.lib $(baselibs)

$(OUTDIR)\WinHttpPostSample.obj:	WinHttpPostSample.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /EHsc /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"UNICODE" /D"_UNICODE" WinhttpPostSample.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)


!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF