! include <win32.mak>

# define these if you want a debug version

all:netinfo.exe 

LIBS =  ws2_32.lib iphlpapi.lib

.c.obj:
	 $(cc) $(cdebug) $(cflags) $(cvarsmt) $*.c


netinfo.exe:netinfo.obj 
	$(link) $(ldebug) $(conlflags) -out:$*.exe $*.obj $(conlibsmt)  $(LIBS)

-pdb:$(PDB)  \


clean:
	-del *.obj

cleanall:clean
	-del *.exe
