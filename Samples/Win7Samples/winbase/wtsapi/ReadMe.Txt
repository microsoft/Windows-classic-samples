WTSAPI Demonstration program - api.exe

This example program uses a menu interface 
to call most of the WtsAPI functions. You can 
also specify the ,username, server, and domain 
to request information. The username, server, and 
domain specified will affect the results of the 
WTSQueryUserConfig() and WTSSetUserConfig() functions. 

This sample requires MFC (Microsoft Foundation Class Library).  If 
you have the Platform SDK you can build the sample by type "nmake -a" 
or "nmake -a nodebug=1" in the sample directory or you can load the 
makefile in to Visual C++.

