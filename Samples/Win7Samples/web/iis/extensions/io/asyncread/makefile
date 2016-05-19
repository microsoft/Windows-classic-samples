!include <win32.mak>

LINKER_FLAGS=$(ldebug) $(dllflags)

TARGETS=areadcli.dll 

all: $(TARGETS) Install

clean:
    erase $(TARGETS) *.obj
    for %i in (areadcli) do (erase %i.exp %i.ilk %i.lib %i.pdb)

.c.obj:
    $(cc) $(cflags) $(cvarsdll) $(cdebug) $< 


areadcli.dll: areadcli.obj
    $(link) $(LINKER_FLAGS) $(guilibsdll) -out:areadcli.dll -def:areadcli.def areadcli.obj kernel32.lib 
Install:
!IF "$(WWWSCRIPTS)" != ""
!IF "$(WWWROOT)" != ""
        copy areadcli.dll $(WWWSCRIPTS)\SDK\areadcli.Dll 
        copy ..\ISAPIsmp.htm $(WWWROOT)\ISAPIsmp.Htm
!ENDIF
!ENDIF
