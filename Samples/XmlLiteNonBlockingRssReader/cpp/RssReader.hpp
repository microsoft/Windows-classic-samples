#pragma once

#include <stdio.h>
#include <ole2.h>
#include <xmllite.h>

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define ASSERT(stmt)            do { if(!(stmt)) { DbgRaiseAssertionFailure(); } } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

class CRssReader : public IBindStatusCallback
{
public:
    CRssReader();
    virtual ~CRssReader();

    HRESULT ReadAsync(LPCWSTR pwszUrl);
    HRESULT ReadSync(LPCWSTR pwszUrl);

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    // IBindStatusCallback
    virtual HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD dwReserved, __RPC__in_opt IBinding *pib);
    virtual HRESULT STDMETHODCALLTYPE GetPriority(__RPC__out LONG *pnPriority);        
    virtual HRESULT STDMETHODCALLTYPE OnLowResource(DWORD reserved);
    virtual HRESULT STDMETHODCALLTYPE OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, __RPC__in_opt LPCWSTR szStatusText);
    virtual HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hresult, __RPC__in_opt LPCWSTR szError);
    virtual HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD *grfBINDF, BINDINFO *pbindinfo);
    virtual HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed);
    virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable(__RPC__in REFIID riid, __RPC__in_opt IUnknown *punk);

private:
    HRESULT Parse();

private:
    ULONG _ulRef;
    bool _bCompleted;
    bool _bAsync;

    IXmlReader* _spXmlReader;
};