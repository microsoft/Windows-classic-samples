// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

//
// CSearchCandidateProvider
//
// TfFnSearchCandidateProvider is used for search integration feature, CSearchCandidateProvider
// implements this interface that can be called by search integration, and offer the candidate list 
// as search integration suggestion.
//
#define FAKECANDIDATENUMBER (16)
class CSearchCandidateProvider : public ITfFnSearchCandidateProvider
{
protected:
    // constructor/destructor
    CSearchCandidateProvider(_In_ ITfTextInputProcessorEx *ptip);
    virtual ~CSearchCandidateProvider();

public:
    // create instance
    static HRESULT CreateInstance(_Outptr_ ITfFnSearchCandidateProvider **ppobj, _In_ ITfTextInputProcessorEx *ptip);
    static HRESULT CreateInstance(REFIID riid, _Outptr_ void **ppvObj, _In_ ITfTextInputProcessorEx *ptip);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfFunction methods
    STDMETHODIMP GetDisplayName(_Out_ BSTR *pbstrName);

    // ITfFnSearchCandidateProvider methods
	//
	// GetSearchCandidates is responsible for supporting the candidates to caller, search integration
	// SetResult is not used
	//
    STDMETHODIMP GetSearchCandidates(BSTR bstrQuery, BSTR bstrApplicationID, _Outptr_result_maybenull_ ITfCandidateList **pplist); 
    STDMETHODIMP SetResult(BSTR bstrQuery, BSTR bstrApplicationID, BSTR bstrResult);

private:
    LONG _refCount;
    ITfTextInputProcessorEx* _pTip;
};

