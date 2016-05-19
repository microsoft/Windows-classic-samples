// **************************************************************************

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  methcli.cpp 
//
// Description:
//        WMI Method Client Sample.
//		  This sample shows how to call a method on an object in WMI
//
// History:
//
// **************************************************************************

#include <objbase.h>
#include <windows.h>                                     
#include <stdio.h>
#include <wbemidl.h> 

//***************************************************************************
//
// main
//
// Purpose: Initialized Ole, call the method, and cleanup.
//
//***************************************************************************

BOOL g_bInProc = FALSE;
 
int main(int iArgCnt, char ** argv)
{
    IWbemLocator *pLocator = NULL;
    IWbemServices *pNamespace = 0;
    IWbemClassObject * pClass = NULL;
    IWbemClassObject * pOutInst = NULL;
    IWbemClassObject * pInClass = NULL;
    IWbemClassObject * pInInst = NULL;
	BSTR Text = NULL;
	HRESULT hr = S_OK;
  
    BSTR path = SysAllocString(L"root\\default");
    BSTR ClassPath = SysAllocString(L"MethProvSamp");
    BSTR MethodName = SysAllocString(L"Echo");
    BSTR ArgName = SysAllocString(L"sInArg");

	if (!path || ! ClassPath || !MethodName || ! ArgName)
	{
		printf("SysAllocString failed. Out of memory.\n");
		goto cleanup;
	}
  

    // Initialize COM and connect up to CIMOM

    hr = CoInitialize(0);
	if (FAILED(hr))
	{
	    printf("CoInitialize returned 0x%x:", hr);
		goto cleanup;
	}

	//  NOTE:
	//  When using asynchronous WMI API's remotely in an environment where the "Local System" account 
	//  has no network identity (such as non-Kerberos domains), the authentication level of 
	//  RPC_C_AUTHN_LEVEL_NONE is needed. However, lowering the authentication level to 
	//  RPC_C_AUTHN_LEVEL_NONE makes your application less secure. It is wise to
	//	use semi-synchronous API's for accessing WMI data and events instead of the asynchronous ones.

    hr = CoInitializeSecurity	( NULL, -1, NULL, NULL,
					RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
					RPC_C_IMP_LEVEL_IMPERSONATE, 
					NULL, 
					EOAC_SECURE_REFS, //change to EOAC_NONE if you change dwAuthnLevel to RPC_C_AUTHN_LEVEL_NONE
					NULL );
	if (FAILED(hr))
	{
	    printf("CoInitializeSecurity returned 0x%x:", hr);
		goto cleanup;
	}

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID *) &pLocator);
	if (FAILED(hr))
	{
	    printf("CoCreateInstance returned 0x%x:", hr);
		goto cleanup;
	}
    hr = pLocator->ConnectServer(path, NULL, NULL, NULL, 0, NULL, NULL, &pNamespace);
    printf("\n\nConnectServer returned 0x%x:", hr);
    if(hr != WBEM_S_NO_ERROR)
		goto cleanup;

    // Get the class object

    hr = pNamespace->GetObject(ClassPath, 0, NULL, &pClass, NULL);
    printf("\nGetObject returned 0x%x:", hr);
    if(hr != WBEM_S_NO_ERROR)
		goto cleanup;

    // Get the input argument and set the property

    hr = pClass->GetMethod(MethodName, 0, &pInClass, NULL); 
    printf("\nGetMethod returned 0x%x:", hr);
    if(hr != WBEM_S_NO_ERROR)
		goto cleanup;

    hr = pInClass->SpawnInstance(0, &pInInst);
    printf("\nSpawnInstance returned 0x%x:", hr);
    if(hr != WBEM_S_NO_ERROR)
		goto cleanup;

    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal= SysAllocString(L"hello");
    if (var.bstrVal == NULL)
		goto cleanup;
    hr = pInInst->Put(ArgName, 0, &var, 0);
    VariantClear(&var);

    // Call the method

    hr = pNamespace->ExecMethod(ClassPath, MethodName, 0, NULL, pInInst, &pOutInst, NULL);
    printf("\nExecMethod returned 0x%x:", hr);
    if(hr != WBEM_S_NO_ERROR)
		goto cleanup;

    
    // Display the results.

    hr = pOutInst->GetObjectText(0, &Text);
	if(hr != WBEM_S_NO_ERROR)
		goto cleanup;
    printf("\n\nThe object text of the output object is:\n%S", Text);
    
	printf("Terminating normally\n");


	// Free up resources
cleanup:

    SysFreeString(path);
    SysFreeString(ClassPath);
    SysFreeString(MethodName);
    SysFreeString(ArgName);
    SysFreeString(Text);

	if (pClass)
		pClass->Release();
	if (pInInst)
		pInInst->Release();
	if (pInClass)
		pInClass->Release();
	if (pOutInst)
		pOutInst->Release();
	if (pLocator)
		pLocator->Release();
	if (pNamespace)
		pNamespace->Release();
    CoUninitialize();
    return 0;
}

