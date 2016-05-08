//
// pstore.cpp
//
// CCustomPropertyStore implementation, ITfPropertyStore example.
//

#include "globals.h"
#include "pstore.h"
#include "mark.h"

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CCustomPropertyStore::CCustomPropertyStore()
{
    // assign some arbitrary state to this object, for demonstration purposes
    _dwState = GetTickCount();

    DllAddRef();
    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CCustomPropertyStore::~CCustomPropertyStore()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfPropertyStore))
    {
        *ppvObj = (ITfPropertyStore *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCustomPropertyStore::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCustomPropertyStore::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// GetType
//
// Returns the GUID of the property this store is assigned to.
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::GetType(GUID *pguidType)
{
    *pguidType = c_guidCustomProperty;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetDataType
//
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::GetDataType(DWORD *pdwReserved)
{
    // this method is reserved, pdwReserved must be set 0
    *pdwReserved = 0;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetData
//
// Returns the data held by this store.
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::GetData(VARIANT *pvarValue)
{
    pvarValue->vt = VT_I4; // dword value for this store
    pvarValue->lVal = _dwState;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnTextUpdated
//
// Called by TSF when the text covered by this store is modified.
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::OnTextUpdated(DWORD dwFlags, ITfRange *pRangeNew, BOOL *pfAccept)
{
    // we will throw away this store (give up) anytime the text is modified.
    // If we did set pfAccept TRUE, the store would be maintained.
    *pfAccept = FALSE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Shrink
//
// Called when the text covered by this store is truncated.
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::Shrink(ITfRange *pRangeNew, BOOL *pfFree)
{
    // give up and free this store
    *pfFree = TRUE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Divide
//
// Called when text covered by the store is deleted, such that the store is
// split in two.  We have the option of freeing the store, or keeping the
// store which moves to cover just pRangeThis (the leftmost text) in which
// case we must provide a new store for the rightmost text as well.
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::Divide(ITfRange *pRangeThis, ITfRange *pRangeNew, ITfPropertyStore **ppPropertyStore)
{
    // just give up
    // a NULL ppPropertyStore tells TSF to release this store
    *ppPropertyStore = NULL;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Clone
//
// Return a new store with a copy of this store's state.
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::Clone(ITfPropertyStore **ppCloneOut)
{
    CCustomPropertyStore *pClone;

    *ppCloneOut = NULL;

    if ((pClone = new CCustomPropertyStore) == NULL)
        return E_OUTOFMEMORY;

    pClone->_dwState = _dwState;

    *ppCloneOut = pClone;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetPropertyRangeCreator
//
// Returns the owning text service.  Used by TSF during Unserialization.
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::GetPropertyRangeCreator(CLSID *pclsid)
{
    *pclsid = c_clsidMarkTextService;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Serialize
//
// Copy this store's state to a binary stream.  This stream will later be returned
// to this text service to recreate the store in a later session.
//----------------------------------------------------------------------------

STDAPI CCustomPropertyStore::Serialize(IStream *pStream, ULONG *pcb)
{
    return pStream->Write(&_dwState, sizeof(_dwState), pcb);
}

//+---------------------------------------------------------------------------
//
// IsStoreSerializable
//
//----------------------------------------------------------------------------

STDAPI CMarkTextService::IsStoreSerializable(REFGUID guidProperty, ITfRange *pRange, ITfPropertyStore *pPropertyStore, BOOL *pfSerializable)
{
    // we don't have any complicated logic, we'll let all our custom property values be serialized
    *pfSerializable = IsEqualGUID(guidProperty, c_guidCustomProperty);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// CreatePropertyStore
//
// Unserializes a custom property.
//----------------------------------------------------------------------------

STDAPI CMarkTextService::CreatePropertyStore(REFGUID guidProperty, ITfRange *pRange, ULONG cb, IStream *pStream, ITfPropertyStore **ppStore)
{
    CCustomPropertyStore *pStore;
    DWORD dwState;

    *ppStore = NULL;

    if (!IsEqualGUID(guidProperty, c_guidCustomProperty))
        return E_INVALIDARG;

    if (cb != sizeof(DWORD))
        return E_INVALIDARG;

    if ((pStore = new CCustomPropertyStore) == NULL)
        return E_OUTOFMEMORY;

    if (pStream->Read(&dwState, sizeof(DWORD), &cb) != S_OK || cb != sizeof(DWORD))
    {
        pStore->Release();
        return E_FAIL;
    }

    pStore->_SetState(dwState);
    *ppStore = pStore;

    return S_OK;
}
