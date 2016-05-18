#include <wchar.h>
#include <activeds.h>

HRESULT CheckDomainModeOfObject(IDirectoryObject *pDirObject, BOOL *bIsMixed);
WCHAR * GetDirectoryObjectAttrib(IDirectoryObject *pDirObject,LPWSTR pAttrName);
HRESULT GetDomainMode(IADs *pDomain, BOOL *bIsMixed);


int main(int argc, char* argv[])
{

wprintf(L"This program checks whether the current domain is in mixed or native mode.\n");

//Intialize COM
CoInitialize(NULL);
HRESULT hr = S_OK;
//Get rootDSE and the domain container's DN.
IADs *pObject = NULL;
VARIANT var;
BOOL bIsMixed;
LPOLESTR szPath = new OLECHAR[MAX_PATH];

if ( !szPath )
{
	wprintf(L"Alloc Failed");
    return FALSE;
}

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
   delete [] szPath;
   CoUninitialize();
   return TRUE;
}

hr = pObject->Get(L"defaultNamingContext",&var);
if (SUCCEEDED(hr))
{
	wcscpy_s(szPath,MAX_PATH,L"LDAP://"); //For NT 4.0 and Win 9.x, you must add the server name, e.g LDAP://myServer
	int len = wcslen(szPath);
	int dnLen = wcslen( var.bstrVal);
	if ( MAX_PATH <= len + dnLen ) // make sure we have enough buffer 
	{
		wprintf(L"The buffer is too small for the DN\n");
		pObject->Release();
		VariantClear(&var);
        delete [] szPath;	
        CoUninitialize();
		return FALSE;
	}


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
					 IID_IADs,
					 (void**)&pObject);
	if (SUCCEEDED(hr))
	{
		hr = GetDomainMode(pObject, &bIsMixed);
		if (SUCCEEDED(hr))
		{
		    hr = pObject->Get(L"name",&var);
			if (bIsMixed)
			  wprintf(L"Current domain %s is in mixed mode\n", var.bstrVal);
			else
			  wprintf(L"Current domain %s is in native mode\n", var.bstrVal);
		}
		else
			wprintf(L"GetDomainMode failed with hr: %x",hr);
	}
	else
		wprintf(L"Bind to domain failed with hr: %x",hr);
}
VariantClear(&var);
if (pObject)
  pObject->Release();
delete [] szPath;
CoUninitialize();
return TRUE;
}
		







HRESULT GetDomainMode(IADs *pDomain, BOOL *bIsMixed)
{
HRESULT hr = E_FAIL;
VARIANT var;
if (pDomain)
{
	VariantClear(&var);
	//Get the ntMixedDomain attribute
	LPOLESTR szAttribute = L"ntMixedDomain";
	hr = pDomain->Get(szAttribute,&var);
	if (SUCCEEDED(hr))
	{
		//Type should be VT_I4.
		if (var.vt==VT_I4)
		{
			//Zero means native mode.
			if (var.lVal == 0)
				*bIsMixed = FALSE;
			//One means mixed mode.
			else if (var.lVal == 1)
				*bIsMixed = TRUE;
			else
				hr=E_FAIL;
		}
	}
	VariantClear(&var);
}
return hr;

}	
