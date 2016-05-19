!include <win32.mak>

Proj = SendTo

LinkObjs = $(Proj).obj $(Proj).res

all: $(Proj).exe

$(Proj).obj: $(Proj).c
    $(cc) $(cflags) $(cvars) $(cdebug) $(Proj).c

$(Proj).exe: $(LinkObjs)
    $(link) $(linkdebug) $(guiflags) -out:$@ $(LinkObjs) $(olelibs) shell32.lib shlwapi.lib strsafe.lib

$(Proj).res: $(Proj).rc
    $(rc) $(rcflags) $(rcvars) $(rcdebug) $(Proj).rc

clean:
    -if exist $(Proj).obj del $(Proj).obj
    -if exist $(Proj).res del $(Proj).res
    -if exist $(Proj).exe del $(Proj).exe
    -if exist $(Proj).pdb del $(Proj).pdb
    -if exist vc60.pdb del vc60.pdb
