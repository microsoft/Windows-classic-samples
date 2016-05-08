This program demonstrates dynamic linking and runtime binding. 
It does not need to be linked with WTSAPI.LIB, because it does 
not call any of the WTSAPI functions directly. Instead, 
LoadLibrary() is used to load WTSAPI.DLL, and then GetProcAddr() 
is used to obtain the addresses of the WTSEnumerateServers()
and WTSFreeMemory()functions. The EnumerateServers() function 
calls WTSEnumerateServers() by using a pointer to the function 
obtained by calling GetProcAddr().


If this program is run on Windows NT, which does not have WTSAPI.DLL 
available, it will print a message and terminate. 

