#ifndef __MAIN_HXX
#define __MAIN_HXX

//
// System Includes
//

#define UNICODE
#define _UNICODE
#define INC_OLE2
#include <windows.h>

//
// CRunTime Includes
//

#include <stdlib.h>
#include <limits.h>
#include <io.h>
#include <stdio.h>
#include <stddef.h>


//
// Public ADs includes
//

#ifdef __cplusplus
extern "C" {
#endif

#include "activeds.h"
#include "oledb.h"
#include "oledberr.h"

#ifdef __cplusplus
}
#endif

#define NULL_TERMINATED 0

//
// *********  Useful macros
//

#define BAIL_ON_NULL(p)       \
        if (!(p)) {           \
                goto error;   \
        }

#define BAIL_ON_FAILURE(hr)   \
        if (FAILED(hr)) {     \
                goto error;   \
        }


#define FREE_INTERFACE(pInterface) \
        if (pInterface) {          \
            pInterface->Release(); \
            pInterface=NULL;       \
        }


#define ADS_FREE(pMem)     \
        if (pMem) {    \
            FreeADsMem(pMem); \
            pMem = NULL;     \
        }


#define FREE_UNICODE_STRING(pMem)     \
        if (pMem) {    \
            FreeUnicodeString(pMem); \
            pMem = NULL;     \
        }


void
PrintColumn(
    PADS_SEARCH_COLUMN pColumn, 
    LPWSTR pszColumnName
    );

int
AnsiToUnicodeString(
    LPSTR pAnsi,
    LPWSTR pUnicode,
    DWORD StringLength
    );

LPWSTR
AllocateUnicodeString(
    LPSTR  pAnsiString
    );

void
FreeUnicodeString(
    LPWSTR  pUnicodeString
    );

void
PrintUsage(
    void
    );


HRESULT 
ProcessArgs(
    int argc,
    char * argv[]
    );


LPWSTR
RemoveWhiteSpaces(
    LPWSTR pszText
    );

#endif  // __MAIN_HXX
