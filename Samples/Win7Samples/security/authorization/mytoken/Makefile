!include <win32.mak>

proj=mytoken

all: $(proj).exe

$(proj).obj: $(proj).cpp
    $(cc) $(cflags) $(cvars) $(cdebug) $(proj).cpp

$(proj).exe: $(proj).obj
    $(link) $(ldebug) -out:$(proj).exe $(proj).obj advapi32.lib user32.lib
