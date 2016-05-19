//----------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996 - 2000.
//
//  File:       drt.cxx
//
//  Contents:   Main for DS Srch
//
//
//
//----------------------------------------------------------------------------

//
// System Includes
//

#define INC_OLE2
#include <windows.h>

//
// CRunTime Includes
//

#include <stdlib.h>
#include <limits.h>
#include <io.h>
#include <stdio.h>

//
// Public OleDs includes
//


//
// Private defines
//

#define BAIL_ON_NULL(p)       \
        if (!(p)) {           \
                goto error;   \
        }

#define BAIL_ON_FAILURE(hr)   \
        if (FAILED(hr)) {     \
                goto error;   \
        }

#include "activeds.h"
#include "main.hxx"

//
// Globals representing the parameters
//

LPWSTR pszSearchBase, pszSearchFilter, pszAttrNames[10], pszAttrList;
DWORD dwNumberAttributes = -1, dwMaxRows = (DWORD) -1;


//
// Preferences
//
BOOL fASynchronous=FALSE, fDerefAliases=FALSE, fAttrsOnly=FALSE;
DWORD fSizeLimit, fTimeLimit, dwTimeOut, dwPageSize, dwSearchScope;

ADS_SEARCHPREF_INFO pSearchPref[10];
ADS_SORTKEY pSortKey[10];
DWORD nSortKeys = 0;
DWORD dwCurrPref = 0;

LPWSTR pszUserName=NULL, pszPassword=NULL;
DWORD dwAuthFlags=0;
DWORD cErr=0;

char *prefNameLookup[] =
    {
    "ADS_SEARCHPREF_ASYNCHRONOUS",
    "ADS_SEARCHPREF_DEREF_ALIASES",
    "ADS_SEARCHPREF_SIZE_LIMIT",
    "ADS_SEARCHPREF_TIME_LIMIT",
    "ADS_SEARCHPREF_ATTRIBTYPES_ONLY",
    "ADS_SEARCHPREF_SEARCH_SCOPE",
    "ADS_SEARCHPREF_TIMEOUT",
    "ADS_SEARCHPREF_PAGESIZE",
    "ADS_SEARCHPREF_PAGED_TIME_LIMIT",
    "ADS_SEARCHPREF_CHASE_REFERRALS"
    "ADS_SEARCHPREF_CACHE_RESULTS"
    }; 


//+---------------------------------------------------------------------------
//
//  Function:   main
//
//  Synopsis:
//
//----------------------------------------------------------------------------
INT main(int argc, char * argv[])
{
    HRESULT hr=S_OK;
    IDirectorySearch *pDSSearch=NULL;
    ADS_SEARCH_HANDLE hSearchHandle=NULL;
    ADS_SEARCH_COLUMN Column;
    DWORD nRows = 0, i;
    LPWSTR pszColumnName = NULL;
	const int buffSize = 256;

#if 0       // Enable if you want to test binary values in filters and send
            // pszBinaryFilter instead of pszSearchFilter in ExecuteSearch

    WCHAR pszBinaryFilter[buffSize] = L"objectSid=";

    LPWSTR pszDest = NULL;

    BYTE column[100] = {
                    0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x15, 0x00,
                    0x00, 0x00, 0x59, 0x51, 0xb8, 0x17, 0x66, 0x72, 0x5d, 0x25,
                    0x64, 0x63, 0x3b, 0x0b, 0x29, 0x99, 0x21, 0x00 };

    hr = ADsEncodeBinaryData (
       column,
       28,
       &pszDest
       );

    wcsncat_s( pszBinaryFilter, buffSize, pszDest ,wcslen(pszDest) );

    FreeADsMem( pszDest );

#endif


    //
    // Sets the global variables with the parameters
    //
    hr = ProcessArgs(argc, argv);
    BAIL_ON_FAILURE(hr);

    hr = CoInitialize(NULL);

    if (FAILED(hr)) {
        printf("CoInitialize failed\n") ;
        return(1) ;
    }

    hr = ADsOpenObject(
        pszSearchBase,
        pszUserName,
        pszPassword,
        dwAuthFlags,
        IID_IDirectorySearch,
        (void **)&pDSSearch
        );


if(pszUserName)	ZeroMemory((PVOID)pszUserName,wcslen(pszUserName)*sizeof(WCHAR));
if(pszPassword)	ZeroMemory((PVOID)pszPassword,wcslen(pszPassword)*sizeof(WCHAR));

#if 0 // If you want to go with the default credentials

    hr = ADsGetObject(
                pszSearchBase,
                IID_IDirectorySearch,
                (void **)&pDSSearch
                );

#endif

    BAIL_ON_FAILURE(hr);

    if (dwCurrPref) {
        hr = pDSSearch->SetSearchPreference(
                 pSearchPref,
                 dwCurrPref
                 );
        BAIL_ON_FAILURE(hr);

        if (hr != S_OK) {
            for (i=0; i<dwCurrPref; i++) {
                if (pSearchPref[i].dwStatus != ADS_STATUS_S_OK) {
                    printf(
                        "Error in setting the preference %s: status = %d\n",
                           prefNameLookup[pSearchPref[i].dwSearchPref],
                           pSearchPref[i].dwStatus
                           );
                    cErr++;
                }
            }

        }
    }

    hr = pDSSearch->ExecuteSearch(
             pszSearchFilter,
             pszAttrNames,
             dwNumberAttributes,
             &hSearchHandle
              );
    BAIL_ON_FAILURE(hr);

    hr = pDSSearch->GetNextRow(
             hSearchHandle
             );
    BAIL_ON_FAILURE(hr);

    while (hr != S_ADS_NOMORE_ROWS && nRows < dwMaxRows) {
        nRows++;

        if (dwNumberAttributes == -1) {
            hr = pDSSearch->GetNextColumnName(
                     hSearchHandle,
                     &pszColumnName
                     );
            BAIL_ON_FAILURE(hr);

            while (hr != S_ADS_NOMORE_COLUMNS) {
                hr = pDSSearch->GetColumn(
                         hSearchHandle,
                         pszColumnName,
                         &Column
                         );

                if (FAILED(hr)  && hr != E_ADS_COLUMN_NOT_SET)
                    goto error;

                if (SUCCEEDED(hr)) {
                    PrintColumn(&Column, pszColumnName);
                    pDSSearch->FreeColumn(&Column);
                }

                FreeADsMem(pszColumnName);
                hr = pDSSearch->GetNextColumnName(
                         hSearchHandle,
                         &pszColumnName
                         );
                BAIL_ON_FAILURE(hr);
            }
            printf("\n");
        }
        else {
            for ( i=0; i<dwNumberAttributes; i++) { 
                hr = pDSSearch->GetColumn(
                         hSearchHandle,
                         pszAttrNames[i],
                         &Column
                         );

                if (hr == E_ADS_COLUMN_NOT_SET)
                    continue;

                BAIL_ON_FAILURE(hr);

                PrintColumn(&Column, pszAttrNames[i]);

                pDSSearch->FreeColumn(&Column);
            }
        printf("\n");
        }

        hr = pDSSearch->GetNextRow(
                 hSearchHandle
                 );
        BAIL_ON_FAILURE(hr);
    }

    wprintf (L"Total Rows: %d\n", nRows);

    if (cErr) {
        wprintf (L"%d warning(s) ignored", cErr);
    }

    if (hSearchHandle)
        pDSSearch->CloseSearchHandle(hSearchHandle);

    FREE_INTERFACE(pDSSearch);

    FREE_UNICODE_STRING(pszSearchBase) ;
    FREE_UNICODE_STRING(pszSearchFilter) ;
    FREE_UNICODE_STRING(pszAttrList) ;
    FREE_UNICODE_STRING(pszUserName) ;
    FREE_UNICODE_STRING(pszPassword) ;  
    for(i=0;i<nSortKeys;i++)    FREE_UNICODE_STRING(pSortKey[i].pszAttrType);
    CoUninitialize(); 

    return(0) ;

error:

    if (hSearchHandle)
        pDSSearch->CloseSearchHandle(hSearchHandle);

    FREE_INTERFACE(pDSSearch);

    FREE_UNICODE_STRING(pszSearchBase) ;
    FREE_UNICODE_STRING(pszSearchFilter) ;
    FREE_UNICODE_STRING(pszAttrList) ;
    FREE_UNICODE_STRING(pszUserName) ;
    FREE_UNICODE_STRING(pszPassword) ;
    for(i=0;i<nSortKeys;i++)    FREE_UNICODE_STRING(pSortKey[i].pszAttrType); 
    wprintf (L"Error! hr = 0x%x\n", hr);
    return(1) ;
}


//+---------------------------------------------------------------------------
//
//  Function:   ProcessArgs
//
//  Synopsis:
//
//----------------------------------------------------------------------------

HRESULT
ProcessArgs(
    int argc,
    char * argv[]
    )
{
    argc--;
    int currArg = 1;
    LPWSTR pTemp;
	LPWSTR pWCurrentPos = NULL;
    char *pszCurrPref, *pszCurrPrefValue;
	char *pszCurrentPos = NULL;

    char *pszAttr;
    DWORD nAttr=0;

    while (argc) {
        if (argv[currArg][0] != '/' && argv[currArg][0] != '-')
            BAIL_ON_FAILURE (E_FAIL);
        switch (argv[currArg][1]) {
        case 'b':
            argc--;
            currArg++;
            if (argc <= 0)
                BAIL_ON_FAILURE (E_FAIL);
            pszSearchBase = AllocateUnicodeString(argv[currArg]);
            BAIL_ON_NULL(pszSearchBase);
            break;

        case 'f':
            argc--;
            currArg++;
            if (argc <= 0)
                BAIL_ON_FAILURE (E_FAIL);
            pszSearchFilter = AllocateUnicodeString(argv[currArg]);
            BAIL_ON_NULL(pszSearchFilter);
            break;

        case 'a':
            argc--;
            currArg++;
            if (argc <= 0)
                BAIL_ON_FAILURE (E_FAIL);
            pszAttrList = AllocateUnicodeString(argv[currArg]);
            BAIL_ON_NULL(pszAttrList);

            dwNumberAttributes = 0;
            pTemp = wcstok_s(pszAttrList, L",", &pWCurrentPos);
            pszAttrNames[dwNumberAttributes] = RemoveWhiteSpaces(pTemp);
            dwNumberAttributes++;

            while (pTemp) {
                pTemp = wcstok_s(NULL, L",", &pWCurrentPos);
                pszAttrNames[dwNumberAttributes] = RemoveWhiteSpaces(pTemp);
                dwNumberAttributes++;
            }
            dwNumberAttributes--;
            break;

        case 'u':
            argc--;
            currArg++;
            if (argc <= 0)
                BAIL_ON_FAILURE (E_FAIL);
            pszUserName = AllocateUnicodeString(argv[currArg]);
            BAIL_ON_NULL(pszSearchBase);
            argc--;
            currArg++;
            if (argc <= 0)
                BAIL_ON_FAILURE (E_FAIL);
            pszPassword = AllocateUnicodeString(argv[currArg]);
            BAIL_ON_NULL(pszSearchBase);
            break;

        case 't':
            argc--;
            currArg++;
            if (argc <= 0)
                BAIL_ON_FAILURE (E_FAIL);

            pszCurrPref = strtok_s(argv[currArg], "=", &pszCurrentPos);
            pszCurrPrefValue = strtok_s(NULL, "=", &pszCurrentPos);
            if (!pszCurrPref || !pszCurrPrefValue)
                BAIL_ON_FAILURE(E_FAIL);

            if (!_stricmp(pszCurrPref, "SecureAuth")) {
                if (!_stricmp(pszCurrPrefValue, "yes" ))
                    dwAuthFlags |= ADS_SECURE_AUTHENTICATION;
                else if (!_stricmp(pszCurrPrefValue, "no" ))
                    dwAuthFlags &= ~ADS_SECURE_AUTHENTICATION;
                else
                    BAIL_ON_FAILURE(E_FAIL);
            }
            else if (!_stricmp(pszCurrPref, "UseEncrypt")) {
                if (!_stricmp(pszCurrPrefValue, "yes" ))
                    dwAuthFlags |= ADS_USE_ENCRYPTION;
                else if (!_stricmp(pszCurrPrefValue, "no" ))
                    dwAuthFlags &= ~ADS_USE_ENCRYPTION;
                else
                    BAIL_ON_FAILURE(E_FAIL);
            }
            else
                BAIL_ON_FAILURE(E_FAIL);
            break;

        case 'p':
            argc--;
            currArg++;
            if (argc <= 0)
                BAIL_ON_FAILURE (E_FAIL);

            pszCurrPref = strtok_s(argv[currArg], "=", &pszCurrentPos);
            pszCurrPrefValue = strtok_s(NULL, "=", &pszCurrentPos);
            if (!pszCurrPref || !pszCurrPrefValue)
                BAIL_ON_FAILURE(E_FAIL);

            if (!_stricmp(pszCurrPref, "asynchronous")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_BOOLEAN;
                if (!_stricmp(pszCurrPrefValue, "yes" ))
                    pSearchPref[dwCurrPref].vValue.Boolean = TRUE;
                else if (!_stricmp(pszCurrPrefValue, "no" ))
                    pSearchPref[dwCurrPref].vValue.Boolean = FALSE;
                else
                    BAIL_ON_FAILURE(E_FAIL);
            }
            else if (!_stricmp(pszCurrPref, "attrTypesOnly")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_ATTRIBTYPES_ONLY;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_BOOLEAN;
                if (!_stricmp(pszCurrPrefValue, "yes" ))
                    pSearchPref[dwCurrPref].vValue.Boolean = TRUE;
                else if (!_stricmp(pszCurrPrefValue, "no" ))
                    pSearchPref[dwCurrPref].vValue.Boolean = FALSE;
                else
                    BAIL_ON_FAILURE(E_FAIL);
            }
            else if (!_stricmp(pszCurrPref, "derefAliases")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_DEREF_ALIASES;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_INTEGER;
                if (!_stricmp(pszCurrPrefValue, "yes" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_DEREF_ALWAYS;
                else if (!_stricmp(pszCurrPrefValue, "no" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_DEREF_NEVER;
                else
                    BAIL_ON_FAILURE(E_FAIL);
            }
            else if (!_stricmp(pszCurrPref, "timeOut")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_TIMEOUT;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_INTEGER;
                pSearchPref[dwCurrPref].vValue.Integer = atoi(pszCurrPrefValue);
            }
            else if (!_stricmp(pszCurrPref, "timeLimit")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_TIME_LIMIT;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_INTEGER;
                pSearchPref[dwCurrPref].vValue.Integer = atoi(pszCurrPrefValue);
            }
            else if (!_stricmp(pszCurrPref, "sizeLimit")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_SIZE_LIMIT;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_INTEGER;
                pSearchPref[dwCurrPref].vValue.Integer = atoi(pszCurrPrefValue);
            }
            else if (!_stricmp(pszCurrPref, "PageSize")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_INTEGER;
                pSearchPref[dwCurrPref].vValue.Integer = atoi(pszCurrPrefValue);
            }
            else if (!_stricmp(pszCurrPref, "PagedTimeLimit")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_PAGED_TIME_LIMIT;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_INTEGER;
                pSearchPref[dwCurrPref].vValue.Integer = atoi(pszCurrPrefValue);
            }
            else if (!_stricmp(pszCurrPref, "SearchScope")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_INTEGER;
                if (!_stricmp(pszCurrPrefValue, "Base" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_SCOPE_BASE;
                else if (!_stricmp(pszCurrPrefValue, "OneLevel" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_SCOPE_ONELEVEL;
                else if (!_stricmp(pszCurrPrefValue, "Subtree" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_SCOPE_SUBTREE;
                else
                    BAIL_ON_FAILURE(E_FAIL);
            }
            else if (!_stricmp(pszCurrPref, "ChaseReferrals")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_CHASE_REFERRALS;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_INTEGER;
                if (!_stricmp(pszCurrPrefValue, "always" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_CHASE_REFERRALS_ALWAYS;
                else if (!_stricmp(pszCurrPrefValue, "never" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_CHASE_REFERRALS_NEVER;
                else if (!_stricmp(pszCurrPrefValue, "external" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_CHASE_REFERRALS_EXTERNAL;
                else if (!_stricmp(pszCurrPrefValue, "subordinate" ))
                    pSearchPref[dwCurrPref].vValue.Integer = ADS_CHASE_REFERRALS_SUBORDINATE;
                else
                    BAIL_ON_FAILURE(E_FAIL);
            }
            else if (!_stricmp(pszCurrPref, "SortOn")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_SORT_ON;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_PROV_SPECIFIC;

                pszAttr = strtok_s(pszCurrPrefValue, ",", &pszCurrentPos);

                for (nAttr=0; pszAttr && nAttr < 10; nAttr++) {

                    pSortKey[nAttr].pszAttrType = AllocateUnicodeString(pszAttr);
                    pSortKey[nAttr].pszReserved = NULL;
                    pSortKey[nAttr].fReverseorder = 0;
		      nSortKeys++;

                    pszAttr = strtok_s(NULL, ",", &pszCurrentPos);
                }
                if (nAttr == 0 && nAttr >= 10) {
                    BAIL_ON_FAILURE(E_FAIL);
                }

                pSearchPref[dwCurrPref].vValue.ProviderSpecific.dwLength = sizeof(ADS_SORTKEY) * nAttr;
                pSearchPref[dwCurrPref].vValue.ProviderSpecific.lpValue = (LPBYTE) pSortKey;
            }
            else if (!_stricmp(pszCurrPref, "cacheResults")) {
                pSearchPref[dwCurrPref].dwSearchPref = ADS_SEARCHPREF_CACHE_RESULTS;
                pSearchPref[dwCurrPref].vValue.dwType = ADSTYPE_BOOLEAN;
                if (!_stricmp(pszCurrPrefValue, "yes" ))
                    pSearchPref[dwCurrPref].vValue.Boolean = TRUE;
                else if (!_stricmp(pszCurrPrefValue, "no" ))
                    pSearchPref[dwCurrPref].vValue.Boolean = FALSE;
                else
                    BAIL_ON_FAILURE(E_FAIL);
            }
            else
                BAIL_ON_FAILURE(E_FAIL);

            dwCurrPref++;
            break;

        case 'n':
            argc--;
            currArg++;
            if (argc <= 0)
                BAIL_ON_FAILURE (E_FAIL);
            dwMaxRows = atoi(argv[currArg]);
            break;

       default:
            BAIL_ON_FAILURE(E_FAIL);
        }

        argc--;
        currArg++;
    }

    //
    // Check for Mandatory arguments;
    //

    if (!pszSearchBase || !pszSearchFilter)
        BAIL_ON_FAILURE(E_FAIL);

    if (dwNumberAttributes == 0) {
        //
        // Get all the attributes
        //
        dwNumberAttributes = -1;
    }

    return (S_OK);
error:

    PrintUsage();
    return E_FAIL;

}


LPWSTR
RemoveWhiteSpaces(
    LPWSTR pszText)
{
    LPWSTR pChar;

    if(!pszText)
        return (pszText);

    while(*pszText && iswspace(*pszText))
        pszText++;

    for(pChar = pszText + wcslen(pszText) - 1; pChar >= pszText; pChar--) {
        if(!iswspace(*pChar))
            break;
        else
            *pChar = L'\0';
    }

    return pszText;
}

