!include <Win32.Mak>

OBJS=printer.obj paint.obj getcaps.obj enumprt.obj getpdriv.obj

all: printer.exe

printer.res: printer.rc printer.h
    rc $(rcvars) $(rcflags) -r -fo printer.res printer.rc

.c.obj:
    $(cc) $(cdebug) $(cflags) $(cvars) $*.c

printer.exe: $(OBJS) printer.res printer.def
    $(link) $(linkdebug) $(guiflags) -out:printer.exe $(OBJS) printer.res $(guilibs)
