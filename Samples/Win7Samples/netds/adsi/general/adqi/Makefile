#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>



all:	$(OUTDIR) $(OUTDIR)\ADQI.exe 

$(OUTDIR)\ADQI.exe:	 $(OUTDIR) $(OUTDIR)\StdAfx.obj $(OUTDIR)\ADQI.obj $(OUTDIR)\ADQIDlg.obj $(OUTDIR)\ADs.obj $(OUTDIR)\ADsContainer.obj $(OUTDIR)\ADsLargeInteger.obj $(OUTDIR)\ADsOpenDSObject.obj $(OUTDIR)\ADsPropertyList.obj $(OUTDIR)\DirectoryObject.obj $(OUTDIR)\DirectorySearch.obj $(OUTDIR)\helper.obj $(OUTDIR)\security.obj $(OUTDIR)\ADQI.res 
	$(link) $(ldebug) /OUT:$(OUTDIR)\ADQI.exe $(guilflags) $(guilibs) /PDB:$(OUTDIR)\ADQI.pdb /MACHINE:$(CPU) $(OUTDIR)\StdAfx.obj $(OUTDIR)\ADQI.obj $(OUTDIR)\ADQIDlg.obj $(OUTDIR)\ADs.obj $(OUTDIR)\ADsContainer.obj $(OUTDIR)\ADsLargeInteger.obj $(OUTDIR)\ADsOpenDSObject.obj $(OUTDIR)\ADsPropertyList.obj $(OUTDIR)\DirectoryObject.obj $(OUTDIR)\DirectorySearch.obj $(OUTDIR)\helper.obj $(OUTDIR)\security.obj $(OUTDIR)\ADQI.res activeds.lib adsiid.lib 

$(OUTDIR)\StdAfx.obj:	StdAfx.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yc"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" StdAfx.cpp

$(OUTDIR)\ADQI.obj:	ADQI.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" ADQI.cpp

$(OUTDIR)\ADQIDlg.obj:	ADQIDlg.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" ADQIDlg.cpp

$(OUTDIR)\ADs.obj:	ADs.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" ADs.cpp

$(OUTDIR)\ADsContainer.obj:	ADsContainer.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" ADsContainer.cpp

$(OUTDIR)\ADsLargeInteger.obj:	ADsLargeInteger.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" ADsLargeInteger.cpp

$(OUTDIR)\ADsOpenDSObject.obj:	ADsOpenDSObject.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" ADsOpenDSObject.cpp

$(OUTDIR)\ADsPropertyList.obj:	ADsPropertyList.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" ADsPropertyList.cpp

$(OUTDIR)\DirectoryObject.obj:	DirectoryObject.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" DirectoryObject.cpp

$(OUTDIR)\DirectorySearch.obj:	DirectorySearch.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" DirectorySearch.cpp

$(OUTDIR)\helper.obj:	helper.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" helper.cpp

$(OUTDIR)\security.obj:	security.cpp
	$(cc) $(cflags) $(cdebug) $(cvarsdll) /Yu"stdafx.h" /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D"_AFXDLL" security.cpp

$(OUTDIR)\ADQI.res:	
	$(rc) $(rcvars) $(rcflags) /Fo$(OUTDIR)\ADQI.res ADQI.rc

        
#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
