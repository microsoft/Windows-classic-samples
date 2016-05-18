//---------------------------------------------------------------------------
//
//  ADSI 2.5 Sample Code - DsSrch
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996 - 2000.
//
//  File:  util.cxx
//
//  Contents:  Ansi to Unicode conversions and misc helper functions
//
//----------------------------------------------------------------------------//------------------------------------------------------------------------------

#include "main.hxx"




void
PrintUsage(
    void
    )
{
    printf("\nUsage: dssrch /b <baseObject> /f <search_filter> [/f <attrlist>] [/p <preference>=value>] ");
    printf(" [/u <UserName> <Password>] [/t <flagName>=<value> \n");
    printf("\n   where:\n" );
    printf("   baseObject     = ADsPath of the base of the search\n");
    printf("   search_filter  = search filter string in LDAP format\n" );
    printf("   attrlist       = list of the attributes to display\n" );
    printf("   preference could be one of:\n");
    printf("   Asynchronous, AttrTypesOnly, DerefAliases, SizeLimit, TimeLimit,\n");
    printf("   TimeOut, PageSize, SearchScope, SortOn, CacheResults\n");
    printf("   flagName could be one of:\n");
    printf("   SecureAuth or UseEncrypt\n");
    printf("   value is yes/no for a Boolean and the respective integer for integers\n");
    printf("   scope is one of \"Base\", \"OneLevel\", or \"Subtree\"\n");

    printf("\nFor Example: dssrch /b NDS://ntmarst/ms /f \"(object Class=*)\" ");
    printf(" /a  \"ADsPath, name, description\" /p searchScope=onelevel /p searchscope=onelevel\n\n OR \n");


    printf("\n dssrch /b \"LDAP://test.mysite.com/");
    printf("OU=testOU,DC=test,DC=mysite,DC=com\"  /f \"(objectClass=*)\"  /a \"ADsPath, name, usnchanged\" ");
    printf(" /u \"CN=user1,CN=Users,DC=test,DC=mysite,DC=COM\" \"secret~1\" ");
    printf("/p searchScope=onelevel /t secureauth=yes /p SortOn=name /p CacheResults=no\n");

}


//
// Print the data depending on its type.
//

void
PrintColumn(
    PADS_SEARCH_COLUMN pColumn, 
    LPWSTR pszColumnName
    )
{

    ULONG j, k;

    if (!pColumn) {
        return;
    }

    wprintf(
        L"%s = ",
        pszColumnName
        );

    for (k=0; k < pColumn->dwNumValues; k++) {
        if (k > 0) 
            wprintf(L"#  ");

        switch(pColumn->dwADsType) {
        case ADSTYPE_DN_STRING         :
            wprintf(
                L"%s  ",
                (LPWSTR) pColumn->pADsValues[k].DNString
                );
            break;
        case ADSTYPE_CASE_EXACT_STRING :
            wprintf(
                L"%s  ",
                (LPWSTR) pColumn->pADsValues[k].CaseExactString
                );
            break;
        case ADSTYPE_CASE_IGNORE_STRING:
            wprintf(
                L"%s  ",
                (LPWSTR) pColumn->pADsValues[k].CaseIgnoreString
                );
            break;
        case ADSTYPE_PRINTABLE_STRING  :
            wprintf(
                L"%s  ",
                (LPWSTR) pColumn->pADsValues[k].PrintableString
                );
            break;
        case ADSTYPE_NUMERIC_STRING    :
            wprintf(
                L"%s  ",
                (LPWSTR) pColumn->pADsValues[k].NumericString
                );
            break;
    
        case ADSTYPE_OBJECT_CLASS    :
            wprintf(
                L"%s  ",
                (LPWSTR) pColumn->pADsValues[k].ClassName
                );
            break;
    
        case ADSTYPE_BOOLEAN           :
            wprintf(
                L"%s  ",
                (DWORD) pColumn->pADsValues[k].Boolean ? 
                L"TRUE" : L"FALSE"
                );
            break;
    
        case ADSTYPE_INTEGER           :
            wprintf(
                L"%d  ",
                (DWORD) pColumn->pADsValues[k].Integer 
                );
            break;
    
        case ADSTYPE_OCTET_STRING      :
            for (j=0; j<pColumn->pADsValues[k].OctetString.dwLength; j++) {
                printf(
                    "%02x",
                    ((BYTE *)pColumn->pADsValues[k].OctetString.lpValue)[j]
                    );
            }
            break;
    
        case ADSTYPE_LARGE_INTEGER     :
            wprintf(
                L"%I64d  ",
                pColumn->pADsValues[k].LargeInteger 
                );
            break;
    
        case ADSTYPE_UTC_TIME          :
            wprintf(
                L"(date value) "
                );
            break;
        case ADSTYPE_PROV_SPECIFIC     :
            wprintf(
                L"(provider specific value) "
                );
            break;
    
        }
    }

    printf("\n");
}


int
AnsiToUnicodeString(
    LPSTR pAnsi,
    LPWSTR pUnicode,
    DWORD StringLength
    )
{
    int iReturn;

    if( StringLength == NULL_TERMINATED )
        StringLength = strlen( pAnsi );

    iReturn = MultiByteToWideChar(CP_ACP,
                                  MB_PRECOMPOSED,
                                  pAnsi,
                                  StringLength + 1,
                                  pUnicode,
                                  StringLength + 1 ); 

    //
    // Ensure NULL termination.
    //
    pUnicode[StringLength] = 0;

    return iReturn;
}

LPWSTR
AllocateUnicodeString(
    LPSTR  pAnsiString
    )
{ 
    LPWSTR  pUnicodeString = NULL;

    if (!pAnsiString)
        return NULL;

    pUnicodeString = (LPWSTR)LocalAlloc(
                        LPTR,
                        strlen(pAnsiString)*sizeof(WCHAR) +sizeof(WCHAR)
                        );

    if (pUnicodeString) {

        AnsiToUnicodeString(
            pAnsiString,
            pUnicodeString,
            NULL_TERMINATED
            );
    }

    return pUnicodeString;
}


void
FreeUnicodeString(
    LPWSTR  pUnicodeString
    )
{
    if (!pUnicodeString)
        return;

    LocalFree(pUnicodeString);

    return;
}

