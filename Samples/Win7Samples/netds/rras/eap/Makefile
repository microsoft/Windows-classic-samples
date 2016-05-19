Proj = eap

!IF "$(TARGETOS)" == "WINNT"

!include <win32.mak>

cflags = $(cflags) -DUNICODE

LIBS=rtutils.lib     
       
OBJS=$(Proj).obj ceapcfg.obj exports.obj $(Proj).res

all: $(Proj).dll

$(Proj).dll: $(OBJS)
    $(link) $(ldebug) $(dlllflags) $(olelibs) $(LIBS)  \
    /DEF:$(Proj).def /OUT:$(Proj).dll $**

.c.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

.cpp.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.cpp

$(Proj).res: $(Proj).rc
    rc -r $*.rc

!ELSE
!MESSAGE Sorry this sample only builds for the Windows NT Platform
!ENDIF
