!include <disable.mak>

#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

all:	$(OUTDIR) $(OUTDIR)\multichan.exe 

$(OUTDIR)\multichan.exe: $(OUTDIR) $(OUTDIR)\childview.obj $(OUTDIR)\dlgdest.obj $(OUTDIR)\dlgsrc.obj $(OUTDIR)\mainfrm.obj $(OUTDIR)\multichan.obj $(OUTDIR)\stdafx.obj $(OUTDIR)\Wave.obj $(OUTDIR)\multichan.res 
	$(link) $(ldebug) /OUT:$(OUTDIR)\multichan.exe $(guilflags) /PDB:$(OUTDIR)\multichan.pdb /MACHINE:$(CPU) $(OUTDIR)\childview.obj $(OUTDIR)\dlgdest.obj $(OUTDIR)\dlgsrc.obj $(OUTDIR)\mainfrm.obj $(OUTDIR)\multichan.obj $(OUTDIR)\stdafx.obj $(OUTDIR)\wave.obj $(OUTDIR)\multichan.res winmm.lib msacm32.lib

$(OUTDIR)\childview.obj: Childview.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Gm /EHsc /Fp$(OUTDIR)\multichan.pch /Yc"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_MBCS /D_AFXDLL childview.cpp

$(OUTDIR)\dlgdest.obj:	dlgdest.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Gm /EHsc /Fp$(OUTDIR)\multichan.pch /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_MBCS /D_AFXDLL dlgdest.cpp

$(OUTDIR)\dlgsrc.obj:	dlgsrc.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Gm /EHsc /Fp$(OUTDIR)\multichan.pch /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_MBCS /D_AFXDLL dlgsrc.cpp

$(OUTDIR)\Mainfrm.obj:	Mainfrm.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Gm /EHsc /Fp$(OUTDIR)\multichan.pch /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_MBCS /D_AFXDLL mainfrm.cpp

$(OUTDIR)\Multichan.obj: Multichan.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Gm /EHsc /Fp$(OUTDIR)\multichan.pch /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_MBCS /D_AFXDLL multichan.cpp

$(OUTDIR)\stdafx.obj:	stdafx.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Gm /EHsc /Fp$(OUTDIR)\multichan.pch /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_MBCS /D_AFXDLL stdafx.cpp

$(OUTDIR)\wave.obj:	wave.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Gm /EHsc /Fp$(OUTDIR)\multichan.pch /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_MBCS /D_AFXDLL wave.cpp

$(OUTDIR)\multichan.res:	
	 $(rc) $(rcflags) /Fo$(OUTDIR)\multichan.res multichan.rc

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)


