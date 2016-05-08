// bindtoparen.cpp : Defines the entry point for the console application.
//
#include <wchar.h>
#include <objbase.h>
#include <activeds.h>

//For converting sid to string.
//Make sure you define UNICODE
//Need to define version 5 for Windows 2000
#define _WIN32_WINNT 0x0500
#include <sddl.h>


HRESULT GetLPBYTEtoOctetString(VARIANT *pVar, //IN. Pointer to variant containing the octetstring.
                               LPBYTE *ppByte //OUT. Return LPBYTE to the data represented in octetstring.
							   );

HRESULT FindUserByName(IDirectorySearch *pSearchBase, //Container to search
					   LPOLESTR szFindUser, //Name of user to find.
					   IADs **ppUser); //Return a pointer to the user



void wmain( int argc, wchar_t *argv[ ])
{

//Handle the command line arguments.
LPOLESTR pszBuffer = NULL;
pszBuffer = new OLECHAR[MAX_PATH*2];
if(pszBuffer == NULL)
    goto ret;
if (argv[1] == NULL)
{
	wprintf(L"This program finds a user in the current Window 2000 domain\n");
	wprintf(L"and displays its objectSid property in string form.\n");
	wprintf(L"This program demonstrates reading a property of type octet string.\n\n");
	
	wprintf(L"Enter Common Name of the user to find:");
	if ( !_getws_s(pszBuffer, MAX_PATH*2))
	{
		delete [] pszBuffer;
		wprintf(L"String exceeded buffer size.\n\n");
		return;
	}
}
else
   if ( !wcscpy_s(pszBuffer, MAX_PATH*2, argv[1]))
   {
	    delete [] pszBuffer;
		wprintf(L"String exceeded buffer size.\n\n");
		return;
   }
//if empty string, exit.
if (0==wcscmp(L"", pszBuffer))
   goto ret;
	
wprintf(L"\nFinding user: %s...\n",pszBuffer);
	
//Intialize COM
CoInitialize(NULL);
HRESULT hr = S_OK;
//Get rootDSE and the domain container's DN.
IADs *pObject = NULL;
IDirectorySearch *pDS = NULL;
LPOLESTR szPath = NULL;
szPath = new OLECHAR[MAX_PATH];
if(szPath == NULL)
    goto ret;

VARIANT var;
hr = ADsOpenObject(L"LDAP://rootDSE",
				 NULL,
				 NULL,
				 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
				 IID_IADs,
				 (void**)&pObject);
if (FAILED(hr))
{
   wprintf(L"Not Found. Could not bind to the domain.\n");
   if (pObject)
     pObject->Release();
   goto ret;
}

VariantInit(&var);
hr = pObject->Get(L"defaultNamingContext",&var);
if (SUCCEEDED(hr))
{
	wcscpy_s(szPath,MAX_PATH,L"LDAP://");
	wcscat_s(szPath,MAX_PATH,var.bstrVal);
	VariantClear(&var);
	if (pObject)
	{
	   pObject->Release();
	   pObject = NULL;
	}
	//Bind to the root of the current domain.
	hr = ADsOpenObject(szPath,
					 NULL,
					 NULL,
					 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
					 IID_IDirectorySearch,
					 (void**)&pDS);
	if (SUCCEEDED(hr))
	{
		hr =  FindUserByName(pDS, //Container to search
						   pszBuffer, //Name of user to find.
						   &pObject); //Return a pointer to the user
		if (SUCCEEDED(hr))
		{
			//Get the objectSid property
			hr = pObject->Get(L"objectSid", &var);
			if (SUCCEEDED(hr))
			{
				LPBYTE pByte = NULL;
				wprintf (L"----------------------------------------------\n");
				wprintf (L"----------Call GetLPBYTEtoOctetString---------\n");
				wprintf (L"----------------------------------------------\n");
				hr = GetLPBYTEtoOctetString(&var, //IN. Pointer to variant containing the octetstring.
							   &pByte //OUT. Return LPBYTE to the data represented in octetstring.
							   );

				PSID pObjectSID = (PSID)pByte;
				//Convert SID to string.
				LPOLESTR szSID = NULL;
				ConvertSidToStringSid(pObjectSID, &szSID);
				wprintf(L"objectSid:%s\n",szSID);
				LocalFree(szSID);
				//Free the buffer.
				CoTaskMemFree(pByte);
			}
			else
				wprintf(L"Get method failed with hr: %x\n",hr);
			VariantClear(&var);
		}
		else
		{
            wprintf(L"User \"%s\" not Found.\n",pszBuffer);
			wprintf (L"FindUserByName failed with the following HR: %x\n", hr);
		}
		if (pObject)
			pObject->Release();
	}

	if (pDS)
	   pDS->Release();
}
ret:
    if(pszBuffer) delete pszBuffer;
    if(szPath)     delete szPath;
//Uninitalize COM
CoUninitialize();

	return;
}

//Function for handling Octet Strings returned by variants
//It allocates memory for data, copies the data to the buffer, and returns a pointer to the buffer.
//Caller must free the buffer with CoTaskMemFree.
HRESULT GetLPBYTEtoOctetString(VARIANT *pVar, //IN. Pointer to variant containing the octetstring.
                               LPBYTE *ppByte //OUT. Return LPBYTE to the data represented in octetstring.
							   )
{
HRESULT hr = E_FAIL;
//Check args
if ((!pVar)||(!ppByte))
  return E_INVALIDARG;
//Check the variant type for unsigned char array (octet string).
if ((pVar->vt)!=(VT_UI1|VT_ARRAY))
  return E_INVALIDARG;

void HUGEP *pArray;
long lLBound, lUBound;
hr = SafeArrayGetLBound(V_ARRAY(pVar), 1, &lLBound);
hr = SafeArrayGetUBound(V_ARRAY(pVar), 1, &lUBound);
//Get the count of elements
long cElements = lUBound-lLBound + 1;
hr = SafeArrayAccessData( V_ARRAY(pVar),
							  &pArray );
if (SUCCEEDED(hr))
{
 LPBYTE pTemp = (LPBYTE)pArray;
 *ppByte = (LPBYTE) CoTaskMemAlloc(cElements);
 if (*ppByte)
    memcpy(*ppByte, pTemp, cElements);
 else
	hr = E_OUTOFMEMORY;
}
SafeArrayUnaccessData( V_ARRAY(pVar) );

return hr;
}


HRESULT FindUserByName(IDirectorySearch *pSearchBase, //Container to search
					   LPOLESTR szFindUser, //Name of user to find.
					   IADs **ppUser) //Return a pointer to the user
{
    HRESULT hrObj = E_FAIL;
    HRESULT hr = E_FAIL;
	if ((!pSearchBase)||(!szFindUser))
		return E_INVALIDARG;
	//Create search filter
	LPOLESTR pszSearchFilter = NULL;
       LPOLESTR szADsPath = NULL;
	pszSearchFilter = new OLECHAR[MAX_PATH];
	szADsPath = new OLECHAR[MAX_PATH];
	if(pszSearchFilter == NULL || szADsPath == NULL)       
	{
		if ( pszSearchFilter )
			delete [] pszSearchFilter;
		if ( szADsPath )
			delete [] szADsPath;
		return  E_OUTOFMEMORY;
	}
	wcscpy_s(pszSearchFilter, MAX_PATH, L"(&(objectCategory=person)(objectClass=user)(cn=");
	wcscat_s(pszSearchFilter, MAX_PATH, szFindUser);
	wcscat_s(pszSearchFilter, MAX_PATH, L"))");
    //Search entire subtree from root.
	ADS_SEARCHPREF_INFO SearchPrefs;
	SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
	SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
	SearchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE;
    DWORD dwNumPrefs = 1;
	// COL for iterations
    ADS_SEARCH_COLUMN col;
    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch;
	// Set the search preference
    hr = pSearchBase->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        goto ret;
	// Set attributes to return
	CONST DWORD dwAttrNameSize = 1;
    LPOLESTR pszAttribute[dwAttrNameSize] = {L"ADsPath"};

    // Execute the search
    hr = pSearchBase->ExecuteSearch(pszSearchFilter,
		                          pszAttribute,
								  dwAttrNameSize,
								  &hSearch
								  );
	if (SUCCEEDED(hr))
	{    

    // Call IDirectorySearch::GetNextRow() to retrieve the next row 
    //of data
        while( pSearchBase->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
		{
            // loop through the array of passed column names,
            // print the data for each column
            for (DWORD x = 0; x < dwAttrNameSize; x++)
            {
			    // Get the data for this column
                hr = pSearchBase->GetColumn( hSearch, pszAttribute[x], &col );
			    if ( SUCCEEDED(hr) )
			    {
				    // Print the data for the column and free the column
					// Note the attribute we asked for is type CaseIgnoreString.
                    wcscpy_s(szADsPath, MAX_PATH, col.pADsValues->CaseIgnoreString); 
					hr = ADsOpenObject(szADsPath,
									 NULL,
									 NULL,
									 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
									 IID_IADs,
									 (void**)ppUser);
					if (SUCCEEDED(hr))
					{
                       wprintf(L"Found User.\n",szFindUser); 
                       wprintf(L"%s: %s\r\n",pszAttribute[x],col.pADsValues->CaseIgnoreString); 
					   hrObj = S_OK;
					}
				    pSearchBase->FreeColumn( &col );
			    }
			    else
				    hr = E_FAIL;
            }
		}
		// Close the search handle to clean up
        pSearchBase->CloseSearchHandle(hSearch);
	}
	if (FAILED(hrObj))
		hr = hrObj;

	ret:
        delete [] pszSearchFilter;
        delete [] szADsPath;
        return hr;
}





