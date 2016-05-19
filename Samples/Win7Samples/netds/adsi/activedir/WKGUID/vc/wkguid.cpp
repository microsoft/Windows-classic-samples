// wkguid.cpp : Defines the entry point for the console application.
//

#include <wchar.h>
#include <objbase.h>

//For ADSI
#include <activeds.h>

//Make sure you define UNICODE
//Need to define version 5 for Windows 2000
#define _WIN32_WINNT 0x0500
#include <ntdsapi.h>

HRESULT GetWKDomainObject(LPOLESTR szBindableWKGUID, //IN. Bindable string GUID of well-known object.
						  IADs **ppObject //OUT. Return a pointer to the specified well-known object.
						  );


void wmain( int argc, wchar_t *argv[ ])
{
wprintf(L"This program finds the User's container in the current Window 2000 domain\n");
wprintf(L"It uses the WKGUID binding string format.\n");
//Intialize COM
CoInitialize(NULL);
HRESULT hr = S_OK;
//Get rootDSE and the domain container's DN.
IADs *pObject = NULL;
hr = GetWKDomainObject(GUID_USERS_CONTAINER_W, //IN. Bindable string GUID of Users container.
						  &pObject //OUT. Return a pointer to the specified well-known object.
						  );
if (FAILED(hr))
{
   wprintf(L"Not Found. Could not bind to the User container.\n");
   if (pObject)
     pObject->Release();
   return;
}

BSTR bstr = NULL;
hr = pObject->get_ADsPath(&bstr);
if (SUCCEEDED(hr))
{
    wprintf (L"\nADsPath of Users Container (using WKGUID binding format):\n%s\n", bstr);
    FreeADsStr(bstr);
}

hr = pObject->get_Name(&bstr);
if (SUCCEEDED(hr))
{
    wprintf (L"\nName of Users Container (using WKGUID binding format):\n%s\n", bstr);
    FreeADsStr(bstr);
}

VARIANT var;
VariantInit(&var);
hr = pObject->Get(L"distinguishedName",&var);
if (SUCCEEDED(hr))
{
    wprintf (L"\nName of Users Container: %s\n", V_BSTR(&var));
}
VariantClear(&var);
    
if (pObject)
	pObject->Release();


//Uninitalize COM
CoUninitialize();
return;

}

// This function gets the specified well-known object for the current user's domain.
/* Note that the following well-known GUIDs are defined for domainDNS in ntdsapi.h:
#define GUID_USERS_CONTAINER_W              L"a9d1ca15768811d1aded00c04fd8d5cd"
#define GUID_COMPUTRS_CONTAINER_W           L"aa312825768811d1aded00c04fd8d5cd"
#define GUID_SYSTEMS_CONTAINER_W            L"ab1d30f3768811d1aded00c04fd8d5cd"
#define GUID_DOMAIN_CONTROLLERS_CONTAINER_W L"a361b2ffffd211d1aa4b00c04fd7d83a"
#define GUID_INFRASTRUCTURE_CONTAINER_W     L"2fbac1870ade11d297c400c04fd8d5cd"
#define GUID_DELETED_OBJECTS_CONTAINER_W    L"18e2ea80684f11d2b9aa00c04f79f805"
#define GUID_LOSTANDFOUND_CONTAINER_W       L"ab8153b7768811d1aded00c04fd8d5cd"
*/

HRESULT GetWKDomainObject(LPOLESTR szBindableWKGUID, //IN. Bindable string GUID of well-known object.
						  IADs **ppObject //OUT. Return a pointer to the specified well-known object.
						  )
{
HRESULT hr = E_FAIL;
//Get rootDSE and the domain container's DN.
IADs *pObject = NULL;
LPOLESTR szPath = new OLECHAR[MAX_PATH];
VARIANT var;
hr = ADsOpenObject(L"LDAP://rootDSE",
				 NULL,
				 NULL,
				 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
				 IID_IADs,
				 (void**)&pObject);

VariantInit(&var);
//Get current domain DN.
if (SUCCEEDED(hr))
{
	hr = pObject->Get(L"defaultNamingContext",&var);
	if (SUCCEEDED(hr))
	{
		//Build the WKGUID binding string.
		wcscpy_s(szPath,MAX_PATH,L"LDAP://");
		wcscat_s(szPath,MAX_PATH,L"<WKGUID=");
		wcscat_s(szPath,MAX_PATH,szBindableWKGUID);
		wcscat_s(szPath,MAX_PATH,L",");
		wcscat_s(szPath,MAX_PATH,V_BSTR(&var));
		wcscat_s(szPath,MAX_PATH,L">");
		//Print the binding string.
		//wprintf(L"WKGUID binding string: %s\n",szPath);
		VariantClear(&var);
		//Bind to the well-known object.
		hr = ADsOpenObject(szPath,
						 NULL,
						 NULL,
						 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
						 IID_IADs,
						 (void**)ppObject);
		if (FAILED(hr))
		{
			if (*ppObject)
			{
			  (*ppObject)->Release();
			  (*ppObject) = NULL;
			}
		}
	}
}

delete [] szPath;

if (pObject)
  pObject->Release();

return hr;
}
	

						  

