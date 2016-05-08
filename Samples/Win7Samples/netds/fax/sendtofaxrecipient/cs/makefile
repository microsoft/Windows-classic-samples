
#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>


# Nmake macros for building Windows 32-Bit apps
!include <win32.mak>

!if ("$(TARGETOS)" == "WINNT") && ("$(CPU)" == "IA64") 

!message Sorry, this is not supported on the IA64 platforms.

!else

PROJ = SendToFaxRecipientCS

all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

_IMPORTS=$(_IMPORTS) 


!IFDEF NODEBUG
DEBUG=
!ELSE
DEBUG=/debug+ /debug:full /define:DEBUG;TRACE
!ENDIF


$(OUTDIR)\$(PROJ).exe: SendToFaxRecipient.cs
	csc.exe /t:exe /out:$@ $** $(_IMPORTS) $(DEBUG) /keyfile:key.snk

clean:
     $(CLEANUP)

!endif
