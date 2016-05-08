//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       enum.cxx
//
//  Contents:   Active Drectory container enumeration
//
//
//----------------------------------------------------------------------------

#include "main.hxx"


//
// Private defines
//

#define MAX_ADS_FILTERS   10
#define MAX_ADS_ENUM      100     // number of entries to read each time


//
// Local functions
//


HRESULT
PrintLongFormat(
    IADs * pObject
    );

//
//  List contents of a container identified by the ADsPath
//

int
DoList(char * AnsiADsPath)
{
    HRESULT hr;
    int i = 0 ;
    LPWSTR pszADsPath, apszTypes[MAX_ADS_FILTERS] ;

    if (!(pszADsPath = AllocateUnicodeString(AnsiADsPath))) {

        return(1) ;
    }

    apszTypes[0] = NULL ;

    //
    //  Filter may be set as follows. For example, to get users and group:
    //
    //  apszTypes[0] = L"User" ;
    //  apszTypes[1] = L"Group" ;
    //  apszTypes[2] = NULL ;
    //

    hr = EnumObject(
             pszADsPath,
             apszTypes,
             i
             );

    FreeUnicodeString(pszADsPath);

    return (FAILED(hr) ? 1 : 0) ;
}

//
// Enumerates the contents of a container object.
//

HRESULT
EnumObject(
    LPWSTR pszADsPath,
    LPWSTR * lppClassNames,
    DWORD dwClassNames
    )
{
    ULONG cElementFetched = 0L;
    IEnumVARIANT * pEnumVariant = NULL;
    VARIANT VarFilter, VariantArray[MAX_ADS_ENUM];

    HRESULT hr;
    IADsContainer * pADsContainer =  NULL;
    DWORD dwObjects = 0, dwEnumCount = 0, i = 0;
    BOOL  fContinue = TRUE;


    VariantInit(&VarFilter);

    hr = ADsGetObject(
                pszADsPath,
                IID_IADsContainer,
                (void **)&pADsContainer
                );

    BAIL_ON_FAILURE(hr, L"Failed to bind to container object");

    hr = ADsBuildVarArrayStr(
                lppClassNames,
                dwClassNames,
                &VarFilter
                );
    BAIL_ON_FAILURE(hr, L"Failed while executing ADsBuildVarArrayStr");


    hr = pADsContainer->put_Filter(VarFilter);
    BAIL_ON_FAILURE(hr, L"Failed to put_Filter on container object");

    hr = ADsBuildEnumerator(
            pADsContainer,
            &pEnumVariant
            );
    BAIL_ON_FAILURE(hr, L"Failed while executing ADsBuildEnumerator");



    while (fContinue) {

        IADs *pObject ;

        hr = ADsEnumerateNext(
                    pEnumVariant,
                    MAX_ADS_ENUM,
                    VariantArray,
                    &cElementFetched
                    );

        if (hr == S_FALSE) {
            fContinue = FALSE;
        }

        dwEnumCount++;

        for (i = 0; i < cElementFetched; i++ ) {

            IDispatch *pDispatch = NULL;

            pDispatch = VariantArray[i].pdispVal;

            hr = pDispatch->QueryInterface(IID_IADs,
                                           (VOID **) &pObject) ;
            BAIL_ON_FAILURE(hr, L"Failed to QI the enumerate varDisp to IADs interface");

            PrintLongFormat(pObject);

            pObject->Release();
            pDispatch->Release();
        }

        memset(VariantArray, 0, sizeof(VARIANT)*MAX_ADS_ENUM);

        dwObjects += cElementFetched;

    }

    printf("Total Number of Objects enumerated is %d\n", dwObjects);

    ADsFreeEnumerator( pEnumVariant ); // You don't need to Release if you call ADsFreeEnumerator


    if (pADsContainer) {
        pADsContainer->Release();
    }

    VariantClear(&VarFilter);

    return(S_OK);

error:
    if (FAILED(hr)) {

        wprintf(L"Unable to list contents of: %s\n", pszADsPath) ;
    }

//exitpoint:

    if (pEnumVariant) {
        pEnumVariant->Release();
    }

    VariantClear(&VarFilter);


    if (pADsContainer) {
        pADsContainer->Release();
    }

    return(hr);
}

HRESULT
PrintLongFormat(IADs * pObject)
{

    HRESULT hr = S_OK;
    BSTR bstrName = NULL;
    BSTR bstrClass = NULL;
    BSTR bstrSchema = NULL;

    hr = pObject->get_Name(&bstrName) ;
    BAIL_ON_FAILURE(hr, L"Failed to get the name of the object");

    hr = pObject->get_Class(&bstrClass);
    BAIL_ON_FAILURE(hr, L"Failed to get the class of the object");

    // hr = pObject->get_Schema(&bstrSchema);

    wprintf(L"  %s(%s)\n", bstrName, bstrClass) ;

error:
    if (bstrClass) {
        SysFreeString(bstrClass);
    }
    if (bstrName) {
        SysFreeString(bstrName);
    }
    if (bstrSchema) {
        SysFreeString(bstrSchema);
    }
    return(hr);
}


