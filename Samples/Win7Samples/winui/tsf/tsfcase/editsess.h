//
// editsess.h
//
// CEditSessionBase declaration.
//

#ifndef EDITSESS_H
#define EDITSESS_H

class CEditSessionBase : public ITfEditSession
{
public:
    CEditSessionBase(ITfContext *pContext)
    {
        _cRef = 1;
        _pContext = pContext;
        _pContext->AddRef();
    }
    virtual ~CEditSessionBase()
    {
        _pContext->Release();
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj)
    {
        if (ppvObj == NULL)
            return E_INVALIDARG;

        *ppvObj = NULL;

        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_ITfEditSession))
        {
            *ppvObj = (ITfLangBarItemButton *)this;
        }

        if (*ppvObj)
        {
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef(void)
    {
        return ++_cRef;
    }
    STDMETHODIMP_(ULONG) Release(void)
    {
        LONG cr = --_cRef;

        assert(_cRef >= 0);

        if (_cRef == 0)
        {
            delete this;
        }

        return cr;
    }

    // ITfEditSession
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) = 0;

protected:
    ITfContext *_pContext;

private:
    LONG _cRef;     // COM ref count
};

#endif // EDITSESS_H
