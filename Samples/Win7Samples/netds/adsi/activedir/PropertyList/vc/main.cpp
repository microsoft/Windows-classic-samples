

// ADsPropertyList Main.cpp
// Active Directory Sample
/*
	Demonstrates use of the following 
	ADSI COM interfaces from C++:

		IADs
		IADsPropertyList
		IADsPropertyEntry
		IADsPropertyValue

	ADSI Functions:
		ADsOpenObject()


    This sample binds to a server using RootDSE, then IADsPropertyList to list all 
    the properties on this object

*/


//


#define INC_OLE2
#define UNICODE 1
#define _WIN32_DCOM

#include <windows.h>
#include <winuser.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <winldap.h>
#include <adsiid.h>
#include <string.h>
#include <activeds.h>

#include "ADSIhelpers.h"


int IS_BUFFER_ENOUGH(UINT maxAlloc, LPWSTR pszTarget, LPCWSTR pszSource, int toCopy=-1);
void main()
{
    // COM result variable
	HRESULT hr;

	// Interface Pointers
	IDirectorySearch    *   pDSSearch=NULL;
	IADs	            *   pIADs = NULL;
	IADsPropertyList    *   pIADpl = NULL;
	VARIANT                 vRet;
	LONG                    lCount;  
    IADs                *   pIADsrootDSE = NULL; 
    BSTR                    bsNamingContext;
    VARIANT                 vResult;
    WCHAR                   wszActualLDAPPath[MAX_PATH];

	VariantInit(&vRet);
	
	// Initialize COM
	CoInitialize(0);
	
	// Bind Through RootDSE
    _putws(L"Binding to a server using LDAP://rootDSE");

    hr = ADsGetObject(  L"LDAP://rootDSE", IID_IADs,(void **)&pIADsrootDSE);

	if (FAILED(hr))
	{
		CoUninitialize();
		return;
	}

     // Get the defaultNamingContext attribute and for binding
    // to the actual DC
    bsNamingContext = SysAllocString(L"defaultNamingContext");
    VariantInit(&vResult);

    // Get the defaultNamingContext Attribute
    hr = pIADsrootDSE->Get(bsNamingContext,&vResult);

    if (SUCCEEDED(hr))
    {
            // Make sure it's a BSTR
            hr = VariantChangeType(&vResult,&vResult,NULL,VT_BSTR);

            if (SUCCEEDED(hr))
            {
                // Build a binding string
				wcscpy_s(wszActualLDAPPath, sizeof(wszActualLDAPPath)/sizeof(wszActualLDAPPath[0]), L"LDAP://");
				if ( IS_BUFFER_ENOUGH(sizeof(wszActualLDAPPath)/sizeof(wszActualLDAPPath[0]),wszActualLDAPPath, vResult.bstrVal) > 0 )
				{
                   wcscat_s(wszActualLDAPPath,sizeof(wszActualLDAPPath)/sizeof(wszActualLDAPPath[0]),vResult.bstrVal );
				   wprintf(L"\nBinding to the path %s\n",wszActualLDAPPath);
				}
				else
				{
					pIADsrootDSE->Release();
					wprintf(L"Buffer is too small to hold the domain DN");
				}

                
                
                // Bind to the actual server
                hr = ADsGetObject(  wszActualLDAPPath, IID_IADs,(void **)&pIADs);
            }
            VariantClear(&vResult);
    }  
    SysFreeString(bsNamingContext);
    
    // Relase the RootDse binding...
    pIADsrootDSE->Release();
    pIADsrootDSE = NULL;

	if (FAILED(hr))
	{
		CoUninitialize();
		return;
	}
	
    wprintf(L"\n\nSuccessfully bound to %s\n",wszActualLDAPPath);
	
    // Print some general info about the object we are bound to
    PrintIADSObject(pIADs);
	
	if (FAILED(hr))
	{
		CoUninitialize();
		return;
	}

    wprintf(L"\n\nEnumerating this object's properties using the IADsPropertyList interface\n\n");
		
	//Call GetInfo to load all the properties. Call GetInfoEx to load only specific ones.
	hr = pIADs->GetInfo();

	if (FAILED(hr))
	{
		CoUninitialize();
		return;
	}
	
	//QueryInterface() for IADsPropertyList ptr.			
	hr = pIADs->QueryInterface( IID_IADsPropertyList,(void **)&pIADpl);

	if (FAILED(hr))
	{
		CoUninitialize();
		return;
	}

	// Retrieve a COUNT of all the properties in the propertylist
	hr = pIADpl->get_PropertyCount(  &lCount  );

	// Output the property count obtained from the IADsPropertyList interface
	printf("\n The Object has %d properties\n",lCount);
	
	// Move to the FIRST property in the list
	hr = pIADpl->Next(&vRet);
	CheckHRESULT(hr,"pIADpl->Next(&vRet);");

 	for (long lElement = 0; lElement < lCount; lElement++)
	{
		
		IDispatch * pDisp = V_DISPATCH(&vRet);
		IADsPropertyEntry * pAdsEntry = NULL;
		
		// QueryInterface for a IADsPropertyEntry interace on the current property
		hr = pDisp->QueryInterface(IID_IADsPropertyEntry,(void**) &pAdsEntry);

		if (SUCCEEDED(hr))
		{
			BSTR  bsName ;
			VARIANT vValue;
			VariantInit(&vValue);

			// Get the NAME of the current property
			hr = pAdsEntry->get_Name( &bsName);
			
			if (SUCCEEDED(hr))
			{
				wprintf(L"\n NAME:%s \t\n",(LPOLESTR)bsName);
				SysFreeString(bsName);
			}

			// Get the values
            hr = pAdsEntry->get_Values(&vValue);

			puts("The Values Variant style is :");
			puts(GetVariantStyle(vValue.vt));

			if (SUCCEEDED(hr))
			{
				// We should now have a VARAIANT ARRAY of IDispath *'s
                // So, read the safe array, enumeraing each IDispatch * 
                // The IDispatch * is then QI'd for the IADsPropertyValue 
                // interface. This interface provides the property information
				if (HAS_BIT_STYLE(vValue.vt,VT_ARRAY))
				{
					LONG dwSLBound = 0;
					LONG dwSUBound = 0;
					LONG i;

					hr = SafeArrayGetLBound(V_ARRAY(&vValue),1,(long FAR *)&dwSLBound);

					hr = SafeArrayGetUBound(V_ARRAY(&vValue),1,(long FAR *)&dwSUBound);

					for (i = dwSLBound; i <= dwSUBound; i++) 
					{
						VARIANT v;
						VariantInit(&v);
						hr = SafeArrayGetElement(V_ARRAY(&vValue),(long FAR *)&i,&v);
						
						if (SUCCEEDED(hr))
						{
							if (HAS_BIT_STYLE(v.vt,VT_DISPATCH))
							{
								IDispatch * pDispEntry = V_DISPATCH(&v);
								IADsPropertyValue * pAdsPV = NULL;
								
								hr = pDispEntry->QueryInterface(IID_IADsPropertyValue,(void **) &pAdsPV);

								if (SUCCEEDED(hr))
								{	
									BSTR bValue;

									// Get the value as a BSTR
                                    hr = GetIADsPropertyValueAsBSTR(&bValue,pAdsEntry,pAdsPV);

									if (SUCCEEDED(hr))
									{
										wprintf(L"<%s>\n",(LPOLESTR)bValue);
										SysFreeString(bValue);
									}
									pAdsPV->Release();
									pAdsPV =NULL;
								}
							}
							else
								puts("!!NO DISPATCH ENTRY!!");
						
							VariantClear(&v);
						}
					}
					VariantClear(&vValue);
				}
				else
					wprintf(L" NAME:%s \t",(LPOLESTR)V_BSTR(&vValue));

			}
			pAdsEntry->Release();
			pAdsEntry = NULL;
		}
		hr = pIADpl->Next(&vRet);
	}

	CoUninitialize();

}

int IS_BUFFER_ENOUGH(UINT maxAlloc, LPWSTR pszTarget, LPCWSTR pszSource, int toCopy)

{	     
         if (toCopy == -1)

          {
               toCopy = wcslen(pszSource);
          }

		  return maxAlloc - (wcslen(pszTarget) + toCopy + 1); 
}

