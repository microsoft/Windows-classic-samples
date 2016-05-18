Proj = KeepAlive

!include <Win32.Mak>

All: KeepAlive.Dll

KeepAlive.Obj: KeepAlive.c
  $(cc) $(cdebug) $(cflags) $(cvarsdll) KeepAlive.c

KeepAlive.Dll: $*.Obj $*.Exp
     $(link) $(linkdebug) $(dlllflags) -base:0x1C000000 -out:KeepAlive.dll  \
      KeepAlive.Obj KeepAlive.Exp  $(guilibsdll)

KeepAlive.Exp: KeepAlive.Def KeepAlive.Obj
  $(implib) -machine:$(CPU) -nologo -Def:KeepAlive.Def -Out:KeepAlive.Lib \
    KeepAlive.Obj

Clean:
    del *.obj

Cleanall:
    del  *.obj
    del *.exp
    del *.obj
    del *.dll
