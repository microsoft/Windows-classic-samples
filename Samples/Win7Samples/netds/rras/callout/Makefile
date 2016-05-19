!include <win32.mak>

all: admindll.dll

.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) -DUNICODE $*.c

admindll.dll: admindll.obj
    $(link) $(linkdebug) $(dlllflags) \
      $** -out:admindll.dll -def:admindll.def   \
      $(conlibsdll) user32.lib shlwapi.lib
