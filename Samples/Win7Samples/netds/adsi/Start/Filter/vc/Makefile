#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\Filter.exe 

$(OUTDIR)\Filter.exe:	 $(OUTDIR) $(OUTDIR)\StdAfx.obj $(OUTDIR)\Filter.obj 
	$(link) $(ldebug) /OUT:$(OUTDIR)\Filter.exe $(conlflags) /PDB:$(OUTDIR)\Filter.pdb /MACHINE:$(CPU) $(OUTDIR)\StdAfx.obj $(OUTDIR)\Filter.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\StdAfx.obj:	StdAfx.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Yc"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" StdAfx.cpp

$(OUTDIR)\Filter.obj:	Filter.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" Filter.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
