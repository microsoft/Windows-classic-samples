Proj = WorkerThread

!include <Win32.Mak>

All: WorkerThread.Dll

WorkerThread.Obj: WorkerThread.c
  $(cc) $(cdebug) $(cflags) $(cvarsdll) WorkerThread.c

WorkerThread.Exp: WorkerThread.Def WorkerThread.Obj
  $(implib) -machine:$(CPU) -nologo -Def:WorkerThread.Def -Out:WorkerThread.Lib \
    WorkerThread.Obj

WorkerThread.Dll: $*.Obj $*.Exp
     $(link) $(linkdebug) $(dlllflags) -base:0x1C000000 -out:WorkerThread.dll  \
      WorkerThread.Obj WorkerThread.Exp  $(guilibsdll)

Clean:
    del *.obj

Cleanall:
    del  *.obj
    del *.exp
    del *.obj
    del *.dll
