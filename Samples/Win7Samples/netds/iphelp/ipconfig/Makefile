! include <win32.mak>

!if "$(CPU)" == "i386"
cflags = $(cflags) -D _USE_32BIT_TIME_T
!endif

# define these if you want a debug version
#PDB = ipconfig.pdb
#CDEBUG= -Zi -Fd"$(PDB)"
#LDEBUG = -debug:full

LIBS = wsock32.lib iphlpapi.lib

.c.obj:
	$(cc) $(cdebug) $(cflags) $(cvarsmt) $*.c

all:ipconfig.exe 

ipconfig.exe:ipconfig.obj
	$(link) $(ldebug) $(conlflags) -out:$*.exe $*.obj user32.lib $(conlibsmt) $(LIBS)

#
# uncomment the line below for a debug build
#	-pdb:$(PDB)  \
	
clean:
	-del *.obj

cleanall:clean
	-del *.exe
