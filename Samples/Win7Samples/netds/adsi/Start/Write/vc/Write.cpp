// Write.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"


int main(int argc, char* argv[])
{
	HRESULT hr;
	IADs *pADs=NULL;
	LPWSTR pszADsPath = L"LDAP://CN=Jane Johnson,OU=testOU,DC=testDom1,DC=testDom2,DC=microsoft,DC=com";

	CoInitialize(NULL);

	///////////////////////////////////
	// Modifying attributes via IADs
	////////////////////////////////////
	hr = ADsGetObject(pszADsPath,
		              IID_IADs, 
			   	      (void**) &pADs );

	if (!SUCCEEDED(hr) )
	{
		return hr;
	}

	VARIANT var;
	
	
	// we omit checking result for brevity..

	// First Name
	VariantInit(&var);
	V_BSTR(&var) = SysAllocString(L"Janet");
	V_VT(&var) = VT_BSTR;
	hr = pADs->Put( L"givenName", var );

	// Last Name
	VariantClear(&var);
	V_BSTR(&var) = SysAllocString(L"Johns");
	V_VT(&var) = VT_BSTR;
	hr = pADs->Put( L"sn", var ); 
	VariantClear(&var);


	// Other Telephones
	LPWSTR pszPhones[] = { L"425 844 1234", L"425 924 4321" };
	DWORD dwNumber = sizeof( pszPhones ) /sizeof(LPWSTR);

	hr = ADsBuildVarArrayStr( pszPhones, dwNumber, &var );
	hr = pADs->Put( L"otherTelephone", var ); 
	VariantClear(&var);

	hr = pADs->SetInfo();
	pADs->Release();

	if (!SUCCEEDED(hr) )
	{
		
		return hr;
	}



	/////////////////////////////////////////////////
	// Alternatively, you can use IDirectoryObject
	//////////////////////////////////////////////////
	IDirectoryObject *pDir=NULL;
	hr = ADsGetObject(pszADsPath,
		              IID_IDirectoryObject, 
			   	      (void**) &pDir );

	if ( !SUCCEEDED(hr) )
	{
		return hr;
	}


	
	DWORD  dwReturn;
	ADSVALUE  snValue;
	ADSVALUE  fNameValue;
	ADSVALUE  phoneValue[2];

	ADS_ATTR_INFO  attrInfo[] = {   
		{L"givenName",ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING,&snValue,1},
		{L"sn", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING,&fNameValue,1 },
		{L"otherTelephone", ADS_ATTR_UPDATE, ADSTYPE_CASE_IGNORE_STRING, phoneValue,2 }
								};
	DWORD dwAttrs = sizeof(attrInfo)/sizeof(ADS_ATTR_INFO); 

	///// First Name ///////////
	fNameValue.dwType=ADSTYPE_CASE_IGNORE_STRING;
	fNameValue.CaseIgnoreString = L"Janet";
	
 
	///// Last Name ///////////
	snValue.dwType= ADSTYPE_CASE_IGNORE_STRING;
	snValue.CaseIgnoreString = L"Johns";
	
	///// Other Telephone ///////////
	phoneValue[0].dwType = ADSTYPE_CASE_IGNORE_STRING;
	phoneValue[0].CaseIgnoreString = L"425 844 1234";

	phoneValue[1].dwType = ADSTYPE_CASE_IGNORE_STRING;
	phoneValue[1].CaseIgnoreString = L"425 924 4321";

	hr = pDir->SetObjectAttributes(attrInfo, dwAttrs, &dwReturn);

	pDir->Release();

	if ( !SUCCEEDED(hr) )
	{
		return hr;
	}



	CoUninitialize();
	return 0;
}
