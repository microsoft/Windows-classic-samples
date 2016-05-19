Proj = Redirect

!include <Win32.Mak>

All: Redirect.Dll

Redirect.Obj: Redirect.cpp
  $(cc) $(cdebug) $(cflags) $(cvarsdll) Redirect.cpp

Redirect.Exp: Redirect.Def Redirect.Obj
  $(implib) -machine:$(CPU) -nologo -Def:Redirect.Def -Out:Redirect.Lib \
    Redirect.Obj

Redirect.Dll: $*.Obj $*.Exp
     $(link) $(linkdebug) $(dlllflags) -base:0x1C000000 -out:Redirect.dll  \
      Redirect.Obj Redirect.Exp  $(guilibsdll)

Clean:
    del *.obj

Cleanall:
    del  *.obj
    del *.exp
    del *.obj
    del *.dll
