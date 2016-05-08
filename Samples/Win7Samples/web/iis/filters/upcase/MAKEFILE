Proj = UpCase

!include <Win32.mak>

all: $(Proj).Dll

$(Proj).Obj: $(Proj).c
  $(cc) $(cdebug) $(cflags) $(cvarsdll) $(Proj).c

$(Proj).Dll: $*.Obj $*.Exp
     $(link) $(linkdebug) $(dlllflags) -base:0x1C000000 -out:$(Proj).dll  \
      $(Proj).Obj $(Proj).Exp  $(guilibsdll)

$(Proj).Exp: $*.Obj
  $(implib) -machine:$(CPU) -nologo -Def:$(Proj).Def -Out:$(Proj).Lib \
    $(Proj).Obj 


Clean:
    del *.obj

Cleanall: clean
    del *.exp
    del *.lib
    del *.dll
