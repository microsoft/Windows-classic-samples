Proj = DumpVars

!include <Win32.Mak>

All: DumpVars.Dll

DumpVars.Obj: DumpVars.cpp
  $(cc) $(cdebug) $(cflags) $(cvarsdll) DumpVars.cpp

DumpVars.Exp: DumpVars.Def DumpVars.Obj
  $(implib) -machine:$(CPU) -nologo -Def:DumpVars.Def -Out:DumpVars.Lib \
    DumpVars.Obj

DumpVars.Dll: $*.Obj $*.Exp
     $(link) $(linkdebug) $(dlllflags) -base:0x1C000000 -out:DumpVars.dll  \
      DumpVars.Obj DumpVars.Exp  $(guilibsdll)

Clean:
    del *.obj

Cleanall:
    del  *.obj
    del *.exp
    del *.obj
    del *.dll
