#ifndef _RASNAPADMIN_H_
#define _RASNAPADMIN_H_

#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <mprapi.h>
#include <raserror.h>
#include <mprerror.h>
// DEFINES

#define LOG_FILE	L"log.txt"
#define MAX_IPV6_STRING 46
#define IPV6_INTERFACE_ID_LENGTH_IN_BYTES 8
#define ADMINDLL_KEY	L"SOFTWARE\\Microsoft\\Ras\\AdminDll"
#define DISPLAYNAME_VALUE	L"DisplayName"
// PROTOTYPES

DWORD InitializeDll();
void CleanUp();

#endif
