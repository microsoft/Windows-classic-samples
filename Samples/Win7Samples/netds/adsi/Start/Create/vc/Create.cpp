// Create.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "activeds.h"

HRESULT CreateUserIADsContainer(void)
{
HRESULT hr;
IADsContainer *pCont = NULL;
IDispatch *pDisp=NULL;
IADsUser *pIADsUser=NULL;

///////////////////////////////////////////////////
// Create a user using IADsContainer::Create
////////////////////////////////////////////////////
hr = ADsGetObject( L"WinNT://seyitb-dev", IID_IADsContainer, (void**) &pCont );
if (FAILED(hr) )
	return hr;


hr = pCont->Create(L"user", L"AliceW", &pDisp );
pCont->Release();

if(FAILED(hr) )
	return hr;

hr = pDisp->QueryInterface( IID_IADsUser, (void**) &pIADsUser );
if (FAILED(hr)) 
    return hr;


//do NOT hard code your password into the source code in production code
//take user input instead
pIADsUser->SetPassword(L"MysEcret1");
hr = pIADsUser->SetInfo(); // Commit
if(SUCCEEDED(hr))
    wprintf(L"User created successfully\n");

pIADsUser->Release();

return hr;
}

	
HRESULT CreateUserIDirectoryObject(void)
{
///////////////////////////////////////////////////////////
// Use IDirectoryObject to create an object
////////////////////////////////////////////////////////////
HRESULT hr;
IDispatch *pDisp=NULL;
IDirectoryObject *pDirObject=NULL;
ADSVALUE   sAMValue;
ADSVALUE   uPNValue;
ADSVALUE   classValue;

ADS_ATTR_INFO  attrInfo[] = 
{  
	{ L"objectClass",   ADS_ATTR_UPDATE, 
		                ADSTYPE_CASE_IGNORE_STRING, &classValue, 1 },
	{L"sAMAccountName", ADS_ATTR_UPDATE, 
                        ADSTYPE_CASE_IGNORE_STRING, &sAMValue, 1},
	{L"userPrincipalName", ADS_ATTR_UPDATE, 
                        ADSTYPE_CASE_IGNORE_STRING, &uPNValue, 1},
};
DWORD dwAttrs = sizeof(attrInfo)/sizeof(ADS_ATTR_INFO); 
 
classValue.dwType = ADSTYPE_CASE_IGNORE_STRING;
classValue.CaseIgnoreString = L"user";
 
sAMValue.dwType=ADSTYPE_CASE_IGNORE_STRING;
sAMValue.CaseIgnoreString = L"mikes";
 
uPNValue.dwType=ADSTYPE_CASE_IGNORE_STRING;
uPNValue.CaseIgnoreString = L"mikes@fabrikam.com";
 
// Create this user in an organizational unit
hr = ADsGetObject(L"LDAP://OU=MyOU,DC=fabrikam,DC=com",
			      IID_IDirectoryObject, (void**) &pDirObject );
 
if ( SUCCEEDED(hr) )
{
	hr = pDirObject->CreateDSObject( L"CN=Mike Smith",  attrInfo, 
									 dwAttrs, &pDisp );
	if ( SUCCEEDED(hr) )
	{
		pDisp->Release();
	}
    pDirObject->Release();
}

return hr;
}

int main(int argc, char* argv[])
{
HRESULT hr;
CoInitialize(NULL);

// Create a local user account
hr = CreateUserIADsContainer();
wprintf(L"Result: 0x%x\n", hr);

// Create a domain user account in Active Directory
// hr = CreateUserIDirectoryObject();
// wprintf(L"Result: 0x%x\n", hr);

CoUninitialize();
return 0;
}
