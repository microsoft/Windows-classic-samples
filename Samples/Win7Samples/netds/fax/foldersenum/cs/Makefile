#----- FoldersEnum-------

#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>


# Nmake macros for building Windows 32-Bit apps
!include <win32.mak>

!if ("$(TARGETOS)" == "WINNT") && ("$(CPU)" == "IA64") 

!message Sorry, this is not supported on the IA64 platforms.

!else

PROJ = FoldersEnumCS

all: $(OUTDIR) $(OUTDIR)\FAXCOMEXLib.dll  $(OUTDIR)\$(PROJ).exe

$(OUTDIR):
     if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

_IMPORTS=$(_IMPORTS) /r:$(OUTDIR)\FAXCOMEXlib.dll


!IFDEF NODEBUG
DEBUG=
!ELSE
DEBUG=/debug+ /debug:full /define:DEBUG;TRACE
!ENDIF


!if ("$(TARGETOS)" == "WINNT") && ("$(CPU)" != "IA64") && ("$(CPU)" != "AMD64")

$(OUTDIR)\FAXCOMEXLib.dll : "$(MSSDK)\lib\faxcomex.tlb"
   set BUILDMSG3=making $(@F)
   tlbimp.exe  "$(MSSDK)\lib\faxcomex.tlb" /primary /keyfile:key.snk /out:$@ 

!endif


!if ("$(TARGETOS)" == "WINNT") && ("$(CPU)" == "AMD64")

$(OUTDIR)\FAXCOMEXLib.dll: "$(MSSDK)\lib\x64\faxcomex.tlb"
   set BUILDMSG3=making $(@F)
   tlbimp.exe   "$(MSSDK)\lib\x64\faxcomex.tlb" /primary /keyfile:key.snk /out:$@ /machine:x64

!endif

$(OUTDIR)\$(PROJ).exe: FoldersEnum.cs
	csc.exe /t:exe /out:$@ $** $(_IMPORTS) $(DEBUG) /keyfile:key.snk

clean:
     $(CLEANUP)

!endif