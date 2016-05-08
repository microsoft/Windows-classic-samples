// attributes.cpp : Defines the entry point for the console application.
//

#include <wchar.h>
#include <activeds.h>
#include <tchar.h>
#include <stdio.h>
#ifndef ADS_SYSTEMFLAG_ATTR_NOT_REPLICATED
#define ADS_SYSTEMFLAG_ATTR_NOT_REPLICATED         (0x00000001) // Non-replicated attribute
#endif

#ifndef ADS_SYSTEMFLAG_ATTR_IS_CONSTRUCTED
#define ADS_SYSTEMFLAG_ATTR_IS_CONSTRUCTED         (0x00000004) // Attribute is a constructed att
#endif

HRESULT FindAttributesByType(IDirectorySearch *pSchemaNC, //IDirectorySearch pointer to schema naming context.
         DWORD dwAttributeType, //Bit flags to search for in systemFlags
         LPWSTR pszAttributerNameType, //ldapDisplayName of the naming attribute you want to display for each returned attribute.
                                         //NULL returns common name.
         BOOL bIsExactMatch //TRUE to find attributes that have systemFlags exactly matching dwAttributeType
                            //FALSE to find attributes that have the dwAttributeType bit set (and possibly others).
        );

HRESULT FindIndexedAttributes(IDirectorySearch *pSchemaNC, //IDirectorySearch pointer to schema naming context.
         LPWSTR pszAttributeNameType, //ldapDisplayName of the naming attribute you want to display for each returned attribute.
                                         //NULL returns common name.
         BOOL bIsIndexed //TRUE to find indexed attributes.
                         //FALSE to find non-indexed attributes.
        );

HRESULT FindGCAttributes(IDirectorySearch *pSchemaNC, //IDirectorySearch pointer to schema naming context.
         LPWSTR pszAttributeNameType, //ldapDisplayName of the naming attribute you want to display for each returned attribute.
                                         //NULL returns common name.
         BOOL bInGC //TRUE to find indexed attributes.
                         //FALSE to find non-indexed attributes.
        );

#define LDAP_PROV L"LDAP://"

int main(int argc, char* argv[])
{
const size_t buffSize = MAX_PATH;
LPWSTR szPath = new WCHAR[buffSize];
HRESULT hr = S_OK;
IADs *pObject = NULL;
IDirectorySearch *pSchemaNC = NULL;
VARIANT var;

//Initialize COM
CoInitialize(NULL);

wprintf(L"This program displays the following types of attributes in the schema:\n");
wprintf(L"Non-Replicated, Indexed, Constructed, Global Catalog\n\n");
 
//wcscpy(szPath, L"LDAP://");
//Get rootDSE and the schema container's DN.
//Bind to current user's domain using current user's security context.
hr = ADsOpenObject(L"LDAP://rootDSE", //NOTE: If you're running NT 4.0, Win9x, you must add 
                                      // the server name, e.g LDAP://myServer/rootDSE
                 NULL,
                 NULL,
                 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
                 IID_IADs,
                 (void**)&pObject);

if (SUCCEEDED(hr))
{
    hr = pObject->Get(L"schemaNamingContext",&var);
    if (SUCCEEDED(hr))
    {
        memset(szPath, 0, MAX_PATH);
        if (MAX_PATH >= (wcslen(LDAP_PROV) + wcslen(var.bstrVal)))
        {
            wcsncpy_s(szPath,buffSize,LDAP_PROV, wcslen(LDAP_PROV));
            wcsncat_s(szPath,buffSize,var.bstrVal, wcslen(var.bstrVal));
        }
        hr = ADsOpenObject(szPath,
                         NULL,
                         NULL,
                         ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
                         IID_IDirectorySearch,
                         (void**)&pSchemaNC);

        if (SUCCEEDED(hr))
        {
            //Find non-replicated attributes
            wprintf (L"----------------------------------------------\n");
            wprintf(L"Non-Replicated attributes (stored on each domain controller but are not replicated elsewhere)\n");
            wprintf(L"Find non-replicated attributes\n");
            hr = FindAttributesByType(pSchemaNC, //IDirectorySearch pointer to schema naming context.
                                     ADS_SYSTEMFLAG_ATTR_NOT_REPLICATED, //Bit flags to search for in systemFlags
                                     L"ldapDisplayName",
                                     TRUE
                                     );

            //Find attributes included in the global catalog
            wprintf (L"----------------------------------------------\n");
            wprintf(L"Global Catalog attributes (replicated to the Global Catalog)\n");
            wprintf(L"Find attributes included in the global catalog\n");
            hr = FindGCAttributes(pSchemaNC,
                                 L"ldapDisplayName",
                                 TRUE
                                 );

            //Find constructed attributes
            wprintf (L"----------------------------------------------\n");
            wprintf(L"Constructed attributes (not stored in the directory but are calculated by the domain controller)\n");
            wprintf(L"Find constructed attributes\n");
            hr = FindAttributesByType(pSchemaNC, //IDirectorySearch pointer to schema naming context.
                                     ADS_SYSTEMFLAG_ATTR_IS_CONSTRUCTED, //Bit flags to search for in systemFlags
                                     L"ldapDisplayName",
                                     FALSE
                                     );


            //Find indexed attributes
            wprintf (L"----------------------------------------------\n");
            wprintf(L"Indexed attributes (indexed for efficient search)\n");
            wprintf(L"Find indexed attributes\n");
            hr = FindIndexedAttributes(pSchemaNC, //IDirectorySearch pointer to schema naming context.
                                      L"ldapDisplayName",
                                      TRUE
                                      );

        }
        if (pSchemaNC)
           pSchemaNC->Release();
    }
    VariantClear(&var);
}
if (pObject)
    pObject->Release();

// Uninitialize COM
CoUninitialize();
delete szPath;
return 0;
}

HRESULT FindAttributesByType(IDirectorySearch *pSchemaNC, //IDirectorySearch pointer to schema naming context.
         DWORD dwAttributeType, //Bit flags to search for in systemFlags
         LPWSTR pszAttributeNameType, //ldapDisplayName of the naming attribute you want to display for each returned attribute.
                                         //NULL returns common name.
         BOOL bIsExactMatch //TRUE to find attributes that have systemFlags exactly matching dwAttributeType
                            //FALSE to find attributes that have the dwAttributeType bit set (and possibly others).
        )
{
    //Create search filter
	const size_t buffSize = MAX_PATH*2;
    LPWSTR pszSearchFilter = new WCHAR[buffSize];
    if (bIsExactMatch)
       //Find attributes with systemFlags that exactly match dwAttributeType
       swprintf_s(pszSearchFilter, buffSize, L"(&(objectCategory=attributeSchema)(systemFlags=%d))",dwAttributeType);
    else
       //Find attributes with systemFlags that contain dwAttributeType
        swprintf_s(pszSearchFilter, buffSize, L"(&(objectCategory=attributeSchema)(systemFlags:1.2.840.113556.1.4.804:=%d))",dwAttributeType);

    //Attributes are one-level deep in the Schema container so only need to search one level.
    ADS_SEARCHPREF_INFO SearchPrefs;
    SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
    SearchPrefs.vValue.Integer = ADS_SCOPE_ONELEVEL;
    DWORD dwNumPrefs = 1;

    // COL for iterations
    ADS_SEARCH_COLUMN col;
    HRESULT hr;
    
    // Interface Pointers
    IADs    *pObj = NULL;
    IADs    * pIADs = NULL;

    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch;
    
    // Set the search preference
    hr = pSchemaNC->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

    CONST DWORD dwAttrNameSize = 1;
    LPWSTR pszAttribute[dwAttrNameSize];

    if (!pszAttributeNameType)
        pszAttribute[0] = L"cn";
    else
        pszAttribute[0] = pszAttributeNameType;


    // Execute the search
    hr = pSchemaNC->ExecuteSearch(pszSearchFilter,
                                  pszAttribute,
                                  dwAttrNameSize,
                                  &hSearch
                                  );
    if ( SUCCEEDED(hr) )
    {    

    // Call IDirectorySearch::GetNextRow() to retrieve the next row 
    //of data
        while( pSchemaNC->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
        {

            // loop through the array of passed column names,
            // print the data for each column
            for (DWORD x = 0; x < dwAttrNameSize; x++)
            {

                // Get the data for this column
                hr = pSchemaNC->GetColumn( hSearch, pszAttribute[x], &col );

                if ( SUCCEEDED(hr) )
                {

                    if (ADSTYPE_CASE_IGNORE_STRING == col.dwADsType)
                    {
                        // Print the data for the column and free the column
                        wprintf(L"%s: %s\r\n",pszAttribute[x],col.pADsValues->CaseIgnoreString); 
                    }
                    else
                        wprintf(L"<%s property is not a string>",pszAttribute[x]);

                    pSchemaNC->FreeColumn( &col );
                }
                else
                    wprintf(L"<Could not get value for %s property>",pszAttribute[x]);
                
            }
        }

        // Close the search handle to clean up
        pSchemaNC->CloseSearchHandle(hSearch);
    }
    delete pszSearchFilter;
    return hr;
}

HRESULT FindIndexedAttributes(IDirectorySearch *pSchemaNC, //IDirectorySearch pointer to schema naming context.
         LPWSTR pszAttributeNameType, //ldapDisplayName of the naming attribute you want to display for each returned attribute.
                                         //NULL returns common name.
         BOOL bIsIndexed //TRUE to find indexed attributes.
                         //FALSE to find non-indexed attributes.
        )
{
    //Create search filter
	const size_t buffSize = MAX_PATH*2;
    LPWSTR pszSearchFilter = new WCHAR[buffSize];
    DWORD dwIndexed;
    if (bIsIndexed)
       dwIndexed = 1;
    else
       dwIndexed = 0;
    
    swprintf_s(pszSearchFilter, buffSize, L"(&(objectCategory=attributeSchema)(searchFlags=%d))",dwIndexed);

    //Attributes are one-level deep in the Schema container so only need to search one level.
    ADS_SEARCHPREF_INFO SearchPrefs;
    SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
    SearchPrefs.vValue.Integer = ADS_SCOPE_ONELEVEL;
    DWORD dwNumPrefs = 1;

    // COL for iterations
    ADS_SEARCH_COLUMN col;
    HRESULT hr;
    
    // Interface Pointers
    IADs    *pObj = NULL;
    IADs    * pIADs = NULL;

    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch;
    
    // Set the search preference
    hr = pSchemaNC->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

    CONST DWORD dwAttrNameSize = 1;
    LPWSTR pszAttribute[dwAttrNameSize];

    if (!pszAttributeNameType)
        pszAttribute[0] = L"cn";
    else
        pszAttribute[0] = pszAttributeNameType;


    // Execute the search
    hr = pSchemaNC->ExecuteSearch(pszSearchFilter,
                                  pszAttribute,
                                  dwAttrNameSize,
                                  &hSearch
                                  );
    if ( SUCCEEDED(hr) )
    {    

    // Call IDirectorySearch::GetNextRow() to retrieve the next row 
    //of data
        while( pSchemaNC->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
        {

            // loop through the array of passed column names,
            // print the data for each column
            for (DWORD x = 0; x < dwAttrNameSize; x++)
            {

                // Get the data for this column
                hr = pSchemaNC->GetColumn( hSearch, pszAttribute[x], &col );

                if ( SUCCEEDED(hr) )
                {

                    if (ADSTYPE_CASE_IGNORE_STRING == col.dwADsType)
                    {
                        // Print the data for the column and free the column
                        wprintf(L"%s: %s\r\n",pszAttribute[x],col.pADsValues->CaseIgnoreString); 
                    }
                    else
                        wprintf(L"<%s property is not a string>",pszAttribute[x]);
                
                    pSchemaNC->FreeColumn( &col );
                }
                else
                    wprintf(L"<Could not get value for %s property>",pszAttribute[x]);
            }
        }

        // Close the search handle to clean up
        pSchemaNC->CloseSearchHandle(hSearch);
    }
    delete pszSearchFilter;
    return hr;
}

HRESULT FindGCAttributes(IDirectorySearch *pSchemaNC, //IDirectorySearch pointer to schema naming context.
         LPWSTR pszAttributeNameType, //ldapDisplayName of the naming attribute you want to display for each returned attribute.
                                         //NULL returns common name.
         BOOL bInGC //TRUE to find GC attributes.
                         //FALSE to find non-GC attributes.
        )
{
    //Create search filter
	const size_t buffSize = MAX_PATH*2;
    LPWSTR pszSearchFilter = new WCHAR[buffSize];
    LPWSTR szBool = NULL;
    if (bInGC)
       szBool = L"TRUE";
    else
       szBool = L"FALSE";
    
    swprintf_s(pszSearchFilter, buffSize, L"(&(objectCategory=attributeSchema)(isMemberOfPartialAttributeSet=%s))",szBool);

    //Attributes are one-level deep in the Schema container so only need to search one level.
    ADS_SEARCHPREF_INFO SearchPrefs;
    SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
    SearchPrefs.vValue.Integer = ADS_SCOPE_ONELEVEL;
    DWORD dwNumPrefs = 1;

    // COL for iterations
    ADS_SEARCH_COLUMN col;
    HRESULT hr;
    
    // Interface Pointers
    IADs    *pObj = NULL;
    IADs    * pIADs = NULL;

    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch;
    
    // Set the search preference
    hr = pSchemaNC->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

    CONST DWORD dwAttrNameSize = 1;
    LPWSTR pszAttribute[dwAttrNameSize];

    if (!pszAttributeNameType)
        pszAttribute[0] = L"cn";
    else
        pszAttribute[0] = pszAttributeNameType;


    // Execute the search
    hr = pSchemaNC->ExecuteSearch(pszSearchFilter,
                                  pszAttribute,
                                  dwAttrNameSize,
                                  &hSearch
                                  );
    if ( SUCCEEDED(hr) )
    {    

    // Call IDirectorySearch::GetNextRow() to retrieve the next row 
    //of data
        while( pSchemaNC->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
        {

            // loop through the array of passed column names,
            // print the data for each column
            for (DWORD x = 0; x < dwAttrNameSize; x++)
            {

                // Get the data for this column
                hr = pSchemaNC->GetColumn( hSearch, pszAttribute[x], &col );

                if ( SUCCEEDED(hr) )
                {

                    if (ADSTYPE_CASE_IGNORE_STRING == col.dwADsType)
                    {
                        // Print the data for the column and free the column
                        wprintf(L"%s: %s\r\n",pszAttribute[x],col.pADsValues->CaseIgnoreString); 
                    }
                    else
                        wprintf(L"<%s property is not a string>",pszAttribute[x]);

                    pSchemaNC->FreeColumn( &col );
                }
                else
                    wprintf(L"<Could not get value for %s property>",pszAttribute[x]);
            }
        }

        // Close the search handle to clean up
        pSchemaNC->CloseSearchHandle(hSearch);
    }
    delete pszSearchFilter;
    return hr;
}

