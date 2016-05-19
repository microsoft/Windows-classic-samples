!include <win32.mak>

all: admapit.exe

.c.obj:
  $(cc) $(cdebug) $(cflags) $(cvars) -DUNICODE $*.c

admapit.exe: admapit.obj
    $(link) $(linkdebug) $(lflags) \
      $** -out:admapit.exe   \
      $(conlibs) shlwapi.lib mprapi.Lib user32.lib
