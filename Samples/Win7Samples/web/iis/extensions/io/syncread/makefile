!include <win32.mak>

LINKER_FLAGS=$(ldebug) $(dllflags)

TARGETS=readcli.dll 

all: $(TARGETS) Install

clean:
    erase $(TARGETS) *.obj
    for %i in (readcli) do (erase %i.exp %i.ilk %i.lib %i.pdb)

.c.obj:
    $(cc) $(cflags) $(cvarsdll) $(cdebug) $< 


readcli.dll: readcli.obj
    $(link) $(LINKER_FLAGS) -out:readcli.dll -def:readcli.def readcli.obj $(baselibs)

Install:
!IF "$(WWWSCRIPTS)" != ""
!IF "$(WWWROOT)" != ""
        copy readcli.dll $(WWWSCRIPTS)\SDK\readcli.Dll 
        copy ..\ISAPIsmp.htm $(WWWROOT)\ISAPIsmp.Htm
!ENDIF
!ENDIF
