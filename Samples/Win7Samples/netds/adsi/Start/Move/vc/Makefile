#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\Move.exe 

$(OUTDIR)\Move.exe:	 $(OUTDIR) $(OUTDIR)\StdAfx.obj $(OUTDIR)\Move.obj
	$(link) $(ldebug) /OUT:$(OUTDIR)\Move.exe $(conlflags) /PDB:$(OUTDIR)\Move.pdb /MACHINE:$(CPU) $(OUTDIR)\StdAfx.obj $(OUTDIR)\Move.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\StdAfx.obj:	StdAfx.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Yc"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" StdAfx.cpp

$(OUTDIR)\Move.obj:	Move.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" Move.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
