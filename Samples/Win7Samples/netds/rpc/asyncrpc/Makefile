#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**                   AsyncRPC Application                  **#
#**         Copyright(c) Microsoft Corp. 1992-1998          **#
#**                                                         **#
#*************************************************************#

!include <win32.mak>

!if "$(CPU)" == "i386"
cflags = $(cflags) -D_CRTAPI1=_cdecl -D_CRTAPI2=_cdecl -DSECURITY_WIN32
!else
cflags = $(cflags) -D_CRTAPI1= -D_CRTAPI2= -DSECURITY_WIN32
!endif

cflags= $(cflags) -GS

!if "$(CPU)" == "i386"
link = $(link) -SAFESEH
!endif

all : AsyncRPCc AsyncRPCs

# Make the client side application AsyncRPCc
AsyncRPCc : AsyncRPCc.exe
AsyncRPCc.exe : AsyncRPCc.obj AsyncRPC_c.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:AsyncRPCc.exe \
      AsyncRPCc.obj AsyncRPC_c.obj spn.obj \
      rpcrt4.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# AsyncRPCc main program
AsyncRPCc.obj : AsyncRPCc.c AsyncRPC.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# AsyncRPCc stub
AsyncRPC_c.obj : AsyncRPC_c.c AsyncRPC.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Make the server side application
AsyncRPCs : AsyncRPCs.exe
AsyncRPCs.exe : AsyncRPCs.obj AsyncRPCp.obj AsyncRPC_s.obj spn.obj
    $(link) $(linkdebug) $(conflags) -out:AsyncRPCs.exe \
      AsyncRPCs.obj AsyncRPC_s.obj AsyncRPCp.obj spn.obj\
      rpcrt4.lib mpr.lib ntdsapi.lib secur32.lib netapi32.lib $(conlibsdll)

# AsyncRPC server main program
AsyncRPCs.obj : AsyncRPCs.c AsyncRPC.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# remote procedures
AsyncRPCp.obj : AsyncRPCp.c AsyncRPC.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# AsyncRPCs stub file
AsyncRPC_s.obj : AsyncRPC_s.c AsyncRPC.h
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# spn 
spn.obj : spn.c
   $(cc) $(cdebug) $(cflags) $(cvarsdll) /W3 $*.c

# Stubs and header file from the IDL file
AsyncRPC.h AsyncRPC_c.c AsyncRPC_s.c : AsyncRPC.idl AsyncRPC.acf
    midl $(MIDL_OPTIMIZATION) -oldnames AsyncRPC.idl -acf AsyncRPC.acf

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del AsyncRPC_c.c
    -del AsyncRPC_s.c
    -del AsyncRPC.h
