//
// pstore.h
//
// CCustomPropertyStore
//

#ifndef PSTORE_H
#define PSTORE_H

class CCustomPropertyStore : public ITfPropertyStore
{
public:
    CCustomPropertyStore();
    ~CCustomPropertyStore();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfPropertyStore
    STDMETHODIMP GetType(GUID *pguidType);
    STDMETHODIMP GetDataType(DWORD *pdwReserved);
    STDMETHODIMP GetData(VARIANT *pvarValue);
    STDMETHODIMP OnTextUpdated(DWORD dwFlags, ITfRange *pRangeNew, BOOL *pfAccept);
    STDMETHODIMP Shrink(ITfRange *pRangeNew, BOOL *pfFree);
    STDMETHODIMP Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropertyStore);
    STDMETHODIMP Clone(ITfPropertyStore **ppClone);
    STDMETHODIMP GetPropertyRangeCreator(CLSID *pclsid);
    STDMETHODIMP Serialize(IStream *pStream, ULONG *pcb);

    void _SetState(DWORD dwState)
    {
        _dwState = dwState;
    }

private:
    DWORD _dwState;
    LONG _cRef;     // COM ref count
};

#endif // PSTORE_H
