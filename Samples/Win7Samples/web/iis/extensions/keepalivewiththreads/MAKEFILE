Proj = KeepAliveP

!include <Win32.Mak>

All: KeepAliveP.Dll

KeepAliveP.Obj: KeepAliveP.c ThreadPool.c
  $(cc) $(cdebug) $(cflags) $(cvarsdll) KeepAliveP.c

ThreadPool.Obj: ThreadPool.c KeepAliveP.c
  $(cc) $(cdebug) $(cflags) $(cvarsdll) ThreadPool.c

KeepAliveP.Exp: KeepAliveP.Def KeepAliveP.Obj ThreadPool.Obj
  $(implib) -machine:$(CPU) -nologo -Def:KeepAliveP.Def -Out:KeepAliveP.Lib \
    KeepAliveP.Obj

KeepAliveP.Dll: $*.Obj $*.Exp ThreadPool.Obj
     $(link) $(linkdebug) $(dlllflags) -base:0x1C000000 -out:KeepAliveP.dll  \
      KeepAliveP.Obj ThreadPool.Obj KeepAliveP.Exp  $(guilibsdll)

Clean:
    del *.obj

Cleanall:
    del  *.obj
    del *.exp
    del *.obj
    del *.dll
