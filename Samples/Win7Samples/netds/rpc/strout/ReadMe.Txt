STROUT  


The STROUT sample demonstrates how to allocate memory at a server for 
a two-dimensional object (an array of pointers) and pass it back to the 
client as an [out]-only parameter. The client then frees the memory. 
This technique allows the stub to call the server without knowing in 
advance how much data will be returned.
This program also allows the user to compile either for UNICODE or ANSI.
See section below for guidelines on how to compile for UNICODE

 
FILES 
===== 
 
The directory samples\rpc\strout contains the following files for
building the sample distributed application STROUT: 
 
File          Description
-------------------------
 
README.TXT      Readme file for the strout sample 
STROUT.IDL      Interface definition language file 
STROUT.ACF      Attribute configuration file 
CLIENT.C        Client main program 
SERVER.C        Server main program 
COMMON.H        Common header file for all the files
REMOTE.C        Remote procedures 
MAKEFILE        nmake file to build 32-bit client and server applications
                for ANSI characters.
MAKEFILE.UNI    nmake file to build 32-bit client and server applications
                for UNICODE characters.

NMAKE builds the executable programs CLIENT.EXE (client) and
SERVER.EXE (server).

Note: The client and server applications can run on the same 
Microsoft Windows NT computer when you use different screen groups.



COMPILING FOR UNICODE:
======================

type nmake /f makefile.uni at the command line. This will cause
the compiler to use the file MAKEFILE.UNI instead of the MAKEFILE.
	

The reason behind the use of TEXT, TCHAR, _TUCHAR, _tprintf, _tcscpy, 
_tcscmp, and _tcslen is that these macros expand to either 
one byte character ANSI functions or to UNICODE (Wide characters) functions
when they are compiled
    TEXT    :   This macro will put an L in front of the string if we are 
                compiling for UNICODE
    TCHAR   :   Maps to either char or wchar_t 
    _TUCHAR :   Maps to either unsigned char or wchar_t 
    _tprintf:   Maps to either printf or wsprintf
    _tcslen :   Maps to either strlen or wcslen
    _tcscpy :   Maps to either strcpy or wcscpy
    _tcscmp :   Maps to either strcmp or wcscmp
