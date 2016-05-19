#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\Schema.exe 

$(OUTDIR)\Schema.exe:	 $(OUTDIR) $(OUTDIR)\StdAfx.obj $(OUTDIR)\Schema.obj
	$(link) $(ldebug) /OUT:$(OUTDIR)\Schema.exe $(conlflags) /PDB:$(OUTDIR)\Schema.pdb /MACHINE:$(CPU) $(OUTDIR)\StdAfx.obj $(OUTDIR)\Schema.obj shell32.lib $(ole2libs) odbc32.lib odbccp32.lib activeds.lib adsiid.lib 

$(OUTDIR)\StdAfx.obj:	StdAfx.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Yc"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" StdAfx.cpp

$(OUTDIR)\Schema.obj:	Schema.cpp
	$(cc) $(cflags) $(cdebug) $(cvars) /GX /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_CONSOLE" /D"_MBCS" Schema.cpp

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
