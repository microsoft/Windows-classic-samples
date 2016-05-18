//---------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:  util.cxx
//
//  Contents:  Ansi to Unicode conversions and misc helper functions
//
//----------------------------------------------------------------------------

#include "main.hxx"

//
// Local functions
//


int
AnsiToUnicodeString(
    LPSTR pAnsi,
    LPWSTR pUnicode,
    DWORD StringLength
    )
{
    int iReturn;

    if( StringLength == 0 )
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

#if 0 //This function is commented out, as it's not being used.
int
UnicodeToAnsiString(
    LPWSTR pUnicode,
    LPSTR pAnsi,
    DWORD StringLength
    )
{
    LPSTR pTempBuf = NULL;
    INT   rc = 0;

    if( StringLength == 0 ) {

        //
        // StringLength is just the
        // number of characters in the string
        //
        StringLength = wcslen( pUnicode );
    }

    //
    // WideCharToMultiByte doesn't NULL terminate if we're copying
    // just part of the string, so terminate here.
    //

    pUnicode[StringLength] = 0;

    //
    // Include one for the NULL
    //
    StringLength++;

    //
    // Unfortunately, WideCharToMultiByte doesn't do conversion in place,
    // so allocate a temporary buffer, which we can then copy:
    //

    if( pAnsi == (LPSTR)pUnicode )
    {
        pTempBuf = (LPSTR)LocalAlloc( LPTR, StringLength );
        pAnsi = pTempBuf;
    }

    if( pAnsi )
    {
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  pUnicode,
                                  StringLength,
                                  pAnsi,
                                  StringLength,
                                  NULL,
                                  NULL );
    }

    /* If pTempBuf is non-null, we must copy the resulting string
     * so that it looks as if we did it in place:
     */
    if( pTempBuf && ( rc > 0 ) )
    {
        pAnsi = (LPSTR)pUnicode;
        int len = strlen(pTempBuf);
        strncpy_s(pAnsi, StringLength, pTempBuf);
        LocalFree( pTempBuf );
    }

    return rc;
}
#endif

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
                        strlen(pAnsiString)*sizeof(WCHAR) + sizeof(WCHAR)
                        );

    if (pUnicodeString) {

        AnsiToUnicodeString(
            pAnsiString,
            pUnicodeString,
            0
            );
    }

    return pUnicodeString;
}


void
FreeUnicodeString(
    LPWSTR  pUnicodeString
    )
{

    LocalFree(pUnicodeString);

    return;
}


//
// Misc helper functions for displaying data.
//


HRESULT
PrintVariant(
    VARIANT varPropData
    )
{
    HRESULT hr;
    BSTR bstrValue = NULL;

    switch (varPropData.vt) {
    case VT_I4:
        printf("%d", varPropData.lVal);
        break;
    case VT_BSTR:
        wprintf(L"%s", varPropData.bstrVal);
        break;

    case VT_BOOL:
        printf("%d", V_BOOL(&varPropData));
        break;

    case (VT_ARRAY | VT_VARIANT):
        PrintVariantArray(varPropData);
        break;

    case VT_DATE:
        hr = VarBstrFromDate(
                 varPropData.date,
                 LOCALE_SYSTEM_DEFAULT,
                 LOCALE_NOUSEROVERRIDE,
                 &bstrValue
                 );
        if ((S_OK == hr) && (NULL != bstrValue)) {
            wprintf(L"%s", bstrValue);
            SysFreeString(bstrValue);
        }
        else
            wprintf(L"Error: 0x%x \t Failed while executing VarBstrFromDate\n");
        break;

    default:
        printf("Data type is %d\n", varPropData.vt);
        break;

    }
    printf("\n");
    return(S_OK);
}


HRESULT
PrintVariantArray(
    VARIANT var
    )
{
    LONG dwSLBound = 0;
    LONG dwSUBound = 0;
    VARIANT v;
    LONG i;
    HRESULT hr = S_OK;

    if(!((V_VT(&var) &  VT_VARIANT) &&  V_ISARRAY(&var))) {
        return(E_FAIL);
    }

    //
    // Check that there is only one dimension in this array
    //

    if ((V_ARRAY(&var))->cDims != 1) {
        hr = E_FAIL;
        BAIL_ON_FAILURE(hr, L"Variant Array dimension is not equal to 1");
    }
    //
    // Check that there is atleast one element in this array
    //

    if ((V_ARRAY(&var))->rgsabound[0].cElements == 0){
        hr = E_FAIL;
        BAIL_ON_FAILURE(hr, L"Variant Array has zero elements");
    }

    //
    // We know that this is a valid single dimension array
    //

    hr = SafeArrayGetLBound(V_ARRAY(&var),
                            1,
                            (long FAR *)&dwSLBound
                            );
    BAIL_ON_FAILURE(hr, L"Failed while executing SafeArrayGetLBound");

    hr = SafeArrayGetUBound(V_ARRAY(&var),
                            1,
                            (long FAR *)&dwSUBound
                            );
    BAIL_ON_FAILURE(hr, L"Failed while executing SafeArrayGetUBound");

    VariantInit(&v);
    for (i = dwSLBound; i <= dwSUBound; i++) {
        hr = SafeArrayGetElement(V_ARRAY(&var),
                                (long FAR *)&i,
                                &v
                                );
        if (FAILED(hr)) {
            wprintf(L"Error: 0x%x \t Failed while executing SafeArrayGetElement", hr);
            continue;
        }
        if (i < dwSUBound) {
            wprintf(L"%s, ", v.bstrVal);
        } else {
            wprintf(L"%s", v.bstrVal);
        }
        VariantClear(&v);
    }
    return(S_OK);

error:
    return(hr);
}


HRESULT
PrintProperty(
    BSTR bstrPropName,
    HRESULT hRetVal,
    VARIANT varPropData
    )
{
    HRESULT hr = S_OK;

    switch (hRetVal) {

    case 0:
        wprintf(L"%-32s: ", bstrPropName);
        PrintVariant(varPropData);
        break;

    case E_ADS_CANT_CONVERT_DATATYPE:
        wprintf(L"%-32s: ", bstrPropName);
        printf("<Data could not be converted for display>\n");
        break;

    default:
        wprintf(L"%-32s: ", bstrPropName);
        printf("<Data not available>\n");
        break;

    }
    return(hr);
}

void
PrintUsage(
    void
    )
{
    printf("usage: adscmd [list|dump] <ADsPath>\n") ;
}

