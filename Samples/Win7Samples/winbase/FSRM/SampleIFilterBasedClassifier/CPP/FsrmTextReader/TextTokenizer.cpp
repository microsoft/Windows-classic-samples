// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// TextTokenizer.cpp : Implementation of CTextTokenizer

#include "stdafx.h"
#include "TextTokenizer.h"
#include <propvarutil.h>

// Define a buffer size for stream reads
#define ReadBufferSize 2048

/*++

    Routine CTextTokenizer::InitializeWithPropertyBag

Description:

    Load the appropriate IFilter based on the filename in the property bag
	Obtain the IStream interface from the property bag for the file

Arguments:

	propertyBag - An object of IFsrmPropertyBag

Return value:

    HRESULT

Notes:

	Obtains file name from property bag. Using the full path of the file
	initializes the IFilter based on the file extention.
	Obtains the stream on the file.

--*/

STDMETHODIMP CTextTokenizer::InitializeWithPropertyBag(IUnknown * propertyBag)
{    
	HRESULT hr = S_OK;		
	CComBSTR strFileName;
	CComBSTR strVolumeName;
	CComBSTR strRelativePath;
	WCHAR szFullPath[MAX_PATH];
	CComPtr<IStream> spStream;
	CComVariant varStream;
	CComQIPtr<IStream> pStream;		
	CComPtr<IFsrmPropertyBag> pIFsrmPropertyBag;		

	Cleanup();

	pIFsrmPropertyBag = (IFsrmPropertyBag *)propertyBag;
	
	hr = pIFsrmPropertyBag->get_Name(&strFileName);
	if (FAILED(hr))
	{
		goto exit;
	}

	hr = pIFsrmPropertyBag->get_VolumeName(&strVolumeName);
	if (FAILED(hr))
	{
		goto exit;
	}

	hr = pIFsrmPropertyBag->get_RelativePath(&strRelativePath);
	if (FAILED(hr))
	{
		goto exit;
	}

	::PathCombine(szFullPath, strVolumeName, strRelativePath);
	::PathCombine(szFullPath, szFullPath, strFileName);

	#pragma warning(disable: 6387) // arg 2 can be NULL
	#pragma warning(disable: 6309) // arg 2 can be 0
	hr = LoadIFilter(szFullPath, NULL, (void **)&m_pIFilter);	
	#pragma warning(error: 6309) 
	#pragma warning(error: 6387)
	if (FAILED(hr))
	{
		goto exit;
	}


	hr = pIFsrmPropertyBag->GetFileStreamInterface(
		FsrmFileStreamingMode_Read,
		FsrmFileStreamingInterfaceType_IStream,
		&varStream);
	
	if (hr != S_OK) {
		goto exit;
	}

	if (!(varStream.vt == VT_UNKNOWN && varStream.punkVal != NULL)) {
		hr = E_FAIL;
		goto exit;
	}
	pStream = varStream.punkVal;
	if (pStream == NULL) {
		hr = E_FAIL;
		goto exit;
	}

	hr = m_pIFilter->QueryInterface(&m_pIPersistStream);
	if (FAILED(hr))
	{
		goto exit;
	}

	hr = m_pIPersistStream->Load(pStream);
	hr = (hr == S_FALSE) ? E_FAIL : hr;
    if (FAILED(hr))
    {
		goto exit;
    }	

exit:
    return hr;
}

/*++

    Routine CTextTokenizer::DoesContainWordsFromList

Description:

    Called by the classifier to determine if the file's IFilter 
	has found any of the passed in words in the contents

Arguments:

	pWordList		- Array of words to search in the chunks
	pBooleanResult  - Boolean result of the search

Return value:

    HRESULT

Notes:

	Iterates over the IFilter chunks and looks for each of the
	words in the passed in word list.

--*/

STDMETHODIMP CTextTokenizer::DoesContainWordsFromList(SAFEARRAY* pWordList, VARIANT_BOOL* pBooleanResult)
{
    HRESULT hr = S_OK;
	ULONG filterFlags;

	hr = m_pIFilter->Init(IFILTER_INIT_CANON_PARAGRAPHS |
			IFILTER_INIT_HARD_LINE_BREAKS |
			IFILTER_INIT_CANON_HYPHENS |
			IFILTER_INIT_CANON_SPACES |
			IFILTER_INIT_INDEXING_ONLY |
			IFILTER_INIT_APPLY_INDEX_ATTRIBUTES,
			0, NULL, &filterFlags);

	if (FAILED(hr))
    {
		goto exit;
    }

	hr = ProcessChunks(pWordList, pBooleanResult);	

exit:

    return hr;
}


void 
CTextTokenizer::FinalRelease()
{
}

/*++

    Routine CTextTokenizer::DoesContainWordsFromList

Description:

    Private method called for each chunk to search against the word list

Arguments:

	pszChunk		- Current chunk from the IFilter
	pWordList		- Wordlist to search against
	pBooleanResult  - Result

Return value:

    HRESULT

Notes:

	Returns true the chunk contains any of the words

--*/

HRESULT
CTextTokenizer::DoesChunkContainWordsFromList(
	wstring *pszChunk,
	SAFEARRAY* pWordList, 
	VARIANT_BOOL* pBooleanResult
	)
{
    HRESULT hr = S_OK;
	LONG lBound = 0;
	LONG uBound = 0;
	_variant_t vWord;
	size_t pos = 0;	
	VARIANT_BOOL foundOne = VARIANT_FALSE;	

	if (pWordList)
	{

		hr = SafeArrayGetLBound(pWordList, 1, &lBound);
		if (FAILED(hr))
		{
			goto exit;
		}

		hr = SafeArrayGetUBound(pWordList, 1, &uBound);
		if (FAILED(hr))
		{
			goto exit;
		}

		for (long i = lBound; i <= uBound; i++){

			hr = SafeArrayGetElement(pWordList, &i, &vWord);
			if (FAILED(hr))
			{
				continue;
			}
			if (SysStringLen(vWord.bstrVal)<=0 || vWord.bstrVal[0] == L'\0')
			{
				continue;
			}
			pos = pszChunk->find(vWord.bstrVal);
			
			if (std::wstring::npos != pos)
			{
				foundOne = VARIANT_TRUE;
				break;
			}
		}

		*pBooleanResult = foundOne;
	}

exit:

    return hr;
}

/*++

    Routine CTextTokenizer::ProcessChunks

Description:

    Private method to iterate over the IFilter

Arguments:

	pWordList		- Wordlist to search against
	pBooleanResult  - Result

Return value:

    HRESULT

Notes:

	Iterates over the IFilter chunks and searchs for the wordlist

--*/

HRESULT
CTextTokenizer::ProcessChunks(
	SAFEARRAY* pWordList, 
	VARIANT_BOOL* pBooleanResult
	)
{
	STAT_CHUNK statChunk = {0};
	HRESULT hr = S_OK;
	wstring szChunk;

	while(TRUE) 
	{
		hr = m_pIFilter->GetChunk(&statChunk);

		if (hr == FILTER_E_EMBEDDING_UNAVAILABLE || hr == FILTER_E_LINK_UNAVAILABLE)
		{
			// Encountered an embed/link for which filter is not available.
			// Continue with other chunks
			continue;
		}
		else if (hr == FILTER_E_END_OF_CHUNKS)
		{
			// Done
			hr = S_OK;
			break;
		}
		else if (FAILED(hr))
		{
			// IFilter::GetChunk failed
			// Critical failure
			break;
		}
		// Else continue with the chunk's content
		szChunk.empty();

		while (TRUE)
		{
			if (CHUNK_TEXT == statChunk.flags)
			{
				WCHAR szBuffer[2048];
				ULONG ccBuffer = ARRAYSIZE(szBuffer) - 1;
				hr = m_pIFilter->GetText(&ccBuffer, szBuffer);
				if (hr == FILTER_E_NO_TEXT)
				{
					// Current text chunk contains no text
					break;
				}
				else if (hr == FILTER_E_NO_MORE_TEXT)
				{
					// Done
					hr = S_OK;

					hr = DoesChunkContainWordsFromList(&szChunk, pWordList, pBooleanResult);
					if (FAILED(hr) || *pBooleanResult == VARIANT_TRUE )
					{
						goto exit;
					}
					break;
				}
				else if (FAILED(hr))
				{
					// IFilter::GetText failed 
					break;
				}
				else
				{
					if (hr == FILTER_S_LAST_TEXT)
                    {
                        // Next one will return FILTER_E_NO_MORE_TEXT
                        hr = S_OK;
                    }

					szBuffer[ccBuffer] = L'\0';					
					szChunk.append(szBuffer, ccBuffer);
				}
			}
			else if (CHUNK_VALUE == statChunk.flags)
			{
				PROPVARIANT *pPropValue;
				hr = m_pIFilter->GetValue(&pPropValue);
				if (hr == FILTER_E_NO_MORE_VALUES)
				{
					// Last time returned the last value.

					hr = S_OK;

					hr = DoesChunkContainWordsFromList(&szChunk, pWordList, pBooleanResult);
					if (FAILED(hr) || *pBooleanResult == VARIANT_TRUE )
					{
						goto exit;
					}

					break; 
				}
				else if (hr == FILTER_E_NO_VALUES)
				{
					// This chunk contains no values.
					break;
				}
				else if (FAILED(hr))
				{
					// IFilter::GetValue failed 
					break;
				}
				else
				{
					PWSTR psz = NULL;
					hr = PropVariantToStringAlloc(*pPropValue, &psz);

					if (SUCCEEDED(hr))
					{						
						szChunk.append(psz, wcslen(psz));
						CoTaskMemFree(psz);
					}

					PropVariantClear(pPropValue);
                    CoTaskMemFree(pPropValue);
                    pPropValue = NULL;
				}
			}
		}
	}

exit:
	return hr;
	
}

STDMETHODIMP CTextTokenizer::Cleanup()
{
	HRESULT hr = S_OK;

	if (m_pIPersistStream.p != NULL)
	{
		m_pIPersistStream.Release();
	}
	if (m_pIFilter.p != NULL)
	{
		m_pIFilter.Release();		
	}

	return hr;
}