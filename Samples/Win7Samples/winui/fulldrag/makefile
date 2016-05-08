!include <win32.mak>

Proj = FullDrag

LinkObjs = $(Proj).obj

all: $(Proj).exe

$(Proj).obj: $(Proj).c
    $(cc) $(cflags) $(cvars) $(cdebug) $(Proj).c

$(Proj).exe: $(LinkObjs)
    $(link) $(linkdebug) $(guiflags) -out:$@ $(LinkObjs) $(guilibs)

clean:
    -if exist $(Proj).obj del $(Proj).obj
    -if exist $(Proj).exe del $(Proj).exe
    -if exist $(Proj).pdb del $(Proj).pdb
    -if exist vc60.pdb del vc60.pdb
