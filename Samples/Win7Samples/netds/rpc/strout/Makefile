#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Samples                   **#
#**                   strout Application                    **#
#**       Copyright 1992 - 1999 Microsoft Corporation       **#
#**                                                         **#
#** This is the makefile used to make the client and the    **#
#** server for the string-out program where the client gets **#
#** the environment string from server, and displays them.  **#
#** This file will compile for ANSI characters, to compile  **#
#** for UNICODE, type nmake /f makefile.uni at the command  **#
#** line                                                    **#
#*************************************************************#
# FILE : MAKEFILE

!include <Win32.Mak>

cflags= $(cflags) -DSECURITY_WIN32 -GS


!if "$(CPU)" == "i386"
link = $(link) -SAFESEH
!endif

all : client server

# Make the client side application 
client : client.exe
client.exe : client.obj strout_c.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:client.exe \
      client.obj strout_c.obj spn.obj\
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# client main program
client.obj : client.c strout.h common.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# client stub
strout_c.obj : strout_c.c strout.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Make the server side application
server : server.exe
server.exe : server.obj remote.obj strout_s.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:server.exe \
      server.obj strout_s.obj remote.obj spn.obj\
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# server main program
server.obj : server.c strout.h common.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# remote procedures
remote.obj : remote.c strout.h common.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# server stub file
strout_s.obj : strout_s.c strout.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# spn 
spn.obj : spn.c
   $(cc) $(cdebug) $(cflags) $(cvarsdll) $*.c

# Stubs and header file from the IDL file
strout.h strout_c.c strout_s.c : strout.idl
	midl $(MIDL_OPTIMIZATION) -cpp_cmd $(cc) strout.idl
	
# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del strout_c.c
    -del strout_s.c
    -del strout.h
