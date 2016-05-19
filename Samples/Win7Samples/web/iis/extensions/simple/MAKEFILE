Proj = Simple

!include <Win32.Mak>

All: Simple.Dll

Simple.Obj: Simple.cpp
  $(cc) $(cdebug) $(cflags) $(cvarsdll) Simple.cpp

Simple.Exp: Simple.Def Simple.Obj
  $(implib) -machine:$(CPU) -nologo -Def:Simple.Def -Out:Simple.Lib \
    Simple.Obj

Simple.Dll: $*.Obj $*.Exp
     $(link) $(linkdebug) $(dlllflags) -base:0x1C000000 -out:Simple.dll  \
      Simple.Obj Simple.Exp  $(guilibsdll)

Clean:
    del *.obj

Cleanall:
    del  *.obj
    del *.exp
    del *.obj
    del *.dll
