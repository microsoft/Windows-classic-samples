/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Abstract:
    This C++ file includes sample code that registers itself with the 
	Windows Firewall using the Microsoft Windows Firewall APIs but does
	not take ownership of any NET_FW_RULE_CATEGORY.
    The API to Register for NET_FW_RULE_CATEGORY_FIREWALL 
	needs the binary that is making this call to be linked
	with /integritycheck option to ensure code integrity. Failure 
	to do so can lead to error SEC_E_CANNOT_INSTALL at runtime
    For more details on code integrity read
	http://msdn2.microsoft.com/en-us/library/ms680339.aspx

--********************************************************************/

#include <windows.h>
#include <strsafe.h>
#include <netfw.h>
#include <conio.h>

#define BAIL_ON_ALLOC_FAILURE(ptr, fnName) \
   if ((ptr) == NULL) \
   { \
      result = ERROR_NOT_ENOUGH_MEMORY; \
      printf(#fnName " = ERROR_NOT_ENOUGH_MEMORY\n"); \
      goto CLEANUP; \
   }



// Forward declarations
DWORD ArrayOfLongsToVariant(
                            __in unsigned long numItems,
                            __in_ecount(numItems) const long* items,
                            __out VARIANT* dst
                            );
//
// Purpose: 
//   Entry point for the process
//
// Parameters:
//   None
// 
// Return value:
//   None
//
void __cdecl main() 
{ 
    DWORD result = NO_ERROR;
	HRESULT hr = S_OK;
    INetFwProduct* product = NULL;
    INetFwProducts* products = NULL;
    IUnknown* registration = NULL;
    BSTR displayName = NULL;
    VARIANT varCategories = { VT_EMPTY };
   
    long count=0;
    BOOL comInit =  FALSE;

    

	displayName = SysAllocString(L"@RegisterWithoutCategoryOwnership.exe,-127");
    BAIL_ON_ALLOC_FAILURE(displayName, SysAllocString);

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        //COM initialize failed
        wprintf(L"CoInitialize failed: 0x%08lx\n", hr);
        goto CLEANUP;
    }
    comInit = TRUE;


    hr = CoCreateInstance(__uuidof(NetFwProduct),NULL,CLSCTX_INPROC_SERVER,__uuidof(INetFwProduct),(void**)&product );

    if (FAILED(hr))
    {
        //CoCreateInstance Failed
        wprintf(L"CoCreateInstance for INetFwProduct failed: 0x%08lx\n", hr);
        goto CLEANUP;
    }

    hr = product->put_DisplayName(displayName);

    if (FAILED(hr))
    {
        //Put_displayName failed
        wprintf(L"put_DisplayName for INetFwProduct failed Error: 0x%08lx\n", hr);
        goto CLEANUP;
    }
    hr = product->put_RuleCategories(varCategories);

    if (FAILED(hr))
    {
        //Put_rulecategories failed
        wprintf(L"put_RuleCategories failed for INetFwProduct Error: 0x%08lx\n", hr);
        goto CLEANUP;
    }


    hr = CoCreateInstance(__uuidof(NetFwProducts),NULL,CLSCTX_INPROC_SERVER,__uuidof(INetFwProducts),(void**)&products );

    if (FAILED(hr))
    {
        //CoCreateInstance Failed
        wprintf(L"CoCreateInstance for INetFwProducts failed: 0x%08lx\n", hr);
        goto CLEANUP;
    }

    hr = products->Register(product, &registration);
    if (FAILED(hr))
    {
        //Failed to Register Products
        wprintf(L"Register failed: 0x%08lx\n", hr);
        goto CLEANUP;
    }			

    hr = products->get_Count( &count);
    if (FAILED(hr))
    {
        //Failed to get Count of Products
        wprintf(L"Get count failed: 0x%08lx\n", hr);
        goto CLEANUP;
    }		
    wprintf(L"INetFwProducts_Count returned %ld.\n", count);

    wprintf(L"Hit any key to unregister.\n");
    
	_getch();



CLEANUP:
	if (registration != NULL)
   {
      registration->Release();
   }
   if (products != NULL)
   {
      products->Release();
   }
   if (product != NULL)
   {
      product->Release();
   }
   if (comInit)
   {
      CoUninitialize();
   }
   
   SysFreeString(displayName);
   VariantClear(&varCategories);
   return;
} 




//This Function Converts and Array of Longs to Variant

DWORD ArrayOfLongsToVariant(
                            __in unsigned long numItems,
                            __in_ecount(numItems) const long* items,
                            __out VARIANT* dst
                            )
{
    DWORD result = NO_ERROR;
    SAFEARRAYBOUND bound[1];
    SAFEARRAY* sa = NULL;
    VARIANT* data;
    unsigned long i;

    VariantInit(dst);

    // If there are no items, just return VT_EMPTY.
    if (numItems == 0)
    {
        goto CLEANUP;
    }

    bound[0].lLbound = 0;
    bound[0].cElements = numItems;

    sa = SafeArrayCreate(VT_VARIANT, ARRAYSIZE(bound), bound);
    BAIL_ON_ALLOC_FAILURE(sa, SafeArrayCreate);

    data = (VARIANT*)(sa->pvData);

    for (i = 0; i < numItems; ++i)
    {
        V_VT(data + i) = VT_I4;
        V_I4(data + i) = items[i];
    }

    V_VT(dst) = VT_ARRAY | VT_VARIANT;
    V_ARRAY(dst) = sa;

CLEANUP:
    return result;
}


