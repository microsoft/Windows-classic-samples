//----------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       dump.cxx
//
//  Contents:   Functions for dumping the properties for an object.
//
//
//----------------------------------------------------------------------

#include "main.hxx"

//
// Given an ADsPath, bind to the object and call the DumpObject routine.
//

int
DoDump(char *AnsiADsPath)
{
 HRESULT hr = E_OUTOFMEMORY ;
 LPWSTR pszADsPath = NULL;
 IADs * pADs = NULL;

 //
 // Convert path to unicode and then bind to the object.
 //

 BAIL_ON_NULL(pszADsPath = AllocateUnicodeString(AnsiADsPath));

 hr = ADsGetObject(
             pszADsPath,
             IID_IADs,
             (void **)&pADs
             );

 if (FAILED(hr)) {

     wprintf(L"Failed to bind to object: %s\n", pszADsPath) ;
     BAIL_ON_FAILURE(hr, L"Failed to do ADsGetObject");
 }
 else {

     //
     // Dump the object
     //

     hr = DumpObject(pADs);

     if (FAILED(hr)) {

         wprintf(L"Unable to read properties of: %s\n", pszADsPath) ;
     }

     pADs->Release();
 }

error:

 FreeUnicodeString(pszADsPath);

 return (FAILED(hr) ? 1 : 0) ;
}

//
// Given an ADs pointer, dump the contents of the object
//

HRESULT
DumpObject(
 IADs * pADs
 )
{
 HRESULT hr;
HRESULT hrSA;
 VARIANT var;
ZeroMemory(&var,sizeof(var));
VARIANT *   pvarPropName = NULL;
 DWORD i = 0;
VARIANT varProperty;
 
 //
 // Access the schema for the object
 //

 hr = GetPropertyList(
             pADs,
             &var);
 BAIL_ON_FAILURE(hr, L"Failed to get property list");

 //
 // List the Properties
//
hr = SafeArrayAccessData(var.parray, (void **) &pvarPropName);
BAIL_ON_FAILURE(hr, L"Failed while calling SafeArrayAccessData");

for (i = 0; i < var.parray->rgsabound[0].cElements; i++){

   //
     // Get a property and print it out. The HRESULT is passed to
     // PrintProperty.
     //

     hr = pADs->Get(
             pvarPropName[i].bstrVal,
             &varProperty
             );
     if (FAILED(hr)) {
         wprintf(L"Error: 0x%x \t Failed to get the property: %s \n", hr, pvarPropName[i].bstrVal);
         continue;
     }
       
     PrintProperty(
         pvarPropName[i].bstrVal,
         hr,
         varProperty
         );

   VariantClear(&varProperty);

}

hr = SafeArrayUnaccessData(var.parray);
BAIL_ON_FAILURE(hr, L"Failed when calling SafeArrayUnaccessData");

error:
// Don't destroy hr in case we're here from BAIL_ON_FAILURE
if(var.parray) hrSA = SafeArrayDestroy(var.parray);

return(hr);
}


HRESULT
GetPropertyList(
 IADs * pADs,
 VARIANT * pvar )
{
 HRESULT hr= S_OK;
 BSTR bstrSchemaPath = NULL;
IADsClass * pADsClass = NULL;

 hr = pADs->get_Schema(&bstrSchemaPath);
 BAIL_ON_FAILURE(hr, L"Failed to get the schema path of the class");

 hr = ADsGetObject(
             bstrSchemaPath,
             IID_IADsClass,
             (void **)&pADsClass);
 BAIL_ON_FAILURE(hr, L"Failed to bind to schema path of the class");

//Put SafeArray of bstr's into input variant struct
hr = pADsClass->get_MandatoryProperties(pvar);
BAIL_ON_FAILURE(hr, L"Failed to get the mandatory properties of the class");

error:
 if (bstrSchemaPath) {
     SysFreeString(bstrSchemaPath);
 }

 if (pADsClass) {
     pADsClass->Release();
 }

 return(hr);
}

