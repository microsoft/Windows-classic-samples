
#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>


# Nmake macros for building Windows 32-Bit apps
!include <win32.mak>

!if ("$(TARGETOS)" == "WINNT") && ("$(CPU)" == "IA64") 

!message Sorry, this is not supported on the IA64 platforms.

!else

PROJ = SendToFaxRecipientVB

all: $(OUTDIR)  $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"


IMPORTS = /imports:Microsoft.VisualBasic,System,System.Collections,System.Collections.Generic,System.Diagnostics  

!IFDEF NODEBUG
DEBUG=
!ELSE
DEBUG=/define:"CONFIG=\"Debug\",DEBUG=-1,TRACE=-1" /debug+ /debug:full
!ENDIF

REFERENCES =/reference:System.dll

VBFILES =SendToFaxRecipient.vb  "My Project\AssemblyInfo.vb" "My Project\Application.Designer.vb" "My Project\Resources.Designer.vb" "My Project\Settings.Designer.vb"



$(OUTDIR)\$(PROJ).exe: $(VBFILES)
	Vbc.exe /noconfig /rootnamespace:SendToFaxRecipient $(DEBUG) /define:"_MyType=\"Console\"" $(REFERENCES) /keyfile:key.snk  /out:$(OUTDIR)\$(PROJ).exe  /target:exe $(VBFILES)

clean:
     $(CLEANUP)

!endif
