// Search.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"


int main(int argc, char* argv[])
{
	HRESULT hr;
	IDirectorySearch *pSearch;
	CoInitialize(NULL);


	///////////////////////////////////////////////
	// Bind to Object, it serves as a base search
	///////////////////////////////////////////////
	hr = ADsGetObject(L"LDAP://DC=testDom1,DC=testDom2,DC=microsoft,DC=com",
		              IID_IDirectorySearch,
					  (void**) &pSearch );

	if ( !SUCCEEDED(hr) )
	{
		return hr;
	}
	

	///////////////////////////////////////
	// We want a subtree search
	/////////////////////////////////////////
	ADS_SEARCHPREF_INFO prefInfo[1];
	prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
	prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
	prefInfo[0].vValue.Integer = ADS_SCOPE_SUBTREE;
	hr = pSearch->SetSearchPreference( prefInfo, 1);

	////////////////////////////////////
	// Prepared for attributed returned
	////////////////////////////////////
	LPWSTR pszAttr[] = { L"Name"};
	ADS_SEARCH_HANDLE hSearch;
	DWORD dwCount= sizeof(pszAttr)/sizeof(LPWSTR);


	//////////////////////////////////////////
	// Search for all groups in a domain
	/////////////////////////////////////////////
	hr = pSearch->ExecuteSearch(L"(objectCategory=Group)", pszAttr, dwCount, &hSearch );

	if ( !SUCCEEDED(hr) )
	{
		pSearch->Release();
		return hr;
	}

	//////////////////////////////////////////
	// Now enumerate the result
	/////////////////////////////////////////////
	ADS_SEARCH_COLUMN col;
    while(  pSearch->GetNextRow(hSearch)  != S_ADS_NOMORE_ROWS )
    {
		// Get 'Name' attribute
		hr = pSearch->GetColumn( hSearch, pszAttr[0], &col );
		if ( SUCCEEDED(hr) )
		{
			printf("%S\n", col.pADsValues->CaseIgnoreString);
			pSearch->FreeColumn( &col ); // You need to FreeColum after use.
		}

      
    }

	////////////////////
	// Clean-up
	////////////////////////
    pSearch->CloseSearchHandle(hSearch);
	pSearch->Release();



	CoUninitialize();
	return 0;
}
