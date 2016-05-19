//
// dap.cpp
//
// ITfDisplayAttributeProvider implementation.
//

#include "globals.h"
#include "mark.h"

static const TCHAR c_szAttributeInfoKey[] = TEXT("Software\\Mark Text Service");
static const TCHAR c_szAttributeInfoValueName[] = TEXT("DisplayAttr");

// this text service has only a single display attribute so we'll use
// a single static object.
class CDisplayAttributeInfo : public ITfDisplayAttributeInfo
{
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void)
    {
        return DllAddRef();
    }
    STDMETHODIMP_(ULONG) Release(void)
    {
        return DllRelease();
    }

    // ITfDisplayAttributeInfo
    STDMETHODIMP GetGUID(GUID *pguid);
    STDMETHODIMP GetDescription(BSTR *pbstrDesc);
    STDMETHODIMP GetAttributeInfo(TF_DISPLAYATTRIBUTE *ptfDisplayAttr);
    STDMETHODIMP SetAttributeInfo(const TF_DISPLAYATTRIBUTE *ptfDisplayAttr);
    STDMETHODIMP Reset();

private:
    static const TF_DISPLAYATTRIBUTE _c_DefaultDisplayAttribute;
}
g_DisplayAttributeInfo;

const TF_DISPLAYATTRIBUTE CDisplayAttributeInfo::_c_DefaultDisplayAttribute =
{
    { TF_CT_COLORREF, RGB(255, 0, 0) },     // text color
    { TF_CT_NONE, 0 },                      // background color (TF_CT_NONE => app default)
    TF_LS_SOLID,                            // underline style
    FALSE,                                  // underline boldness
    { TF_CT_COLORREF, RGB(255, 0, 0) },     // underline color
    TF_ATTR_INPUT                           // attribute info
};

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfDisplayAttributeInfo))
    {
        *ppvObj = (ITfDisplayAttributeInfo *)this;
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
// GetGUID
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::GetGUID(GUID *pguid)
{
    if (pguid == NULL)
        return E_INVALIDARG;

    *pguid = c_guidMarkDisplayAttribute;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetDescription
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::GetDescription(BSTR *pbstrDesc)
{
    BSTR bstrDesc;

    if (pbstrDesc == NULL)
        return E_INVALIDARG;

    *pbstrDesc = NULL;

    if ((bstrDesc = SysAllocString(L"Mark Display Attribute")) == NULL)
        return E_OUTOFMEMORY;

    *pbstrDesc = bstrDesc;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::GetAttributeInfo(TF_DISPLAYATTRIBUTE *ptfDisplayAttr)
{
    HKEY hKeyAttributeInfo;
    LONG lResult;
    DWORD cbData;

    if (ptfDisplayAttr == NULL)
        return E_INVALIDARG;

    lResult = E_FAIL;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szAttributeInfoKey, 0, KEY_READ, &hKeyAttributeInfo) == ERROR_SUCCESS)
    {
        cbData = sizeof(*ptfDisplayAttr);

        lResult = RegQueryValueEx(hKeyAttributeInfo, c_szAttributeInfoValueName,
                                  NULL, NULL,
                                  (LPBYTE)ptfDisplayAttr, &cbData);

        RegCloseKey(hKeyAttributeInfo);
    }

    if (lResult != ERROR_SUCCESS || cbData != sizeof(*ptfDisplayAttr))
    {
        // go with the defaults
        *ptfDisplayAttr = _c_DefaultDisplayAttribute;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// SetAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::SetAttributeInfo(const TF_DISPLAYATTRIBUTE *ptfDisplayAttr)
{
    HKEY hKeyAttributeInfo;
    LONG lResult;

    lResult = RegCreateKeyEx(HKEY_CURRENT_USER, c_szAttributeInfoKey, 0, TEXT(""),
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                             &hKeyAttributeInfo, NULL);

    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

    lResult = RegSetValueEx(hKeyAttributeInfo, c_szAttributeInfoValueName,
                            0, REG_BINARY, (const BYTE *)ptfDisplayAttr,
                            sizeof(*ptfDisplayAttr));

    RegCloseKey(hKeyAttributeInfo);

    return (lResult == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

//+---------------------------------------------------------------------------
//
// Reset
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::Reset()
{
    return SetAttributeInfo(&_c_DefaultDisplayAttribute);
}

class CEnumDisplayAttributeInfo : public IEnumTfDisplayAttributeInfo
{
public:
    CEnumDisplayAttributeInfo();
    ~CEnumDisplayAttributeInfo();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IEnumTfDisplayAttributeInfo
    STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo **ppEnum);
    STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

private:
    LONG _iIndex; // next display attribute to enum
    LONG _cRef; // COM ref count
};

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CEnumDisplayAttributeInfo::CEnumDisplayAttributeInfo()
{
    DllAddRef();

    _iIndex = 0;
    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CEnumDisplayAttributeInfo::~CEnumDisplayAttributeInfo()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumTfDisplayAttributeInfo))
    {
        *ppvObj = (IEnumTfDisplayAttributeInfo *)this;
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

STDAPI_(ULONG) CEnumDisplayAttributeInfo::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CEnumDisplayAttributeInfo::Release()
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
// Clone
//
// Returns a copy of the object.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Clone(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pClone;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pClone = new CEnumDisplayAttributeInfo) == NULL)
        return E_OUTOFMEMORY;

    // the clone should match this object's state
    pClone->_iIndex = _iIndex;

    *ppEnum = pClone;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Next
//
// Returns an array of display attribute info objects supported by this service.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched)
{
    ULONG cFetched;

    if (pcFetched == NULL)
    {
        // technically this is only legal if ulCount == 1, but we won't check
        pcFetched = &cFetched;
    }

    *pcFetched = 0;

    if (ulCount == 0)
        return S_OK;

    // we only have a single display attribute to enum, so this is trivial

    if (_iIndex == 0)
    {
        *rgInfo = &g_DisplayAttributeInfo;
        (*rgInfo)->AddRef();
        *pcFetched = 1;
        _iIndex++;
    }

    return (*pcFetched == ulCount) ? S_OK : S_FALSE;
}

//+---------------------------------------------------------------------------
//
// Reset
//
// Resets the enumeration.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Reset()
{
    _iIndex = 0;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Skip
//
// Skips past objects in the enumeration.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Skip(ULONG ulCount)
{
    // we have only a single item to enum
    // so we can just skip it and avoid any overflow errors
    if (ulCount > 0 && _iIndex == 0)
    {
        _iIndex++;
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// EnumDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CMarkTextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pAttributeEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pAttributeEnum = new CEnumDisplayAttributeInfo) == NULL)
        return E_OUTOFMEMORY;

    *ppEnum = pAttributeEnum;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CMarkTextService::GetDisplayAttributeInfo(REFGUID guidInfo, ITfDisplayAttributeInfo **ppInfo)
{
    if (ppInfo == NULL)
        return E_INVALIDARG;

    *ppInfo = NULL;

    // unsupported GUID?
    if (!IsEqualGUID(guidInfo, c_guidMarkDisplayAttribute))
        return E_INVALIDARG;

    *ppInfo = &g_DisplayAttributeInfo;
    (*ppInfo)->AddRef();

    return S_OK;
}
