/*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
* Copyright (c) Microsoft Corporation.  All Rights Reserved.
*/

// bindtoparent.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <wchar.h>
#include <objbase.h>
#include <activeds.h>

#define INPUT_BUFF_SIZE     MAX_PATH
#define SEARCH_FILTER_SIZE  INPUT_BUFF_SIZE + 64

HRESULT GetParentObject(IADs *pObject, //Pointer the object whose parent to bind to.
                        IADs **ppParent //Return a pointer to the parent object.
                        );

HRESULT FindUserByName(IDirectorySearch *pSearchBase, //Container to search
                       LPOLESTR szFindUser, //Name of user to find.
                       IADs **ppUser); //Return a pointer to the user

int __cdecl wmain( int argc, wchar_t *argv[])
{
    int inputsize = 0;

    //Handle the command line arguments.
    OLECHAR szBuffer[INPUT_BUFF_SIZE] = {0};
    if (NULL == argv[1])
    {
        wprintf(L"This program finds a user in the current Window 2000 domain\n");
        wprintf(L"and displays its parent container's ADsPath and binds to that container.\n");	
        wprintf(L"Enter Common Name of the user to find:");
        fgetws(szBuffer, ARRAYSIZE(szBuffer), stdin);

        // Note that fgetws retains final newline - strip this
        inputsize = wcsnlen(szBuffer, ARRAYSIZE(szBuffer));
        if (0 < inputsize && '\n' == szBuffer[inputsize-1])
            szBuffer[inputsize-1] = '\0';
    }
    else
        wcscpy_s(szBuffer, ARRAYSIZE(szBuffer), argv[1]);

    //if empty string, exit.
    if (0 == wcscmp(L"", szBuffer))
    {
        wprintf(L"Empty user name.\n");
        return 1; 
    }

    wprintf(L"\nFinding user: %s...\n",szBuffer);

    //Intialize COM
    HRESULT hr = S_OK;
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        wprintf(L"CoInitialize failed. hr=0x%x\n", hr);
        return 1;
    }

    //Get rootDSE and the domain container's DN.
    IADs *pObject = NULL;
    IADs *pParent = NULL;
    IDirectorySearch *pDS = NULL;
    OLECHAR szPath[MAX_PATH];
    VARIANT var;
    BSTR bstr = NULL;
    BSTR bstrC = NULL;

    VariantInit(&var);

    hr = ADsOpenObject(L"LDAP://rootDSE",
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
        IID_IADs,
        (void**)&pObject);
    if (FAILED(hr))
    {
        wprintf(L"Not Found. Could not bind to the domain. hr=0x%x\n", hr);
        goto CleanUp;
    }

    BSTR bNamingContext = NULL;
    bNamingContext = SysAllocString(L"defaultNamingContext");
    if (NULL == bNamingContext)
    {
        wprintf(L"SysAllocString for defaultNamingContext failed.\n");
        goto CleanUp;
    }

    hr = pObject->Get(bNamingContext,&var);
    SysFreeString(bNamingContext);
    bNamingContext = NULL;

    if (FAILED(hr))
    {
        wprintf(L"Get for %s failed. hr=0x%x\n", bNamingContext, hr);
        goto CleanUp;
    }

    wcscpy_s(szPath, ARRAYSIZE(szPath), L"LDAP://"); // If you're running on NT 4.0 or Win9.x machine, you need to 
    // add the server name e.g L"LDAP://myServer"
    wcscat_s(szPath, ARRAYSIZE(szPath), var.bstrVal);

    //Bind to the root of the current domain.
    hr = ADsOpenObject(szPath,
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
        IID_IDirectorySearch,
        (void**)&pDS);
    if (FAILED(hr))
    {
        wprintf (L"ADsOpenObject failed for path %s. hr=0x%x\n", szPath, hr);
        goto CleanUp;
    }

    if (pObject)
    {
        pObject->Release();
        pObject = NULL;
    }
    hr =  FindUserByName(pDS, //Container to search
        szBuffer, //Name of user to find.
        &pObject); //Return a pointer to the user
    if (FAILED(hr))
    {
        wprintf(L"User \"%s\" not Found.\n",szBuffer);
        wprintf (L"FindUserByName failed with the following HR: 0x%x\n", hr);
        goto CleanUp;
    }

    hr = GetParentObject(pObject, //Pointer the object whose parent to bind to.
        &pParent //Return a pointer to the parent object.
        );
    if (FAILED(hr))
    {
        wprintf(L"GetParentObject failed with hr: %x\n",hr);
        goto CleanUp;
    }
    wprintf(L"Successfully bound to parent container\n");

    //Get ADsPath
    hr = pParent->get_ADsPath(&bstr);
    if (FAILED(hr))
    {
        wprintf(L"get_ADsPath failed. hr = 0x%x\n", hr);
        goto CleanUp;
    }

    //Get the distinguishedName property
    BSTR bDName = SysAllocString(L"distinguishedName");
    if (NULL == bDName)
    {
        wprintf(L"SysAllocString for distinguishedName failed.\n");
        goto CleanUp;
    }

    VariantClear(&var);
    hr = pParent->Get(bDName, &var);
    SysFreeString(bDName);
    if (FAILED(hr))
    {
        wprintf(L"Get failed for %s. hr = 0x%x\n", bDName, hr);
        goto CleanUp;
    }

    //Get class
    hr = pParent->get_Class(&bstrC);
    if (FAILED(hr))
    {
        wprintf(L"get_Class failed with hr: 0x%x\n",hr);
        goto CleanUp;
    }

    wprintf(L"ADsPath: %s\n",bstr);
    wprintf(L"DN: %s\n",var.bstrVal);
    wprintf(L"Class: %s\n",bstrC);


CleanUp:

    if (bstr)
    {
        FreeADsStr(bstr);
        bstr = NULL;
    }
    if (bstrC)
    {
        FreeADsStr(bstrC);
        bstrC = NULL;
    }
    VariantClear(&var);

    if (pObject)
    {
        pObject->Release();
        pObject = NULL;
    }
    if (pDS)
    {
        pDS->Release();
        pDS = NULL;
    }
    if (pParent)
    {
        pParent->Release();
        pParent = NULL;
    }

    //Uninitalize COM
    CoUninitialize();

    return FAILED(hr)?1:0;
}

HRESULT GetParentObject(IADs *pObject, //Pointer the object whose parent to bind to.
                        IADs **ppParent //Return a pointer to the parent object.
                        )
{
    if ((!pObject)||(!ppParent))
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    BSTR bstr = NULL;
    hr = pObject->get_Parent(&bstr);
    if (SUCCEEDED(hr))
    {
        //Bind to the parent container.
        *ppParent = NULL;
        hr = ADsOpenObject(bstr,
            NULL,
            NULL,
            ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
            IID_IADs,
            (void**)ppParent);
        if(FAILED(hr))
        {
            if ((*ppParent))
            {
                (*ppParent)->Release();
                (*ppParent) = NULL;
            }
        }
    }
    if (bstr)
    {
        FreeADsStr(bstr);
    }
    return hr;
}





HRESULT FindUserByName(IDirectorySearch *pSearchBase, //Container to search
                       LPOLESTR szFindUser, //Name of user to find.
                       IADs **ppUser) //Return a pointer to the user
{
    HRESULT hr = S_OK;

    if ((!pSearchBase)||(!szFindUser))
        return E_INVALIDARG;

    //Create search filter
    OLECHAR szSearchFilter[SEARCH_FILTER_SIZE] = {0};
    OLECHAR szADsPath[MAX_PATH] = {0};
    wcscpy_s(szSearchFilter, ARRAYSIZE(szSearchFilter), L"(&(objectCategory=person)(objectClass=user)(cn=");
    wcscat_s(szSearchFilter, ARRAYSIZE(szSearchFilter), szFindUser);
    wcscat_s(szSearchFilter, ARRAYSIZE(szSearchFilter), L"))");

    //Search entire subtree from root.
    ADS_SEARCHPREF_INFO SearchPrefs;
    SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
    SearchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE;
    DWORD dwNumPrefs = 1;

    // COL for iterations
    ADS_SEARCH_COLUMN col = {0};
    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch = NULL;
    // Set the search preference

    hr = pSearchBase->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

    // Set attributes to return
    LPOLESTR pszAttribute[1] = {L"ADsPath"};

    // Execute the search
    hr = pSearchBase->ExecuteSearch(szSearchFilter,
        pszAttribute,
        1,
        &hSearch
        );

    DWORD noUsersFound = 0; 
    if (SUCCEEDED(hr))
    {    

        // Call IDirectorySearch::GetNextRow() to retrieve the next row 
        //of data
        while( pSearchBase->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
        {	

            if(noUsersFound>0 && ((*ppUser) != NULL))(*ppUser)->Release();
            wprintf(L"Found User %s.\n",szFindUser); 
            // Get the data for this column
            hr = pSearchBase->GetColumn( hSearch, pszAttribute[0], &col );
            if ( SUCCEEDED(hr) )
            {
                // Print the data for the column and free the column
                // Note the attribute we asked for is type CaseIgnoreString.
                wcscpy_s(szADsPath, ARRAYSIZE(szADsPath), col.pADsValues->CaseIgnoreString); 					
                wprintf(L"%s: %s\r\n",pszAttribute[0],col.pADsValues->CaseIgnoreString); 
                hr = ADsOpenObject(szADsPath,
                    NULL,
                    NULL,
                    ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
                    IID_IADs,
                    (void**)ppUser);
                if(SUCCEEDED(hr)) noUsersFound ++;

                pSearchBase->FreeColumn( &col );
            }
        }

    }

    // Close the search handle to clean up
    if (hSearch)
    {
        pSearchBase->CloseSearchHandle(hSearch);
        hSearch = NULL;
    }

    if (noUsersFound>1)
    {
        VARIANT var;
        wprintf(L"---------------------------------------------------\n"); 
        wprintf(L"More than one user with CN %s was found.\n",szFindUser); 

        BSTR bDName = SysAllocString(L"distinguishedName");
        if (NULL == bDName)
            return E_FAIL;

        (*ppUser)->Get(bDName, &var);
        SysFreeString(bDName);

        wprintf(L"Returning pointer to User (DN): %s\n",var.bstrVal); 
        wprintf(L"---------------------------------------------------\n"); 
        VariantClear(&var);
    }

    if(0 == noUsersFound) 
        hr = E_FAIL;

    return hr;
}
