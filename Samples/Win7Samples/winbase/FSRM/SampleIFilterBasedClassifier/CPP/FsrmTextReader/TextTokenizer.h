// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// TextTokenizer.h : Declaration of the CTextTokenizer

#pragma once
#include "resource.h"       // main symbols

#include "FsrmTextReader.h"
#include <ntquery.h>
#include <filter.h>
#include <filterr.h>
#include <string>
#include <fsrmpipeline.h>
using namespace std;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



/*++

    class CTextTokenizer

Description:

    This is the ATL wizard added class for the ATL object.
	This class implements the ITextTokenizer interface methods for
	loading an IFilter based on the file's extension type and allowing
	for searching against the contents of the file.

--*/

class ATL_NO_VTABLE CTextTokenizer :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTextTokenizer, &CLSID_TextTokenizer>,
	public IDispatchImpl<ITextTokenizer, &IID_ITextTokenizer, &LIBID_FsrmTextReaderLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CTextTokenizer()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TEXTTOKENIZER)

DECLARE_NOT_AGGREGATABLE(CTextTokenizer)

BEGIN_COM_MAP(CTextTokenizer)
	COM_INTERFACE_ENTRY(ITextTokenizer)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease();	

public:

	// ITextTokenizer interface methods

	STDMETHOD(InitializeWithPropertyBag)(IUnknown * propertyBag);
	STDMETHOD(DoesContainWordsFromList)(SAFEARRAY* pWordList, VARIANT_BOOL* pBooleanResult);
	STDMETHOD(Cleanup)();

private:

	CComPtr<IFilter> m_pIFilter;
	CComQIPtr<IPersistStream> m_pIPersistStream;

private:

	// Called for each chunk to determine if it contains
	// any of the words passed in the array
	HRESULT
	DoesChunkContainWordsFromList(
		wstring *pszChunk,
		SAFEARRAY* pWordList, 
		VARIANT_BOOL* pBooleanResult
		);

	// Iterates of the IFilter chunks of the file
	// For each chunk, calls DoesChunkContainWordsFromList
	HRESULT
	ProcessChunks(
		SAFEARRAY* pWordList, 
		VARIANT_BOOL* pBooleanResult
		);
};

OBJECT_ENTRY_AUTO(__uuidof(TextTokenizer), CTextTokenizer)
