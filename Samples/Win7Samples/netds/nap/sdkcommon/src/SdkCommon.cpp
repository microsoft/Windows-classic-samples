// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#include "SdkCommon.h"
#include <ObjBase.h>
#include "DebugHelper.h"
#include <Strsafe.h>
#include <NapUtil.h>


namespace SDK_SAMPLE_COMMON
{

//
// Declarations of private methods (not accessible by external code).
//

// Create a basic SoH Constructor object.
HRESULT CreateSoHConstructor(INapSoHConstructor* &pISohConstructor) throw();


// Create a basic SoH Processor object.
HRESULT CreateSoHProcessor(INapSoHProcessor* &pISohProcessor) throw();



//
// Implementation of public methods.
//



// Helper Function for populating already allocated CountedString structures
// This makes filling the NapRegistrationInfo struct cleaner in the sample code
HRESULT FillCountedString(const WCHAR* src, CountedString* dest) throw ()
{
    HRESULT hr = S_OK;
    CountedString *tCS = NULL;

    if ( (NULL == src) )
    {
        DebugPrintfW(L" --- SdkCommon - FillCountedString(): bad src pointer");
        hr = E_POINTER;
        goto Cleanup;
    }

    if ( (NULL != dest->string) ||
            (0 != dest->length) )
    {
        DebugPrintfW(L" --- SdkCommon - FillCountedString(): dest already has values set/allocated");
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hr = AllocCountedString( &tCS, src );

    if ( FAILED(hr) )
    {
        // Failed to allocate memory
        DebugPrintfW(L" --- SdkCommon - FillCountedString: Failed to allocate buffer memory (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    *dest = *tCS;

    // free the struct, but not the embedded string buffer, as we are passing out a reference to it
    CoTaskMemFree(tCS);

Cleanup:
    return hr;
}


// Helper Function for depopulating CountedString structures
// leaves the structure intact, but frees the buffer underneath
// for use with FillCountedString above
HRESULT EmptyCountedString( CountedString * cs ) throw ()
{
    HRESULT hr = S_OK;

    if ( NULL == cs)
    {
        DebugPrintfW(L" --- SdkCommon - EmptyCountedString(): bad pointer");
        hr = E_POINTER;
        goto Cleanup;
    }

    CoTaskMemFree(cs->string);
    cs->string = NULL;
    cs->length = 0;

Cleanup:
    return hr;

}


// Create an SoH Constructor object.
HRESULT CreateOutputSoHConstructor(INapSoHConstructor* &pISohConstructor,
                                   SystemHealthEntityId  systemHealthId,
                                   BOOL                  sohType) throw()
{
    HRESULT hr = S_OK;

    //
    // Create an empty SoH Constructor for this client.
    //

    hr = CreateSoHConstructor(pISohConstructor);
    if ( FAILED(hr) )
    {
        DebugPrintfW(L" --- SdkCommon - CreateOutputSoHConstructor(): failed on call to CreateSoHConstructor (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    // Initialize the SoH Constructor, as specified by the caller.
    hr = pISohConstructor->Initialize(systemHealthId, sohType);
    if ( FAILED(hr) )
    {
        DebugPrintfW(L" --- SdkCommon - CreateOutputSoHConstructor(): failed on call to Initialize (error = 0x%08x)" ,hr);
        ReleaseObject(pISohConstructor);
    }

 Cleanup:
    return hr;
}


// Create an SoH Processor object.
HRESULT CreateInputSoHProcessor(INapSoHProcessor*   &pISohProcessor,
                                SystemHealthEntityId &systemHealthId,
                                BOOL                  sohType,
                                SoH                  *pInputSoh) throw()
{
    HRESULT hr = S_OK;

    //
    // Create the SoH processor that will be used for this input SoH.
    //

    hr = CreateSoHProcessor(pISohProcessor);
    if ( FAILED(hr) )
    {
        DebugPrintfW(L" --- SdkCommon - CreateInputSoHProcessor(): failed on call to CreateSoHProcessor (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    // Initialize the SoH Processor, as specified by the caller.
    hr = pISohProcessor->Initialize(pInputSoh, sohType, &systemHealthId);
    if ( FAILED(hr) )
    {
        DebugPrintfW(L" --- SdkCommon - CreateInputSoHProcessor(): failed on call to Initialize (error = 0x%08x)" ,hr);
        ReleaseObject(pISohProcessor);
    }

 Cleanup:
    return hr;
}


// Release a reference (pointer) to an IUnknown object.
void ReleaseObject(IUnknown *pIUnknown) throw()
{
    if (pIUnknown)
    {
        pIUnknown->Release();
        pIUnknown = NULL;
    }
}


// Free a WCHAR string buffer. Upon exit, the input variable will be set to
// NULL.
void FreeMemory(WCHAR* &pAllocatedMemory) throw()
{	
	if (pAllocatedMemory)
	{
		CoTaskMemFree(pAllocatedMemory);
		pAllocatedMemory = NULL;
	}
}


// Free a CountedString struct, including the buffer pointed to by the
// internal string member. Upon exit, the input variable will be set to NULL.
void FreeMemory(CountedString* &pAllocatedMemory) throw()
{
	if (pAllocatedMemory)
	{
		FreeMemory(pAllocatedMemory->string);
		CoTaskMemFree(pAllocatedMemory);
		pAllocatedMemory = NULL;
	}
}


// Allocate a buffer to contain a WCHAR string, which is "stringSizeInBytes" wchars long.
HRESULT AllocateMemory(OUT WCHAR* &pString, IN size_t stringSizeInBytes) throw()
{
	HRESULT hr = S_OK;

	pString = (WCHAR*) CoTaskMemAlloc(stringSizeInBytes);
	if (!(pString))
	{
		// Failed to allocate memory
		hr = E_OUTOFMEMORY;
		goto Cleanup;
	}

	ZeroMemory(pString, stringSizeInBytes);

Cleanup:
	return hr;
}


// Allocate a buffer to contain a CountedString struct, whose overall string member's
// buffer is "stringSizeInBytes" wchars long.
HRESULT AllocateMemory(OUT CountedString* &pString, IN size_t stringSizeInBytes) throw()
{
	HRESULT hr = S_OK;

	pString = (CountedString*) CoTaskMemAlloc(sizeof(CountedString));
	if (!pString)
	{
		// Failed to allocate memory
		hr = E_OUTOFMEMORY;
		goto Cleanup;
	}

	ZeroMemory(pString, sizeof(CountedString));

	hr = AllocateMemory(pString->string, stringSizeInBytes);
	if (FAILED(hr))
	{
		// delete the allocated memory
		FreeMemory(pString);
		goto Cleanup;
	}

	ZeroMemory(pString->string, stringSizeInBytes);

Cleanup:
	return hr;
}

//
// Implementation of private methods.
//

// Create a basic SoH Constructor object.
HRESULT CreateSoHConstructor(INapSoHConstructor* &pISohConstructor) throw()
{
    HRESULT hr = S_OK;

    //
    // Construct a basic SoH Constructor object.
    //

    hr = CoCreateInstance( CLSID_NapSoHConstructor,
                           0,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(INapSoHConstructor),
                           reinterpret_cast<void**>(&pISohConstructor) );

    if (FAILED(hr))
    {
        DebugPrintfW(L" --- SdkCommon - CreateSoHConstructor(): failed on call to CoCreateInstance (error = 0x%08x)" ,hr);
        goto Cleanup;
    }


    if (! pISohConstructor)
    {
        DebugPrintfW(L" --- SdkCommon - CreateSoHConstructor(): CoCreate success, but pointer is bad.");
        hr = E_POINTER;
        goto Cleanup;
    }

 Cleanup:
    return hr;
}


// Create a basic SoH Processor object.
HRESULT CreateSoHProcessor(INapSoHProcessor* &pISohProcessor) throw()
{
    HRESULT hr = S_OK;

    //
    // Construct a basic SoH object (QuarSoH interface).
    //

    hr = CoCreateInstance( CLSID_NapSoHProcessor,
                           0,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(INapSoHProcessor),
                           reinterpret_cast<void**>(&pISohProcessor) );

    if (FAILED(hr))
    {
        DebugPrintfW(L" --- SdkCommon - CreateSoHProcessor(): failed on call to CoCreateInstance (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    if (! pISohProcessor)
    {
        DebugPrintfW(L" --- SdkCommon - CreateSoHProcessor(): CoCreate success, but pointer is bad.");
        hr = E_POINTER;
        goto Cleanup;
    }

 Cleanup:
    return hr;
}


}  // End "namespace SDK_SAMPLE_COMMON".
