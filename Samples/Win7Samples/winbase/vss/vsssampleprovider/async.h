/*++

Copyright (c) 2008  Microsoft Corporation

Module Name:

    Async.h

Abstract:

    Declaration of CVssAsync

TBD:


Revision History:

    Name        Date            Comments
    reuvenl     2/17/2008   Created

--*/

#pragma once

#include "stdafx.h"
////////////////////////////////////////////////////////////////////////
//  Standard foo for file name aliasing.  This code block must be after
//  all includes of VSS header files.
//
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "RESYNCASYNC"
//
////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

// Out parameters are not Pointers
template <class T>
void inline VssZeroOut( T* param )
{
    if ( param != NULL ) // OK to be NULL, the caller must treat this case separately
        ::ZeroMemory( reinterpret_cast<PVOID>(param), sizeof(T) );
}


class ATL_NO_VTABLE CVssAsync :
    public CComObjectRoot,
    public IVssAsync
    {
// Constructors& destructors
public:
    CVssAsync() : op_hresult(S_OK) 
    {}
    ~CVssAsync(){}
private:
	static const int ASYNC_WAIT_MILISEC = 3000; //Sleep for 3 seconds when IVssAsync::Wait is called from the client, to fake the latency of lun Resync
    static const int ASYNC_FAKE_PERSENT = 100;  //Currently percentage is not supported, always return 100%
    HRESULT op_hresult;
public:
    // ATL Stuff
    BEGIN_COM_MAP( CVssAsync )
        COM_INTERFACE_ENTRY( IVssAsync )
    END_COM_MAP()
public:
    void SetOpHresult(HRESULT hr)
    {
      op_hresult = hr;
    }
    static IVssAsync* CreateInstanceAndStartJob(
        HRESULT fakeReturnHr
        )
    { 
        UNREFERENCED_PARAMETER(fakeReturnHr);
        TRACE_FUNCTION();
        CComPtr<IVssAsync> pAsync;

        // Allocate the COM object.
        CComObject<CVssAsync>* pObject;
        HRESULT hr = CComObject<CVssAsync>::CreateInstance(&pObject);
        if ( FAILED(hr) )
        {
            TraceMsg(L"CComObject<CVssAsync>::CreateInstance(&pObject); failed with %x\n", hr);
            return NULL;
        }

        // Increment reference count immediately
        pObject->AddRef();  // now the ref count is 1
        pObject->SetOpHresult(fakeReturnHr);

        // Querying the IVssAsync interface. Then the ref count becomes 2.
        CComPtr<IUnknown> pUnknown = pObject->GetUnknown();
        hr = pUnknown->QueryInterface(__uuidof(IVssAsync), (void**)&pAsync); // The ref count is 2.
        if ( FAILED(hr) ) {
            TraceMsg(L"CComObject<CVssAsync>::CreateInstance(&pObject); failed with %x\n", hr);
            return NULL;
        }

        pObject->Release(); // reduce the ref count to 1

        return pAsync.Detach(); // The ref count remains 1.  The client  code has the responsibility to release this reference. 
    }

    //IVssAsync::Cancel implementation
    STDMETHOD(Cancel)() {return S_OK;}

    //IVssAsync::Wait implementation
    STDMETHOD(Wait)(
		__in DWORD dwMilliseconds = INFINITE
		)
	{
	    UNREFERENCED_PARAMETER(dwMilliseconds);
		::Sleep(CVssAsync::ASYNC_WAIT_MILISEC);
		return S_OK;
	}

    //IVssAsync::QueryStatus implementation
    STDMETHOD(QueryStatus)(
        __out           HRESULT* pHrResult,
        __out_opt     INT* pnPercentDone
    )
	{
        ::VssZeroOut(pHrResult);
        ::VssZeroOut(pnPercentDone);
        *pnPercentDone = ASYNC_FAKE_PERSENT;
		return S_OK;
	} 

};

    
