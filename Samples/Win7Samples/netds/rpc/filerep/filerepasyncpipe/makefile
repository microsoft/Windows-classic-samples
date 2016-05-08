#*************************************************************#
#**                                                         **#
#**                 Microsoft RPC Examples                  **#
#**          Async Pipe File Replication Service            **#
#**         Copyright(c) Microsoft Corp. 1992-2002          **#
#**                                                         **#
#*************************************************************#

!include <win32.mak>

# Compile with UNICODE support
cflags = $(cflags) $(cdebug) -D_UNICODE -DUNICODE 
midlflags = $(MIDL_OPTIMIZATION) -D_UNICODE



!if "$(NODEBUG)" != "1"
MSVCRT = msvcrtd.lib
LIBC = libcmtd.lib
!else
MSVCRT = msvcrt.lib
LIBC = libcmt.lib
!endif

cflags = $(cflags) -GS 

!if "$(CPU)" == "i386"
link = $(link) -SAFESEH
!endif

!if "$(DEBUG)" == "1"
cflags = $(cflags) -DDEBUG1 -DDBG
DBG_DEPS = DbgMsg.obj
DBG_LIBS = DbgMsg.obj
!elseif "$(DEBUG)" == "2"
cflags = $(cflags) -DDEBUG1 -DDBG -DDEBUG2 -D_DEBUG
DBG_DEPS = DbgMsg.obj
DBG_LIBS = DbgMsg.obj
!endif

!if "$(PROF)" == "1"
cflags = $(cflags) -DPROF
PROF_DEPS = Prof.obj
PROF_LIBS = Prof.obj
!else
!endif

!if "$(STRESS)" == "1"
cflags = $(cflags) -DRETRY_EXCEPTION -DLARGE_RES_BOUNDS
!else
!endif

!if "$(NO_SEC)" == "1"
cflags = $(cflags) -DNO_SEC
!else
!endif

all : FileRep FileRepService FileRepServer

# Make the file replication utility
FileRep : FileRep.obj common.obj FileRepClient_c.obj 
	$(link) $(lflags) $(ldebug) $(baselibs) -out:FileRep.exe \
        FileRep.obj common.obj FileRepClient_c.obj \
        rpcrt4.lib Ntdsapi.lib Advapi32.lib kernel32.lib shell32.lib \
        $(MSVCRT) /NODEFAULTLIB:libcmt

FileRep.obj : FileRep.cpp FileRepClient.h
   $(cc) $(cflags) $*.cpp

# Make the services
FileRepService : FileRepService.obj common.obj Service.obj Resources.obj \
                 FileRepClientProc.obj FileRepServerProc.obj \
                 FileRepClient_s.obj \
                 FileRepServer_c.obj FileRepServer_s.obj \
                 $(PROF_DEPS) $(DBG_DEPS)
	$(link) $(lflags) $(ldebug) $(baselibs) -out:FileRepService.exe \
        FileRepService.obj common.obj Service.obj Resources.obj \
        FileRepClientProc.obj FileRepServerProc.obj \
        FileRepClient_s.obj \
        FileRepServer_c.obj FileRepServer_s.obj \
        $(PROF_LIBS) $(DBG_LIBS) \
        rpcrt4.lib advapi32.lib kernel32.lib shell32.lib Secur32.lib \
        Ntdsapi.lib Netapi32.lib $(MSVCRT) /NODEFAULTLIB:libcmt

FileRepService.obj : FileRepService.cpp common.h \
                     FileRepClient.h FileRepServer.h
        $(cc) $(cflags) $*.cpp

# Make the server
FileRepServer : FileRepServer.obj common.obj Service.obj Resources.obj \
                FileRepClientProc.obj FileRepServerProc.obj \
                FileRepClient_s.obj \
                FileRepServer_c.obj FileRepServer_s.obj \
                $(PROF_DEPS) $(DBG_DEPS)
	$(link) $(lflags) $(ldebug) $(baselibs) -out:FileRepServer.exe \
        FileRepServer.obj common.obj Service.obj Resources.obj \
        FileRepClientProc.obj FileRepServerProc.obj \
        FileRepClient_s.obj \
        FileRepServer_c.obj FileRepServer_s.obj \
        $(PROF_LIBS) $(DBG_LIBS) \
        rpcrt4.lib advapi32.lib kernel32.lib shell32.lib Secur32.lib \
        Ntdsapi.lib Netapi32.lib $(MSVCRT) /NODEFAULTLIB:libcmt

FileRepServer.obj : FileRepServer.cpp common.h \
                    FileRepClient.h FileRepServer.h
        $(cc) $(cflags) $*.cpp

FileRepClientProc.obj : FileRepClientProc.cpp FileRepClient.h common.h
        $(cc) $(cflags) $*.cpp

FileRepServerProc.obj : FileRepServerProc.cpp FileRepServer.h common.h
        $(cc) $(cflags) $*.cpp

Resources.obj : Resources.cpp Resources.h
        $(cc) $(cflags) $*.cpp

Service.obj : Service.cpp Service.h
        $(cc) $(cflags) $*.cpp

common.obj : common.cpp common.h
        $(cc) $(cflags) $*.cpp

DbgMsg.obj : DbgMsg.cpp DbgMsg.h
        $(cc) $(cflags) $*.cpp

Prof.obj : Prof.cpp Prof.h
        $(cc) $(cflags) $*.cpp

FileRepServer_c.obj : FileRepServer_c.c FileRepServer.h
        $(cc) $(cflags) $*.c

FileRepServer_s.obj : FileRepServer_s.c FileRepServer.h
        $(cc) $(cflags) $*.c

FileRepClient_c.obj : FileRepClient_c.c FileRepClient.h
        $(cc) $(cflags) $*.c

FileRepClient_s.obj : FileRepClient_s.c FileRepClient.h
        $(cc) $(cflags) $*.c

# Make stubs and header files from the IDL files
FileRepClient.h FileRepClient_c.c FileRepClient_s.c : FileRepClient.idl
	$(midl) $(midlflags) FileRepClient.idl

FileRepServer.h FileRepServer_c.c FileRepServer_s.c : FileRepServer.idl
	$(midl) $(midlflags) -prefix client "c_" server "s_"  FileRepServer.idl

# Clean up everything
cleanall : clean
        -del *.exe
        -del *~

# Clean up everything but the .EXEs
clean :
        -del *.obj
        -del FileRepServer_c.c
        -del FileRepServer_s.c
        -del FileRepServer.h
        -del FileRepClient_c.c
        -del FileRepClient_s.c
        -del FileRepClient.h

